
#include <string.h>
#include "../engine/variant.h"
#include "bench.h"

#define DATAS ("testdb")

pthread_mutex_t write_mutex;
pthread_mutex_t read_mutex;

long int added;
long int found;

int write_ceil(double num) {
    int inum = (int)num;
    if (num > inum) {
        return inum + 1;
    }
    return inum;
}

void _write_test(long int count, int r)
{
	int i;
	double cost;
	long long start,end;
	Variant sk, sv;
	DB* db;

	char key[KSIZE + 1];
	char val[VSIZE + 1];
	char sbuf[1024];

	memset(key, 0, KSIZE + 1);
	memset(val, 0, VSIZE + 1);
	memset(sbuf, 0, 1024);

	db = db_open(DATAS);

	start = get_ustime_sec();
	for (i = 0; i < count; i++) {
		if (r)
			_random_key(key, KSIZE);
		else
			snprintf(key, KSIZE, "key-%d", i);
		fprintf(stderr, "%d adding %s\n", i, key);
		snprintf(val, VSIZE, "val-%d", i);

		sk.length = KSIZE;
		sk.mem = key;
		sv.length = VSIZE;
		sv.mem = val;

		db_add(db, &sk, &sv);
		if ((i % 10000) == 0) {
			fprintf(stderr,"random write finished %d ops%30s\r", 
					i, 
					"");

			fflush(stderr);
		}
	}

	db_close(db);

	end = get_ustime_sec();
	cost = end -start;

	printf(LINE);
	printf("|Random-Write	(done:%ld): %.6f sec/op; %.1f writes/sec(estimated); cost:%.3f(sec);\n"
		,count, (double)(cost / count)
		,(double)(count / cost)
		,cost);	
}

void _read_test(long int count, int r)
{
	int i;
	int ret;
	int found = 0;
	double cost;
	long long start,end;
	Variant sk;
	Variant sv;
	DB* db;
	char key[KSIZE + 1];

	db = db_open(DATAS);
	start = get_ustime_sec();
	for (i = 0; i < count; i++) {
		memset(key, 0, KSIZE + 1);

		/* if you want to test random write, use the following */
		if (r)
			_random_key(key, KSIZE);
		else
			snprintf(key, KSIZE, "key-%d", i);
		fprintf(stderr, "%d searching %s\n", i, key);
		sk.length = KSIZE;
		sk.mem = key;
		ret = db_get(db, &sk, &sv);
		if (ret) {
			//db_free_data(sv.mem);
			found++;
		} else {
			INFO("not found key#%s", 
					sk.mem);
    	}

		if ((i % 10000) == 0) {
			fprintf(stderr,"random read finished %d ops%30s\r", 
					i, 
					"");

			fflush(stderr);
		}
	}

	db_close(db);

	end = get_ustime_sec();
	cost = end - start;
	printf(LINE);
	printf("|Random-Read	(done:%ld, found:%d): %.6f sec/op; %.1f reads /sec(estimated); cost:%.3f(sec)\n",
		count, found,
		(double)(cost / count),
		(double)(count / cost),
		cost);
}


void *thread_write(void * arg) {
    thread_data_t *data = (thread_data_t *) arg; // cast the arguments passed from void to thread_data_t
    int i;
    double cost;
    long long start, end;
    Variant sk, sv;

    char key[KSIZE + 1];
    char val[VSIZE + 1];
    char sbuf[1024];

    memset(key, 0, KSIZE + 1);
    memset(val, 0, VSIZE + 1);
    memset(sbuf, 0, 1024);
    
    start = get_ustime_sec(); // start time of the operation
    for (i = data->begin; i < data->end; i++) { // loop from count_begin of thread to count end
        if (data->r)
            _random_key(key, KSIZE);
        else
            snprintf(key, KSIZE, "key-%d", i);
        fprintf(stderr, "%d adding %s\n", i, key);
        snprintf(val, VSIZE, "val-%d", i);

        sk.length = KSIZE;
        sk.mem = key;
        sv.length = VSIZE;
        sv.mem = val;
		pthread_mutex_lock(&write_mutex); // lock the statistics mutex for writing
        
		db_add(data -> db, &sk, &sv);
		added++;

		pthread_mutex_unlock(&write_mutex); // unlock the statistics mutex for writing

        if ((i % 10000) == 0) {
            fprintf(stderr, "random write finished %d ops%30s\r", 
                    i, 
                    "");

            fflush(stderr);
        }
		
    }

    end = get_ustime_sec();
    cost = end - start;
    
    return NULL;
}

