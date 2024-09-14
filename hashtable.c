#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashtable.h"

static unsigned int hash(char* key, size_t size) {
    unsigned int hash = 5381;
    int c;
    while ((c = *key++)) hash = ((hash << 5) + hash) + c;
    return hash % size;
}

static void hash_table_resize(hash_table_t* hash_table, size_t new_size) {
    key_value_pair_t** new_table = calloc(new_size, sizeof(key_value_pair_t*));
    if (new_table == NULL) {
        perror("Failed to allocate memory for resized hash table");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < hash_table->size; i++) {
        key_value_pair_t* pair = hash_table->table[i];
        while (pair != NULL) {
            key_value_pair_t* next_pair = pair->next;
            unsigned int new_index = hash(pair->key, new_size);

            pair->next = new_table[new_index];
            new_table[new_index] = pair;

            pair = next_pair;
        }
    }

    free(hash_table->table);
    hash_table->table = new_table;
    hash_table->size = new_size;
}

hash_table_t* hash_table_create(void) {
    hash_table_t* hash_table = malloc(sizeof(hash_table_t));
    if (hash_table == NULL) {
        perror("Failed to create hash table");
        exit(EXIT_FAILURE);
    }
    hash_table->size = INITIAL_HASH_SIZE;
    hash_table->count = 0;
    hash_table->table = calloc(hash_table->size, sizeof(key_value_pair_t*));
    if (hash_table->table == NULL) {
        perror("Failed to allocate memory for hash table");
        free(hash_table);
        exit(EXIT_FAILURE);
    }
    return hash_table;
}

void hash_table_insert(hash_table_t* hash_table, char* key, char* value) {
    if ((double)hash_table->count / hash_table->size > LOAD_FACTOR_THRESHOLD)
        hash_table_resize(hash_table, hash_table->size * 2);

    unsigned int index = hash(key, hash_table->size);

    key_value_pair_t* current = hash_table->table[index];
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            free(current->value);
            current->value = malloc(strlen(value) + 1);
            if (current->value == NULL) {
                perror("Failed to allocate memory for value");
                exit(EXIT_FAILURE);
            }
            strcpy(current->value, value);
            return;
        }
        current = current->next;
    }

    key_value_pair_t* new_pair = malloc(sizeof(key_value_pair_t));
    if (new_pair == NULL) {
        perror("Failed to allocate memory for key-value pair");
        exit(EXIT_FAILURE);
    }
    new_pair->key = malloc(strlen(key) + 1);
    new_pair->value = malloc(strlen(value) + 1);
    if (new_pair->key == NULL || new_pair->value == NULL) {
        perror("Failed to allocate memory for key or value");
        free(new_pair->key);
        free(new_pair->value);
        free(new_pair);
        exit(EXIT_FAILURE);
    }
    strcpy(new_pair->key, key);
    strcpy(new_pair->value, value);
    new_pair->next = hash_table->table[index];
    hash_table->table[index] = new_pair;
    hash_table->count++;
}

char* hash_table_lookup(hash_table_t* hash_table, char* key) {
    unsigned int index = hash(key, hash_table->size);
    key_value_pair_t* current = hash_table->table[index];
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) return current->value;
        current = current->next;
    }
    return NULL;
}

char* hash_table_lookup_key(hash_table_t* hash_table, char* value) {
    for (unsigned int i = 0; i < hash_table->size; i++) {
        key_value_pair_t* current = hash_table->table[i];
        while (current != NULL) {
            if (strcmp(current->value, value) == 0) return current->key;
            current = current->next;
        }
    }
    return NULL;
}


void hash_table_load_from_file(hash_table_t* hash_table, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        return;
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';

        char* space_pos = strchr(line, ' ');
        if (space_pos == NULL) {
            fprintf(stderr, "Invalid line format: %s\n", line);
            continue;
        }

        *space_pos = '\0';
        char* key = line;
        char* value = space_pos + 1;

        char* key_copy = malloc(strlen(key) + 1);
        char* value_copy = malloc(strlen(value) + 1);
        if (key_copy == NULL || value_copy == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            free(key_copy);
            free(value_copy);
            fclose(file);
            return;
        }
        strcpy(key_copy, key);
        strcpy(value_copy, value);

        hash_table_insert(hash_table, key_copy, value_copy);
    }

    fclose(file);
}

void hash_table_free(hash_table_t* hash_table) {
    for (size_t i = 0; i < hash_table->size; i++) {
        key_value_pair_t* pair = hash_table->table[i];
        while (pair != NULL) {
            key_value_pair_t* next_pair = pair->next;
            free(pair->key);
            free(pair->value);
            free(pair);
            pair = next_pair;
        }
    }
    free(hash_table->table);
    free(hash_table);
}
