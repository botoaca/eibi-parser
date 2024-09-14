#ifndef STATION_H
#define STATION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct station {
    char* frequency;   // kHz
    char* time;        // UTC  
    char* days;
    char* itu;
    char* station;
    char* language;
    char* target_area;
    char* remarks;
} station_t;

char* station_data_trim(const char* str);
station_t station_create(const char* data);
void station_free(station_t* station);

#endif // STATION_H