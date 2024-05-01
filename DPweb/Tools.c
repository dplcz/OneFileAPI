#include "Tools.h"

const int TASK_QUEUE_SIZE = MAX_TASK_COUNT + 1;

int merror(int redata, int error, char* showTips) {
	if (redata == error)
	{
		perror(showTips);
		printf("\n");
		getchar();
		exit(MERROR);
	}
	return 1;
}

char* GetStatusCode(STATUSCODE code) {
	switch (code)
	{
	case SUCCESS:
		return "HTTP/1.1 200 OK\r\n";
		break;
	case CREATED:
		return "HTTP/1.1 201 Created\r\n";
		break;
	case BADREQUEST:
		return "HTTP/1.1 400 Bad Request\r\n";
		break;
	case NOTFOUND:
		return "HTTP/1.1 404 Not Found\r\n";
		break;
	default:
		break;
	}
}

char* GetHeader(char* key, char* value) {
	char headers[1024] = "";
	char temp[] = ": ";
	strcat_s(headers, 1024, key);
	strcat_s(headers, 1024, temp);
	strcat_s(headers, 1024, value);
	strcat_s(headers, 1024, "\r\n");
	return headers;
}

int JudgeType(char* path) {
	char* file_type = strstr(path, ".jpeg");
	if (file_type != NULL)
		return 1;
	file_type = strstr(path, ".jpe");
	if (file_type != NULL)
		return 1;
	file_type = strstr(path, ".jpg");
	if (file_type != NULL)
		return 1;
	file_type = strstr(path, ".jfif");
	if (file_type != NULL)
		return 1;
	file_type = strstr(path, ".png");
	if (file_type != NULL)
		return 2;
	file_type = strstr(path, ".html");
	if (file_type != NULL)
		return 3;
	file_type = strstr(path, ".js");
	if (file_type != NULL)
		return 4;
	file_type = strstr(path, ".css");
	if (file_type != NULL)
		return 5;
	file_type = strstr(path, ".xml");
	if (file_type != NULL)
		return 6;
	return 0;
}

ClientSolver* GetTask(ThreadPool* pool, int block) {
	ClientSolver* temp;
	int taskLength = 0;
	if (block == 1) {
		while (1)
		{
			taskLength = TaskLength(&pool->task);
			if (taskLength == 0) {
				/*if (ReleaseThreadLock(&pool->lock, CONSUMER))
					printf("\n1 释放锁");*/
				sem_wait(&pool->_sem_items);
				sem_wait(&pool->_mutex);
				//pthread_cond_wait(&pool->cond, &pool->lock);
				//if (GetThreadLock(&pool->lock, CONSUMER))
				//	printf("");
				temp = pool->task.taskQueue[pool->task.head];
				pool->task.head = (pool->task.head + 1) % TASK_QUEUE_SIZE;
				//sem_post(&queue->sem_out);
				//if (ReleaseThreadLock(&pool->lock, CONSUMER))
				//	printf("");
				sem_post(&pool->_mutex);
				sem_post(&pool->_sem_blanks);
				return temp;
				//pthread_cond_wait(&queue->cond, &queue->lock);
				//printf("\n释放锁");
				//pthread_mutex_unlock(&queue->lock_in);
			}
			else
			{
				//if (GetThreadLock(&pool->lock, CONSUMER))
				//	printf("");
				//pthread_mutex_lock(&queue->lock_out);
				sem_wait(&pool->_sem_items);
				sem_wait(&pool->_mutex);
				temp = pool->task.taskQueue[pool->task.head];
				pool->task.head = (pool->task.head + 1) % TASK_QUEUE_SIZE;
				//if (ReleaseThreadLock(&pool->lock, CONSUMER))
				//	printf("");
				sem_post(&pool->_mutex);
				sem_post(&pool->_sem_blanks);
				//pthread_mutex_unlock(&queue->lock_out);
				return temp;
			}
		}
	}
	else
	{
		if (TaskLength(&pool->task) == 0)
			return NULL;
		else
		{
			temp = pool->task.taskQueue[pool->task.head];
			pool->task.head = (pool->task.head + 1) % TASK_QUEUE_SIZE;
			return temp;
		}
	}
}

