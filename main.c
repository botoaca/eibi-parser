#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "curl/curl.h"

#include "hashtable.h"
#include "station.h"

void download_eibi_schedule() {
    CURL* curl = curl_easy_init();
    if (curl) {
        FILE* file = fopen("eibi_schedule.txt", "wb");
        if (file) {
            curl_easy_setopt(curl, CURLOPT_URL, "http://www.eibispace.de/dx/eibi.txt");
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
            curl_easy_perform(curl);
            fclose(file);
        }
        curl_easy_cleanup(curl);
    }
}

char** read_file_into_string(const char* filename, int* num_lines) {
    char** lines = NULL;
    *num_lines = 0;
    size_t line_capacity = 0;
    char* buffer = NULL;

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open file");
        return NULL;
    }

    while (getline(&buffer, &line_capacity, file) != -1) {
        buffer[strcspn(buffer, "\n")] = '\0';
        lines = realloc(lines, (*num_lines + 1) * sizeof(char*));
        if (lines == NULL) {
            perror("Failed to reallocate memory");
            free(buffer);
            fclose(file);
            return NULL;
        }

        lines[*num_lines] = malloc(strlen(buffer) + 1);
        strcpy(lines[*num_lines], buffer);
        (*num_lines)++;
    }

    free(buffer);
    fclose(file);

    return lines;
}

int all_patterns_found(const char* line, const char* patterns[], int num_patterns) {
    for (int i = 0; i < num_patterns; i++) 
        if (strstr(line, patterns[i]) == NULL) return 0;
    return 1;
}

char** find_lines_with_patterns(char* lines[], int num_lines, const char* patterns[], int num_patterns, int* num_matches) {
    char** matched_lines = NULL;
    *num_matches = 0;

    for (int i = 0; i < num_lines; i++) {
        if (all_patterns_found(lines[i], patterns, num_patterns)) {
            matched_lines = realloc(matched_lines, (*num_matches + 1) * sizeof(char*));
            if (matched_lines == NULL) {
                fprintf(stderr, "Failed to reallocate memory for matched lines\n");
                return NULL;
            }
            matched_lines[*num_matches] = lines[i];
            (*num_matches)++;
        }
    }

    return matched_lines;
}

// char** find_pattern_in_string(char* lines[], int num_lines, const char* pattern, int* num_matches) {
//     char** matched_lines = NULL;
//     *num_matches = 0;
//
//     for (int i = 0; i < num_lines; i++) {
//         if (strstr(lines[i], pattern) != NULL) {
//             matched_lines = realloc(matched_lines, (*num_matches + 1) * sizeof(char*));
//             if (matched_lines == NULL) {
//                 perror("Failed to reallocate memory for matched lines");
//                 return NULL;
//             }
//             matched_lines[*num_matches] = lines[i];
//             (*num_matches)++;
//         }
//     }
//
//     return matched_lines;
// }

time_t get_expiration_date(const char* line) {
    struct tm tm = {0};
    const char* month_names[] = { 
        "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" 
    };

    const char* end_date_str = strstr(line, " - ");
    if (end_date_str == NULL) {
        fprintf(stderr, "Cannot find the end date in line: %s\n", line);
        return (time_t)-1;
    }
    end_date_str += 3;

    char month[20];
    int day, year;

    if (sscanf(end_date_str, "%s %d, %d", month, &day, &year) != 3) {
        fprintf(stderr, "Error parsing date from line: %s\n", end_date_str);
        return (time_t)-1;
    }

    int month_index = -1;
    for (int i = 0; i < 12; i++) {
        if (strcmp(month, month_names[i]) == 0) {
            month_index = i;
            break;
        }
    }
    if (month_index == -1) {
        fprintf(stderr, "Cannot find month in line: %s\n", month);
        return (time_t)-1;
    }

    tm.tm_year = year - 1900;
    tm.tm_mon = month_index;
    tm.tm_mday = day;

    time_t expiration_time = mktime(&tm);
    if (expiration_time == (time_t)-1) {
        fprintf(stderr, "Error converting time to time_t\n");
        return (time_t)-1;
    }

    return expiration_time;
}

int file_exists(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
}

