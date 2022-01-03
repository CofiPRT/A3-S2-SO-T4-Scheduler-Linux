#include "so_scheduler.h"
#include "scheduler.h"
#include <stdio.h>

#define SO_INIT_SUCCESS (0)
#define SO_INIT_FAILURE (-1)

#define SO_FORK_FAILURE (INVALID_TID)

#define SO_WAIT_SUCCESS (0)
#define SO_WAIT_FAILURE (-1)

#define SO_SIGNAL_FAILURE (-1)

DECL_PREFIX int so_init(unsigned int time_quantum, unsigned int io)
{
	// preliminary check
	if (io > SO_MAX_NUM_EVENTS || time_quantum <= 0)
		return SO_INIT_FAILURE;

	// already started
	if (sched && sched->started)
		return SO_INIT_FAILURE;

	// attempt to start the scheduler
	if (scheduler_init(time_quantum, io) != INIT_SCHEDULER_SUCCESS)
		return SO_INIT_FAILURE;

	return SO_INIT_SUCCESS;
}

DECL_PREFIX tid_t so_fork(so_handler *func, unsigned int priority)
{
	// preliminary checks
	if (!func || priority > SO_MAX_PRIO)
		return SO_FORK_FAILURE;

	// obtain a new thread to run this function
	thread *new_thread = thread_init(func, priority);

	if (!new_thread)
		return SO_FORK_FAILURE;

	// attempt to start this thread
	if (pthread_create(&new_thread->tid, NULL,
		(void *) thread_routine, (void *) new_thread))
		return SO_FORK_FAILURE;

	scheduler_register(new_thread);

	if (!sched->current_thread) {
		// if no other thread is running, run this newly created one
		scheduler_run(new_thread);
	} else {
		// if another thread is running, schedule it
		new_thread->status = THREAD_STATUS_READY;
		scheduler_push(new_thread);

		// consume a time unit for this calling thread
		so_exec();
	}

	return new_thread->tid;
}

DECL_PREFIX int so_wait(unsigned int io)
{
	thread *current_thread = sched->current_thread;

	// preliminary checks
	if (sched->max_io <= io || !current_thread)
		return SO_WAIT_FAILURE;

	thread *next_thread = scheduler_pop();

	if (!next_thread)
		return SO_WAIT_FAILURE; // this suggests a deadlock, should not happen

	// set the running thread in the waiting state
	current_thread->io = io;
	current_thread->status = THREAD_STATUS_WAITING;
	scheduler_push(current_thread);

	// run the next available thread
	scheduler_run(next_thread);

	// this thread will wait until awaken
	if (sem_wait(&current_thread->semaphore))
		return SO_WAIT_FAILURE;

	return SO_WAIT_SUCCESS;
}

DECL_PREFIX int so_signal(unsigned int io)
{
	if (sched->max_io <= io)
		return SO_SIGNAL_FAILURE;

	unsigned int signaled_threads = 0;

	// wake up every thread waiting on this device
	for (unsigned int i = 0; i < sched->queue_size; i++) {
		thread *queued_thread = sched->queue[i];

		if (queued_thread->io == io) {
			queued_thread->io = THREAD_UNDEFINED_IO;
			queued_thread->status = THREAD_STATUS_READY;
			signaled_threads++;
		}
	}

	thread *current_thread = sched->current_thread;

	if (!current_thread)
		return SO_SIGNAL_FAILURE; // should not happen

	// reschedule this thread
	current_thread->status = THREAD_STATUS_READY;
	scheduler_push(current_thread);

	/**
	 * run the next available thread;
	 * it is possible the current thread will continue running, depending
	 * on its priority
	 */
	scheduler_run(scheduler_pop());

	// wait on this thread
	if (sem_wait(&current_thread->semaphore))
		return SO_SIGNAL_FAILURE;

	return signaled_threads;
}

DECL_PREFIX void so_exec(void)
{
	thread *current_thread = sched->current_thread;

	if (!current_thread)
		return;

	// consume a time unit from the currently running thread
	if (--current_thread->remaining_time > 0)
		return;

	// at this point, the running thread's quantum ended; reset it
	current_thread->remaining_time = sched->quantum;

	// reschedule this thread and run the next available one
	scheduler_push(current_thread);
	scheduler_run(scheduler_pop());

	// this thread now requires awakening
	sem_wait(&current_thread->semaphore);
}

DECL_PREFIX void so_end(void)
{
	if (!sched || !sched->started)
		return; // nonsense

	// join every thread; in case of a failure, continue freeing up memory below
	for (unsigned int i = 0; i < sched->threads_size; i++)
		pthread_join(sched->threads[i]->tid, NULL);

	scheduler_terminate();
}