void multi_write_test(long int count,int r, int threads) {
    extern DB* db;       // DB OPENED IN MAIN
	thread_data_t* thread_args = malloc(threads * sizeof(thread_data_t));
    pthread_t* tid = malloc(threads * sizeof(pthread_t));      // array of thread ids
    long long start, end;        // start time and end time
    double cost;        // the total time of operations
	

	pthread_mutex_init(&write_mutex, NULL); // initialize the stats mutex for writing
    start = get_ustime_sec();        // calculate starting time for the operation
    for (int i = 0; i < threads; i++) {
        // calculate the thread arguments for each thread
		thread_args[i].db = db;
        thread_args[i].begin = i * count / threads;
        thread_args[i].end = (i + 1) * count / threads;
        thread_args[i].r = r;
        thread_args[i].thread_id = i;
        thread_args[i].num_threads = threads;
        // create writing threads
        pthread_create(&tid[i], NULL, thread_write, &thread_args[i]);
		
    }
    for (int i = 0; i < threads; i++) {
        pthread_join(tid[i], NULL);
    }
    db_close(db);            // close the database
    end = get_ustime_sec();            // calculate end time of the operation
    cost = end - start;            // calculate operating time

	printf(LINE);
	printf("|Random-Multi-Thread-Write (done:%ld, added:%ld): %.6f sec/op; %.1f writes/sec(estimated); cost:%.3f(sec); Threads: %d\n",
       count, added, (double)(cost / count),
       (double)(count / cost),
       cost, threads);
	
	pthread_mutex_destroy(&write_mutex);
	
	free(thread_args);
	free(tid);

}

void *thread_read(void *arg) {
    thread_data_t *data = (thread_data_t *) arg; // cast the arguments passed from void to thread_data_t
    int i;
	int ret;
    double cost;
    long long start, end;
    Variant sk, sv;
    int local_found = 0;

    char key[KSIZE + 1];
    char sbuf[1024];

    memset(key, 0, KSIZE + 1);
    memset(sbuf, 0, 1024);

    start = get_ustime_sec(); // start time of the operation
    for (i = data->begin; i < data->end; i++) { // loop from count_begin of thread to count end
        if (data->r)
            _random_key(key, KSIZE);
        else
            snprintf(key, KSIZE, "key-%d", i);
        fprintf(stderr, "%d searching %s\n", i, key);

        sk.length = KSIZE;
        sk.mem = key;
		
		
		ret = db_get(data->db, &sk, &sv);
        if (ret) {
            local_found++;
        }
		
        if ((i % 10000) == 0) {
            fprintf(stderr, "random read finished %d ops%30s\r", 
                    i, 
                    "");

            fflush(stderr);
        }
    }

    end = get_ustime_sec();
    cost = end - start;

	pthread_mutex_lock(&read_mutex); // lock the statistics mutex for reading	
	found += local_found;
	pthread_mutex_unlock(&read_mutex);

    
    

    return NULL;
}

void multi_read_test(long int count, int r, int threads) {
    extern DB* db;       // DB OPENED IN MAIN
	thread_data_t* thread_args = malloc(threads * sizeof(thread_data_t));
    pthread_t* tid = malloc(threads * sizeof(pthread_t));      // array of thread ids
    long long start, end;        // start time and end time
    double cost;        // the total time of operations
    

    pthread_mutex_init(&read_mutex, NULL); // initialize the stats mutex for writing
    start = get_ustime_sec();        // calculate starting time for the operation
    for (int i = 0; i < threads; i++) {
        // calculate the thread arguments for each thread
        thread_args[i].db = db;
        thread_args[i].begin = i * count / threads;
        thread_args[i].end = (i + 1) * count / threads;
        thread_args[i].r = r;
        thread_args[i].thread_id = i;
        thread_args[i].num_threads = threads;
        // create reading threads
        pthread_create(&tid[i], NULL, thread_read, &thread_args[i]);
    }
    for (int i = 0; i < threads; i++) {
        pthread_join(tid[i], NULL);
    }
    db_close(db);            // close the database
    end = get_ustime_sec();            // calculate end time of the operation
    cost = end - start;            // calculate operating time

    printf(LINE);
    printf("|Random-Multi-Thread-Read (done:%ld, found:%ld): %.6f sec/op; %.1f reads/sec(estimated); cost:%.3f(sec); Threads: %d\n",
           count,found, (double)(cost / count),
           (double)(count / cost),
           cost, threads);
	
	pthread_mutex_destroy(&read_mutex);
	
	free(thread_args);
	free(tid);
}

