#include <arena.h>
#include <assert.h>

#include "../../include/task.h"
#include "../../include/thread_safe_queue.h"
#include "../../include/thread.h"

typedef struct
{
	ThreadSafeQueue *queue;
	Arena arena;
	size_t groupCapacity;
} TaskBroker;

static TaskBroker broker;

static void _cleanup(Arena *a);

void broker_init(ThreadSafeQueue *queue, size_t taskArenaCapacity)
{
	broker.arena = a_new(sizeof(TaskGroup));
	broker.queue = queue;
	broker.groupCapacity = taskArenaCapacity;
}

void broker_commit(TaskGroup **tg)
{
	assert(*tg);
	enqueue(broker.queue, *tg, &taskQueueLock, &taskQueueCondition);

	*tg = NULL;
}

TaskGroup * broker_create_group()
{
	TaskGroup *tg = a_alloc(&broker.arena, sizeof(TaskGroup), __alignof(TaskGroup));
	
	assert(tg);

	tg->a = a_new(broker.groupCapacity);
	tg->head = NULL;
	tg->tail = NULL;
	tg->cleanup = _cleanup;

	return tg;
}

TaskGroup * broker_read()
{
	TaskGroup *tg = peek(broker.queue, &taskQueueLock, &taskQueueCondition);

	dequeue(broker.queue, &taskQueueLock, &taskQueueCondition);

	return tg;
}

void broker_cleanup()
{
	a_free(&broker.arena);
}

static void _cleanup(Arena *a)
{
	a_free(a);

	if (broker.arena.region->next) r_free_head(&broker.arena);
}
