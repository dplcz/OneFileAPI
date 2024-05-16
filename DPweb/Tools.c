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

ClientSolver* GetTask(MyThreadPool* pool, int block) {
	ClientSolver* temp;
	
	int taskLength = 0;
	if (block == 1) {
		while (1)
		{
			taskLength = TaskLength(&pool->task);
			if (taskLength == 0) {
				WaitSemaphore(pool->_sem_items);
				WaitSemaphore(pool->_mutex);
				temp = pool->task.taskQueue[pool->task.head];
				pool->task.head = (pool->task.head + 1) % TASK_QUEUE_SIZE;
				PostSemaphore(pool->_mutex);
				PostSemaphore(pool->_sem_blanks);
				return temp;
			}
			else
			{
				WaitSemaphore(pool->_sem_items);
				WaitSemaphore(pool->_mutex);
				temp = pool->task.taskQueue[pool->task.head];
				pool->task.head = (pool->task.head + 1) % TASK_QUEUE_SIZE;
				PostSemaphore(pool->_mutex);
				PostSemaphore(pool->_sem_blanks);
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

int PutTask(MyThreadPool* pool, ClientSolver* task, int block) {
	if (block) {
		int taskLength = TaskLength(&pool->task);
		while (1) {
			if (taskLength == MAX_TASK_COUNT) {
				WaitSemaphore(pool->_sem_blanks);
				WaitSemaphore(pool->_mutex);
				
				pool->task.taskQueue[pool->task.tail] = task;
				pool->task.tail = (pool->task.tail + 1) % TASK_QUEUE_SIZE;

				PostSemaphore(pool->_mutex);
				PostSemaphore(pool->_sem_items);
				return 1;
			}
			else
			{
				WaitSemaphore(pool->_sem_blanks);
				WaitSemaphore(pool->_mutex);

				pool->task.taskQueue[pool->task.tail] = task;
				pool->task.tail = (pool->task.tail + 1) % TASK_QUEUE_SIZE;

				PostSemaphore(pool->_mutex);
				PostSemaphore(pool->_sem_items);
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

#if defined(_WIN32) && defined(DEV)
MyThread CreateMyThread(thread_func_t func, void* args) {
	MyThread thread = CreateThread(NULL, 0, func, args, 0, NULL);
	if (thread == NULL) {
		printf("创建线程失败");
		return 0;
	}
	//printf("创建windows线程");
	return thread;
}
//#elif defined(__linux__)
#else
MyThread CreateMyThread(thread_func_t func, void* args) {
	MyThread thread;
	//printf("创建linux线程");
	pthread_create(&thread, NULL, func, args);
	return thread;
}

#endif // _WIN32




void InitThread(ThreadTask* thread,int threadId) {
	thread->_taskCount = 0;
	thread->_threadId = threadId;
}

void CALLBACK MyWorkCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Parameter, PTP_WORK Work) {
}

#if defined(_WIN32) && defined(DEV)
void InitThreadPool(MyThreadPool* pool, APP* app, thread_func_t func, ThreadArgs* args) {
	pool->_sem_items = CreateSemaphore(NULL, 0, MAX_TASK_COUNT, NULL);
	pool->_sem_blanks = CreateSemaphore(NULL, MAX_TASK_COUNT, MAX_TASK_COUNT, NULL);
	pool->_mutex = CreateSemaphore(NULL, 1, 1, NULL);
	pool->task.head = 0;
	pool->task.tail = 0;

	args[0].app = app;
	args[0].pool = pool;
	args[0].thisThread = NULL;

	InitializeThreadpoolEnvironment(&pool->workcallback);
	pool->pool = CreateThreadpool(NULL);
	SetThreadpoolThreadMaximum(pool->pool, MAX_THREAD_COUNT * 2);
	SetThreadpoolThreadMinimum(pool->pool, MAX_THREAD_COUNT / 2);
	SetThreadpoolCallbackPool(&pool->workcallback, pool->pool);
	TrySubmitThreadpoolCallback(func, args, &pool->workcallback);
	
}
#else
void InitThreadPool(MyThreadPool* pool, APP* app, thread_func_t func, ThreadArgs* args) {
	sem_init(&pool->_sem_items, 0, 0);
	sem_init(&pool->_sem_blanks, 0, MAX_TASK_COUNT);
	sem_init(&pool->_mutex, 0, 1);
	pool->task.head = 0;
	pool->task.tail = 0;
	for (int i = 0; i < MAX_THREAD_COUNT; i++) {
		InitThread(&pool->pool[i], i);
		args[i].app = app;
		args[i].pool = pool;
		args[i].thisThread = &pool->pool[i];
		pool->pool[i]._thread = CreateMyThread(func, &args[i]);
	}
}
#endif

//int GetThreadLock(ThreadLock* lock, int USER) {
//	if (!lock->flag) {
//		printf("\n%d 尝试获取锁", USER);
//		pthread_mutex_lock(&lock->lock);
//		printf("\n%d 获取成功", USER);
//		lock->flag = USER;
//		return 1;
//	}
//	else if (lock->flag != USER && USER != 0) {
//		printf("\n%d 尝试获取锁", USER);
//		pthread_mutex_lock(&lock->lock);
//		printf("\n%d 获取成功", USER);
//		lock->flag = USER;
//		return 1;
//	}
//	return 0;
//}
//int ReleaseThreadLock(ThreadLock* lock, int USER) {
//	if (lock->flag && USER == lock->flag) {
//		lock->flag = 0;
//		int temp = pthread_mutex_unlock(&lock->lock);
//		printf("\n%d 释放锁", USER);
//		return 1;
//	}
//	return 0;
//}

void ShowThreadStatus(MyThreadPool pool) {
	printf("\nNAME\t\tCOUNT");
	//for (int i = 0; i < MAX_THREAD_COUNT; i++) {
	//	printf("\nTh-%d\t\t%d", pool.pool[i]._threadId, pool.pool[i]._taskCount);
	//}
}

#if defined(_WIN32) && defined(DEV)
void WaitSemaphore(Sem sem) {
	WaitForSingleObject(sem, INFINITE);
}
#else
void WaitSemaphore(Sem sem) {
	sem_wait(&sem);
}
#endif // defined(_WIN32) && defined(DEV)

#if defined(_WIN32) && defined(DEV)
void PostSemaphore(Sem sem) {
	ReleaseSemaphore(sem, 1, NULL);
}
#else
void PostSemaphore(Sem sem) {
	sem_post(&sem);
}
#endif // defined(_WIN32) && defined(DEV)