int PutTask(ThreadPool* pool, ClientSolver* task, int block) {
	if (block) {
		int taskLength = TaskLength(&pool->task);
		while (1) {
			if (taskLength == MAX_TASK_COUNT) {
				sem_wait(&pool->_sem_blanks);
				sem_wait(&pool->_mutex);
				
				pool->task.taskQueue[pool->task.tail] = task;
				pool->task.tail = (pool->task.tail + 1) % TASK_QUEUE_SIZE;

				sem_post(&pool->_mutex);
				sem_post(&pool->_sem_items);
				return 1;
			}
			else
			{
				sem_wait(&pool->_sem_blanks);
				sem_wait(&pool->_mutex);

				pool->task.taskQueue[pool->task.tail] = task;
				pool->task.tail = (pool->task.tail + 1) % TASK_QUEUE_SIZE;

				sem_post(&pool->_mutex);
				sem_post(&pool->_sem_items);
				return 1;
			}
		}
	}
	else
	{
		if (TaskLength(&pool->task) == MAX_TASK_COUNT)
			return 0;
		else
		{
			pool->task.taskQueue[pool->task.tail] = task;
			pool->task.tail = (pool->task.tail + 1) % TASK_QUEUE_SIZE;
			return 1;
		}
	}
}

//Thread *GetThread(ThreadPool* pool) {
//	Thread* temp;
//	temp = &pool->pool[pool->index];
//	pool->index = (pool->index + 1) % MAX_THREAD_COUNT;
//	return temp;
//}

int TaskLength(TaskQueue* queue) {
	int count = (queue->tail - queue->head + TASK_QUEUE_SIZE) % TASK_QUEUE_SIZE;
	//if(count<10
	//printf("%d", count);
	return count;
}


void InitThread(Thread* thread,int threadId) {
	thread->_taskCount = 0;
	thread->_threadId = threadId;
}

void InitThreadPool(ThreadPool* pool) {
	pool->_lock.lock = PTHREAD_MUTEX_INITIALIZER;
	pool->_lock.flag = 0;
	pool->_cond = PTHREAD_COND_INITIALIZER;
	sem_init(&pool->_sem_items, 0, 0);
	sem_init(&pool->_sem_blanks, 0, MAX_TASK_COUNT);
	sem_init(&pool->_mutex, 0, 1);
	pool->task.head = 0;
	pool->task.tail = 0;
	for (int i = 0; i < MAX_THREAD_COUNT; i++)
		InitThread(&pool->pool[i], i);
}

int GetThreadLock(ThreadLock* lock, int USER) {
	if (!lock->flag) {
		printf("\n%d 尝试获取锁", USER);
		pthread_mutex_lock(&lock->lock);
		printf("\n%d 获取成功", USER);
		lock->flag = USER;
		return 1;
	}
	else if (lock->flag != USER && USER != 0) {
		printf("\n%d 尝试获取锁", USER);
		pthread_mutex_lock(&lock->lock);
		printf("\n%d 获取成功", USER);
		lock->flag = USER;
		return 1;
	}
	return 0;
}
int ReleaseThreadLock(ThreadLock* lock, int USER) {
	if (lock->flag && USER == lock->flag) {
		lock->flag = 0;
		int temp = pthread_mutex_unlock(&lock->lock);
		printf("\n%d 释放锁", USER);
		return 1;
	}
	return 0;
}

void ShowThreadStatus(ThreadPool pool) {
	printf("\nNAME\t\tCOUNT");
	for (int i = 0; i < MAX_THREAD_COUNT; i++) {
		printf("\nTh-%d\t\t%d", pool.pool[i]._threadId, pool.pool[i]._taskCount);
	}
}