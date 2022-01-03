#include "scheduler.h"
#include <stdlib.h>
#include <stdio.h>

scheduler *sched = NULL;

int scheduler_init(unsigned int quantum, unsigned int io)
{
	sched = malloc(sizeof(scheduler));
	if (!sched)
		return INIT_SCHEDULER_FAILURE;

	sched->queue = malloc(sizeof(thread *) * SCHEDULER_MAX_CAPACITY);
	if (!sched->queue) {
		free(sched);
		return INIT_SCHEDULER_FAILURE;
	}

	sched->threads = malloc(sizeof(thread *) * SCHEDULER_MAX_CAPACITY);
	if (!sched->threads) {
		free(sched);
		free(sched->queue);
		return INIT_SCHEDULER_FAILURE;
	}

	sched->threads_size = 0;
	sched->queue_size = 0;
	sched->started = TRUE;
	sched->quantum = quantum;
	sched->max_io = io;
	sched->current_thread = NULL;

	return INIT_SCHEDULER_SUCCESS;
}

thread *scheduler_pop(void)
{
	// only look for threads that are not in the waiting state
	for (unsigned int i = 0; i < sched->queue_size; i++) {
		thread *thread_to_return = sched->queue[i];

		if (thread_to_return->status != THREAD_STATUS_WAITING) {
			// shift the queue
			for (unsigned int j = i; j < sched->queue_size - 1; j++)
				sched->queue[j] = sched->queue[j + 1];

			sched->queue[--sched->queue_size] = NULL;

			return thread_to_return;
		}
	}

	// when the queue is empty
	return NULL;
}

void scheduler_push(thread *thread_to_push)
{
	// search the thread's place in the queue based on its priority
	unsigned int index;

	for (index = 0; index < sched->queue_size; index++) {
		if (thread_to_push->priority > sched->queue[index]->priority)
			break;
	}

	// shift the other elements to make room for this thread
	for (unsigned int i = sched->queue_size; i > index; i--)
		sched->queue[i] = sched->queue[i - 1];

	sched->queue[index] = thread_to_push;
	sched->queue_size++;
}

void scheduler_terminate(void)
{
	for (unsigned int i = 0; i < sched->threads_size; i++) {
		// destroy the semaphore; in case of an error, don't stop freeing memory
		sem_destroy(&sched->threads[i]->semaphore);

		// free the thread pointer
		free(sched->threads[i]);
	}

	// free up remaining arrays and poitners
	free(sched->threads);
	free(sched->queue);
	free(sched);

	sched = NULL;
}

void scheduler_register(thread *new_thread)
{
	sched->threads[sched->threads_size++] = new_thread;
}

void scheduler_run(thread *thread_to_run)
{
	if (!thread_to_run)
		return;

	// set the thread's status
	sched->current_thread = thread_to_run;
	thread_to_run->status = THREAD_STATUS_RUNNING;

	// allow the thread to run
	sem_post(&thread_to_run->semaphore);
}