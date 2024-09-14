#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "station.h"

char* station_data_trim(const char* str) {
    // Skip leading spaces
    while (*str == ' ') str++;

    // If the string is empty or contains only spaces
    if (*str == '\0') return NULL;

    // Skip trailing spaces
    const char* end = str + strlen(str) - 1;
    while (end > str && *end == ' ') end--;

    // Calculate length and copy trimmed string
    size_t len = end - str + 1;
    char* result = (char*)malloc(len + 1);
    if (result) {
        strncpy(result, str, len);
        result[len] = '\0';
    }
    return result;
}

station_t station_create(const char* data) {
    station_t station = {0};
    char temp[1024];
    char *ptr;
    
    strncpy(temp, data, sizeof(temp));
    temp[sizeof(temp) - 1] = '\0';

    // Extract frequency (up to 13 characters)
    char freq[14] = {0};  // 13 characters + null terminator
    strncpy(freq, temp, 13);
    station.frequency = station_data_trim(freq);
    ptr = temp + 14;

    // Extract time (up to 9 characters)
    char time[10] = {0};  // 9 characters + null terminator
    strncpy(time, ptr, 9);
    station.time = station_data_trim(time);
    ptr += 10;

    // Extract days (up to 5 characters)
    char days[6] = {0};  // 5 characters + null terminator
    strncpy(days, ptr, 5);
    station.days = station_data_trim(days);
    ptr += 6;

    // Extract ITU (up to 3 characters)
    char itu[4] = {0};  // 3 characters + null terminator
    strncpy(itu, ptr, 3);
    station.itu = station_data_trim(itu);
    ptr += 4;

    // Extract station (up to 24 characters)
    char station_name[25] = {0};  // 24 characters + null terminator
    strncpy(station_name, ptr, 24);
    station.station = station_data_trim(station_name);
    ptr += 25;

    // Extract language (up to 3 characters)
    char language[4] = {0};  // 3 characters + null terminator
    strncpy(language, ptr, 3);
    station.language = station_data_trim(language);
    ptr += 4;

    // Extract target area (up to 11 characters)
    char target_area[12] = {0};  // 11 characters + null terminator
    strncpy(target_area, ptr, 11);
    station.target_area = station_data_trim(target_area);
    ptr += 12;

    // Extract remarks (remaining part of the string)
    station.remarks = station_data_trim(ptr);
    // If remarks is just spaces, it should be NULL
    if (station.remarks != NULL && strlen(station.remarks) == 0) {
        free(station.remarks);
        station.remarks = NULL;
    }

    return station;
}

void station_free(station_t* station) {
    if (station != NULL) {
        free(station->time);
        free(station->days);
        free(station->itu);
        free(station->station);
        free(station->language);
        free(station->target_area);
        free(station->remarks);
        free(station);
    }
}