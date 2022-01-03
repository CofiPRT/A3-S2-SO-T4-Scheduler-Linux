#include "thread.h"
#include "scheduler.h"
#include <stdlib.h>
#include <stdio.h>

thread *thread_init(so_handler *func, unsigned int priority)
{
	// fast fail checks
	thread *new_thread = malloc(sizeof(thread));

	if (!new_thread)
		return NULL;

	if (sem_init(&new_thread->semaphore, 0, 0)) {
		free(new_thread);
		return NULL;
	}

	// easy to set fields
	new_thread->io = THREAD_UNDEFINED_IO;
	new_thread->tid = INVALID_TID;
	new_thread->handler = func;
	new_thread->status = THREAD_STATUS_NEW;
	new_thread->priority = priority;
	new_thread->remaining_time = sched->quantum;

	return new_thread;
}

void thread_routine(void *args)
{
	thread *given_thread = (thread *) args;

	// wait for this thread to be scheduled
	if (sem_wait(&given_thread->semaphore))
		return;

	// run the handler
	given_thread->handler(given_thread->priority);

	// at this step, the thread has finished its execution
	given_thread->status = THREAD_STATUS_TERMINATED;

	// nothing to do with this thread anymore, attempt to run another one
	scheduler_run(scheduler_pop());
}
