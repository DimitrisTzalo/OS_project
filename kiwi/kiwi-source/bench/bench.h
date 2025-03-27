#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <pthread.h>
#include "../engine/db.h"


#define KSIZE (16)
#define VSIZE (1000)

#define LINE "+-----------------------------+----------------+------------------------------+-------------------+\n"
#define LINE1 "---------------------------------------------------------------------------------------------------\n"

#define DATAS ("testdb") //allaxe to onoma tou db

long long get_ustime_sec(void);
void _random_key(char *key,int length);

typedef struct { // thread arguments
    DB* db;
	long int begin, end;
    int r;
    int thread_id;
    int num_threads;
} thread_data_t;



void multi_write_test(long int count, int r, int num_threads);
void *_write_thread(void *arg);