void readwrite_test (long int count, int r, int threads, int write_percentage) {
	extern DB* db;       // DB OPENED IN MAIN
	double wthreads = write_ceil((double)(threads * write_percentage) / 100.0); // calculate the number of write threads
	int write_threads = (int)wthreads;
	int read_threads = threads - write_threads; // calculate the number of read threads
	thread_data_t *write_args = malloc(sizeof(thread_data_t) * write_threads);   // array of structs data (bench.h) to pass as arguments
	thread_data_t *read_args = malloc(sizeof(thread_data_t) * read_threads);   ;   // array of structs data (bench.h) to pass as arguments
	pthread_t* write_tid = malloc(write_threads * sizeof(pthread_t));
    pthread_t* read_tid = malloc(read_threads * sizeof(pthread_t));        // array of thread ids
	long long start_write,start_read,end_write,end_read;       // start time and end time
	double write_cost;        // the total time of write operations
	double read_cost;        // the total time of read operations
	double cost;        // the total time of operations
	
	
	

	pthread_mutex_init(&write_mutex, NULL); // initialize the stats mutex for writing
	pthread_mutex_init(&read_mutex, NULL); // initialize the stats mutex for reading
	
	start_write = get_ustime_sec();        // calculate starting time for the operation
	for (int i = 0; i < write_threads; i++) {
		// calculate the thread arguments for each thread
		//pthread_mutex_lock(&write_mutex); // lock the statistics mutex for writing
		write_args[i].db = db;
		write_args[i].begin = i * count / write_threads;
		write_args[i].end = (i + 1) * count / write_threads;
		write_args[i].r = r;
		write_args[i].thread_id = i;
		write_args[i].num_threads = write_threads;
		// create writing threads
		pthread_create(&write_tid[i], NULL, thread_write, &write_args[i]);

		//pthread_mutex_unlock(&write_mutex); // unlock the statistics mutex for writing
	}


	start_read = get_ustime_sec();        // calculate starting time for the operation
	for (int i = 0; i < read_threads; i++) {
		// calculate the thread arguments for each thread
		//pthread_mutex_lock(&read_mutex);
		read_args[i].db = db;
		read_args[i].begin = i * count / read_threads;
		read_args[i].end = (i + 1) * count / read_threads;
		read_args[i].r = r;
		read_args[i].thread_id = i;
		read_args[i].num_threads = read_threads;
		// create reading threads
		pthread_create(&read_tid[i], NULL, thread_read, &read_args[i]);

		//pthread_mutex_unlock(&read_mutex);
	}
	

	for (int i = 0; i < write_threads; i++) {
		pthread_join(write_tid[i], NULL);
	}

	end_write = get_ustime_sec();            // calculate end time of the operation
	write_cost = end_write - start_write;            // calculate operating time

	/*o kodikas apo 347-352 einai kato apo start_read*/
	          
	
	for (int i = 0; i < read_threads; i++) {
		pthread_join(read_tid[i], NULL);
	}
	
	end_read = get_ustime_sec();            // calculate end time of the operation
	read_cost = end_read - start_read;            // calculate operating time

	
	cost = write_cost + read_cost;            // calculate operating time
	db_close(db);            // close the database

	printf(LINE);
    printf("|Random-Multi-Thread-ReadWrite (done:%ld): %.6f sec/op; %.1f writes&reads/sec(estimated); cost:%.3f(sec); Threads: %d\n",
       count,(double)(cost / count),
       (double)(count / cost),
       cost, threads);



	printf("|Random-Multi-Thread-Write (done:%ld, added:%ld): %.6f sec/op; %.1f writes/sec(estimated); cost:%.3f(sec); Threads: %d\n",
       count, added, (double)(write_cost / count),
       (double)(count / write_cost),
       write_cost, write_threads);

	printf("|Random-Multi-Thread-Read (done:%ld, found:%ld): %.6f sec/op; %.1f reads/sec(estimated); cost:%.3f(sec); Threads: %d\n",
		count,found, (double)(read_cost / count),
		(double)(count / read_cost),
		read_cost, read_threads);


	
	pthread_mutex_destroy(&write_mutex);
	pthread_mutex_destroy(&read_mutex);
	
	
	free(write_args);
	free(read_args);
	free(write_tid);
	free(read_tid);
	



}

