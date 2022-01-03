#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "thread.h"

#define TRUE	(1)
#define FALSE	(0)

#define SCHEDULER_MAX_CAPACITY (1024)

#define INIT_SCHEDULER_SUCCESS (0)
#define INIT_SCHEDULER_FAILURE (-1)

typedef struct {
	char started;

	thread *current_thread;
	thread **threads;
	thread **queue;

	unsigned int threads_size; // keep track of each thread for memory dealloc
	unsigned int queue_size;

	unsigned int quantum;
	unsigned int max_io;
} scheduler;

extern scheduler *sched;

/**
 * Initialize the scheduler.
 * Returns 0 on success.
 */
int scheduler_init(unsigned int quantum, unsigned int io);

/**
 * Returns the next thread in the queue, and removes it.
 * Returns NULL if the queue is empty.
 */
thread *scheduler_pop(void);

/**
 * Adds a thread to the queue.
 */
void scheduler_push(thread *thread_to_push);

/**
 * Frees up allocated memory and shuts down the scheduler.
 */
void scheduler_terminate(void);

/**
 * Registers a new thread to the scheduler so its memory can be deallocated
 * at shut down.
 */
void scheduler_register(thread *new_thread);

/**
 * Sets the given thread as the current one and runs it.
 * No operation is performed if the given thread is NULL.
 */
void scheduler_run(thread *thread_to_run);

#endif
