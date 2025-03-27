#include "bench.h"

 //oxi sto kiwi.c giati fault

DB* db;
pthread_mutex_t readwrite_mutex;
pthread_mutex_t compaction_mutex;
pthread_cond_t readwrite_cond;
int writers; //gia na kseroume an kapoios grafei kai posoi grfoun
int readers;	
//NEW METHODS NA SXOLIASTOUN
void initialize_mutexes() {
    pthread_mutex_init(&readwrite_mutex, NULL);
	pthread_mutex_init(&compaction_mutex, NULL);
    pthread_cond_init(&readwrite_cond, NULL);
       
}

void destroy_mutexes() {
    pthread_mutex_destroy(&readwrite_mutex);
    pthread_mutex_destroy(&compaction_mutex);
    pthread_cond_destroy(&readwrite_cond);
}

void _random_key(char *key,int length) {
	int i;
	char salt[36]= "abcdefghijklmnopqrstuvwxyz0123456789";

	for (i = 0; i < length; i++)
		key[i] = salt[rand() % 36];
}

void _print_header(int count)
{
	double index_size = (double)((double)(KSIZE + 8 + 1) * count) / 1048576.0;
	double data_size = (double)((double)(VSIZE + 4) * count) / 1048576.0;

	printf("Keys:\t\t%d bytes each\n", 
			KSIZE);
	printf("Values: \t%d bytes each\n", 
			VSIZE);
	printf("Entries:\t%d\n", 
			count);
	printf("IndexSize:\t%.1f MB (estimated)\n",
			index_size);
	printf("DataSize:\t%.1f MB (estimated)\n",
			data_size);

	printf(LINE1);
}

void _print_environment()
{
	time_t now = time(NULL);

	printf("Date:\t\t%s", 
			(char*)ctime(&now));

	int num_cpus = 0;
	char cpu_type[256] = {0};
	char cache_size[256] = {0};

	FILE* cpuinfo = fopen("/proc/cpuinfo", "r");
	if (cpuinfo) {
		char line[1024] = {0};
		while (fgets(line, sizeof(line), cpuinfo) != NULL) {
			const char* sep = strchr(line, ':');
			if (sep == NULL || strlen(sep) < 10)
				continue;

			char key[1024] = {0};
			char val[1024] = {0};
			strncpy(key, line, sep-1-line);
			strncpy(val, sep+1, strlen(sep)-1);
			if (strcmp("model name", key) == 0) {
				num_cpus++;
				strcpy(cpu_type, val);
			}
			else if (strcmp("cache size", key) == 0)
				strncpy(cache_size, val + 1, strlen(val) - 1);	
		}

		fclose(cpuinfo);
		printf("CPU:\t\t%d * %s", 
				num_cpus, 
				cpu_type);

		printf("CPUCache:\t%s\n", 
				cache_size);
	}
}

int main(int argc,char** argv)
{
	writers = 0;
	long int count;
	int threads;
	int write_percentage;
	initialize_mutexes();
	

	srand(time(NULL));
	if (argc < 3) {
		fprintf(stderr,"Usage: db-bench <write | read> <count>\n");
		exit(1);
	}
	
	if (strcmp(argv[1], "write") == 0) {
		int r = 0;

		count = atoi(argv[2]);
		_print_header(count);
		_print_environment();
		if (argc == 4)
			r = 1;
		_write_test(count, r);
	} else if (strcmp(argv[1], "read") == 0) {
		int r = 0;

		count = atoi(argv[2]);
		_print_header(count);
		_print_environment();
		if (argc == 4)
			r = 1;
		
		_read_test(count, r);
	} else if (strcmp(argv[1], "multi_write") == 0) {
		
		if (argc < 4) {
			fprintf(stderr,"Usage: db-bench <multi_write | multi_read> <count> <number of threads>\n");
			exit(1);
		}
		
		int r = 0;
		


		count = atoi(argv[2]);
		threads = atoi(argv[3]);
		_print_header(count);
		_print_environment();
		if (argc == 5)
			r = 1;
		db = db_open(DATAS);
	
		multi_write_test(count, r, threads);

		//db_close(db);
		
		
	} 
	
	else if (strcmp(argv[1], "multi_read") == 0) {
		if (argc < 4) {
			fprintf(stderr,"Usage: db-bench <multi_write | multi_read> <count> <number of threads>\n");
			exit(1);
		}
		
		int r = 0;
		


		count = atoi(argv[2]);
		threads = atoi(argv[3]);
		_print_header(count);
		_print_environment();
		if (argc == 5)
			r = 1;
		db = db_open(DATAS);
	
		multi_read_test(count, r, threads);
	}

	else if (strcmp(argv[1], "readwrite") == 0) { //keep the write percentage as input
		if (argc < 5) {
			fprintf(stderr,"Usage: db-bench <readwrite> <count> <number of threads> <write percentage>\n");
			exit(1);
		}
		
		int r = 0;

		count = atoi(argv[2]);
		threads = atoi(argv[3]);
		write_percentage = atoi(argv[4]);
		

		
		_print_header(count);
		_print_environment();
		if (argc == 6)
			r = 1;
		
		db = db_open(DATAS);

		readwrite_test(count, r, threads, write_percentage);




	}
	
	
	
	
	else {
		fprintf(stderr,"Usage: db-bench <write | read> <count> <random>\n");
		exit(1);
	}

	destroy_mutexes();

	return 1;
}
