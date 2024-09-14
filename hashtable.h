#ifndef HASHTABLE_H
#define HASHTABLE_H

#define MAX_LINE_LENGTH 256
#define INITIAL_HASH_SIZE 100
#define LOAD_FACTOR_THRESHOLD 0.75

typedef struct key_value_pair {
    char* key;
    char* value;
    struct key_value_pair* next;
} key_value_pair_t;

typedef struct {
    key_value_pair_t** table;
    size_t size;
    size_t count;
} hash_table_t;

hash_table_t* hash_table_create(void);
void hash_table_insert(hash_table_t* hash_table, char* key, char* value);
char* hash_table_lookup(hash_table_t* hash_table, char* key);
char* hash_table_lookup_key(hash_table_t* hash_table, char* value);
void hash_table_load_from_file(hash_table_t* hash_table, const char* filename);
void hash_table_free(hash_table_t* hash_table);

#endif // HASHTABLE_H
