#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "threadpool.h"



int main (int argc, char* argv[]) {

	threadpool_t pool;


	threadpool_init(&pool);
	
	
	for (int i = 0; i < 100000; ++i) {

		int* num = malloc(sizeof(int));
		*num = i;

		threadpool_add_task(&pool, example_task, (void*) num);

	}

	sleep(10); // to ensure that all tasks are completed

	threadpool_destroy(&pool);

	return 0;
}