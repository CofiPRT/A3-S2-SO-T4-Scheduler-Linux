#ifndef THREAD_
#define THREAD_

#include "so_scheduler.h"
#include <semaphore.h>

// thread status codes
#define THREAD_STATUS_NEW			(0)
#define THREAD_STATUS_READY			(1)
#define THREAD_STATUS_RUNNING		(2)
#define THREAD_STATUS_WAITING		(3)
#define THREAD_STATUS_TERMINATED	(4)

#define THREAD_UNDEFINED_IO	(SO_MAX_NUM_EVENTS + 1)

typedef struct {
	tid_t tid;
	sem_t semaphore;

	so_handler *handler;

	unsigned int io;
	int status;
	int priority;
	int remaining_time;
} thread;

/**
 * Constructs a new thread.
 * Returns NULL on failure.
 */
thread *thread_init(so_handler *func, unsigned int priority);

/**
 * A routine to be executed whenever 'pthread_create' is called
 * to start a thread. Not to be called manually.
 */
void thread_routine(void *args);

#endif