char* convert_time_range(const char* time_range) {
    char* result = (char*)malloc(50 * sizeof(char)); 
    if (result == NULL) {
        perror("Failed to allocate memory");
        return NULL;
    }

    char start_time[5] = {0};
    char end_time[5] = {0};
    sscanf(time_range, "%4s-%4s", start_time, end_time);

    char start_hour[3] = {0};
    char start_minute[3] = {0};
    sscanf(start_time, "%2s%2s", start_hour, start_minute);
    char start_formatted[10];
    snprintf(start_formatted, sizeof(start_formatted), "%s:%s UTC", start_hour, start_minute);

    char end_hour[3] = {0};
    char end_minute[3] = {0};
    sscanf(end_time, "%2s%2s", end_hour, end_minute);
    char end_formatted[10];
    snprintf(end_formatted, sizeof(end_formatted), "%s:%s UTC", end_hour, end_minute);

    snprintf(result, 50, "%s to %s", start_formatted, end_formatted);
    return result;
}

typedef struct args_t {
    char* name;
    char* frequency;
    char* time;
    char* country;
    char* language;
    char* target_area;
} args_t;

int main(int argc, char* argv[]) {
    args_t args = {0};
    for (int i = 1; i < argc; i++) {
        if      (strcmp(argv[i], "--name") == 0)                                      args.name = argv[++i];
        else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--freq") == 0)        args.frequency = argv[++i];
        else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--time") == 0)        args.time = argv[++i];
        else if (strcmp(argv[i], "--country") == 0)                                   args.country = argv[++i];
        else if (strcmp(argv[i], "-lang") == 0 || strcmp(argv[i], "--language") == 0) args.language = argv[++i];
        else if (strcmp(argv[i], "--target") == 0)                                    args.target_area = argv[++i];
    }
    
    hash_table_t* ht_lang_codes = hash_table_create();
    hash_table_t* ht_country_codes = hash_table_create();
    hash_table_t* ht_target_area_codes = hash_table_create();
    hash_table_load_from_file(ht_lang_codes, "eibi_language_codes.txt");
    hash_table_load_from_file(ht_country_codes, "eibi_country_codes.txt");
    hash_table_load_from_file(ht_target_area_codes, "eibi_target_area_codes.txt");
    
    const char* schedule_file = "eibi_schedule.txt";
    int num_lines;
    char** lines = read_file_into_string(schedule_file, &num_lines);

    if (lines == NULL) {
        fprintf(stderr, "Failed to read file into string\n");
        return 1;
    }

    int num_matches;
    time_t expiration_date = get_expiration_date(find_lines_with_patterns(lines, num_lines, (const char*[]){"Valid"}, 1, &num_matches)[0]);
    if (expiration_date == (time_t)-1 || time(NULL) > expiration_date) { 
        download_eibi_schedule();
        lines = read_file_into_string(schedule_file, &num_lines);
    }
    num_matches = 0;

    //////////
    
    const char* patterns[] = {
        args.name == NULL ? "" : args.name,
        args.frequency == NULL ? "" : args.frequency,
        args.time == NULL ? "" : args.time,
        args.country == NULL ? "" : args.country,
        args.language == NULL ? "" : hash_table_lookup_key(ht_lang_codes, args.language) == NULL ? "" : hash_table_lookup_key(ht_lang_codes, args.language),
        args.target_area == NULL ? "" : (hash_table_lookup_key(ht_target_area_codes, args.target_area) == NULL ? "" : hash_table_lookup_key(ht_target_area_codes, args.target_area))
    };
    int num_patterns = sizeof(patterns) / sizeof(patterns[0]);

    char** matched_lines = find_lines_with_patterns(lines + 17, num_lines - 17, patterns, num_patterns, &num_matches);

    station_t stations[num_matches];
    for (int i = 0; i < num_matches; i++) {
        stations[i] = station_create(matched_lines[i]);
    }

    printf("Number of matches: %d\n\n", num_matches);
    for (int i = 0; i < num_matches; i++) {
        printf("Frequency: %s kHz\n", stations[i].frequency);
        printf("Time: %s\n", convert_time_range(stations[i].time));
        printf("Days: %s\n", stations[i].days == NULL ? "N/A" : stations[i].days);
        printf("ITU: %s\n", stations[i].itu == NULL ? "N/A" : hash_table_lookup(ht_country_codes, stations[i].itu));
        printf("Station: %s\n", stations[i].station);
        printf("Language: %s\n", stations[i].language == NULL ? "N/A" : hash_table_lookup(ht_lang_codes, stations[i].language));
        printf("Target Area: %s\n", stations[i].target_area == NULL ? "N/A" : hash_table_lookup(ht_target_area_codes, stations[i].target_area));
        printf("Remarks: %s\n", stations[i].remarks == NULL ? "None" : stations[i].remarks);
        printf("\n");
    }

    //////////

    return 0;
}
