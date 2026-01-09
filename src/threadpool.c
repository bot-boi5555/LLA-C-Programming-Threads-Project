#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

//#include "threadpool.h"

#include <pthread.h>

#define THREADS 16
#define QUEUE_SIZE 100

typedef struct {
	void (*fn)(void* arg);
	void* arg;

} task_t;

typedef struct {
	pthread_mutex_t lock;
	pthread_cond_t notify;
	pthread_t threads[THREADS];
	task_t task_queue[QUEUE_SIZE];
	int queued;
	int queue_front;
	int queue_back;
	int stop;

} threadpool_t;

void* threadpool_function(void* threadpool) {

	threadpool_t* pool = (threadpool_t*) threadpool;

	if (pool == NULL) {
		return NULL;
	}

	while(true) {

		pthread_mutex_lock(&(pool->lock)); // lock the other threads to avoid unwanted modifications

		// case 1: no tasks in queue
		while(pool->queued == 0 && !pool->stop) {
			pthread_cond_wait(&(pool->notify), &(pool->lock)); // wait until the condition var is true
		}

		// case 2: the pool eneded
		if (pool->stop) {
			pthread_mutex_unlock(&(pool->lock)); // unlock the mutex to allow other threads to exit the pool (prevents deadlock)
			pthread_exit(NULL); 
		}
		
		// case 3: there is a task in the queue
		task_t task = pool->task_queue[pool->queue_front];

		pool->queue_front = (pool->queue_front + 1) % QUEUE_SIZE;
		pool->queued--;

		pthread_mutex_unlock(&(pool->lock)); 

		(*(task.fn))(task.arg); // de-reference the function pointer and run the function with arg

	}
	
}


void threadpool_init(threadpool_t* pool) {

	
	if (pool == NULL) {
		return;
	}

	pool->queued = 0;
	pool->queue_back = 0;
	pool->queue_front = 0;
	pool->stop = 0;

	pthread_mutex_init(&(pool->lock), NULL);
	pthread_cond_init(&(pool->notify), NULL);

	for (int i = 0; i < THREADS; ++i) {
		pthread_create(&(pool->threads[i]), NULL, threadpool_function, (void*) pool);
	}

}


void threadpool_destroy(threadpool_t* pool) {

	
	if (pool == NULL) {
		return;
	}

	pthread_mutex_lock(&(pool->lock));
	
	pool->stop = true;
	pthread_cond_broadcast(&(pool->notify)); // ensures idle threads are stopped

	pthread_mutex_unlock(&(pool->lock));

	for (int i = 0; i < THREADS; ++i) {
		pthread_join(pool->threads[i], NULL);
	}

	pthread_cond_destroy(&(pool->notify));
	pthread_mutex_destroy(&(pool->lock));

}


void threadpool_add_task(threadpool_t* pool, void (*function)(void*), void* arg) {

	
	if (pool == NULL || function == NULL || pool->stop) {
		return;
	}

	pthread_mutex_lock(&(pool->lock));

	if (pool->queued <= QUEUE_SIZE) {

		pool->task_queue[pool->queue_back].fn = function;
		pool->task_queue[pool->queue_back].arg = arg;
		
		pool->queue_back = (pool->queue_back + 1) % QUEUE_SIZE;
		pool->queued++;

		pthread_cond_signal(&(pool->notify));
	
	}
	else {
		printf("Failed to add task (Task queue is full).\n");
	}

	pthread_mutex_unlock(&(pool->lock));

}


void example_task(void* arg) {

	if (arg == NULL) {
		return;
	}
	
	int* numPtr = (int*) arg;

	printf("Processing task %d.\n", *numPtr);
	sleep(1);

	free(numPtr);
}
