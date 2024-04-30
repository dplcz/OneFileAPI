#pragma once
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<WinSock2.h>
#include<WS2tcpip.h>
#include<pthread.h>
#include<semaphore.h>
#include<mysql/mysql.h>
#define ERROR -1
#define OK 1
#define CONSUMER 1
#define PRODUCER 2
//导入MySQL包
#pragma comment(lib,"libmysql")
//导入Socket包----------------
#pragma comment(lib,"ws2_32.lib")
//导入多线程包
#pragma comment(lib,"pthreadVC2.lib")

//定义最大连接队列
#define MAX_CONNECTION_QUEUE 128

//定义最大接口数
#define MAX_API_COUNT 10
//定义线程池大小
#define MAX_THREAD_COUNT 24
//定义单个线程最大任务数量
#define MAX_TASK_COUNT 5000


//定义状态
typedef int Status;

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in claddr;

//请求方式
typedef enum { GET = 1000, POST = 1100, HEAD = 1200 }METHODTYPE;
//响应方式
typedef enum { TEXT = 0, HTML = 10, XML = 20, JS = 30, JSON = 40, CSS = 50,  JPG = 60, JPEG = 60, JPE = 60, JFIF = 60, PNG = 70 }RESPONSETYPE;
//响应状态码
typedef enum { SUCCESS = 200, CREATED = 201, BADREQUEST = 400, NOTFOUND = 404 }STATUSCODE;

typedef char* URI;
typedef char* NAME;
typedef char* DATA;
typedef char* DEFAULT;

typedef struct SERVER {
	SOCKET server;
	Status flag;
}SERVER;

//接收参数
typedef struct PARAMS {
	NAME name;
	DATA data;
	DEFAULT default_data;
}PARAMS;

//回调函数
typedef void*(*CALLBACKFUNC)(PARAMS* args);

//方法结构体
typedef struct Method {
	METHODTYPE type;
	URI uri;
	CALLBACKFUNC callback;
	PARAMS *params;
	char* directory;
	int param_len;
	int if_static;
}Method;

//方法列表结构体
typedef struct RequestMethod {
	Method methods[MAX_API_COUNT];
	int method_len;
}RequestMethod;

typedef struct ClientSolver {
	SOCKET* client;
	RequestMethod method;
}ClientSolver;


typedef struct ThreadLock {
	pthread_mutex_t lock;
	int flag;
}ThreadLock;
//任务队列
typedef struct TaskQueue {
	ClientSolver* taskQueue[MAX_TASK_COUNT + 1];
	int head;
	int tail;
}TaskQueue;
//线程
typedef struct Thread{
	pthread_t thread;
	TaskQueue task;
}Thread;

//线程池
typedef struct ThreadPool{
	pthread_t pool[MAX_THREAD_COUNT];
	TaskQueue task;
	ThreadLock lock;
	pthread_cond_t cond;
	sem_t sem_items;
	sem_t sem_blanks;
	sem_t mutex;
}ThreadPool;

typedef struct app {
	SERVER* serverM;
	claddr client;
	int cllen;
	RequestMethod requestM;
}APP;

//线程池参数
typedef struct ThreadArgs {
	APP* app;
	ThreadPool* pool;
	int threadId;
}ThreadArgs;

//ClientControl线程参数
typedef struct ClientControlArgs {
	APP* app;
	int pool_flag;
}ClientControlArgs;

typedef struct Header {
	char* key;
	char* value;
}Header;

typedef struct MainHeader {
	char* method;
	char* url;
	char* version;
}MainHeader;

typedef struct Data {
	char name[20];
	char filename[50];
	char type[50];
	char data[50 * 1024];
}Data;

typedef struct PostData {
	char* boundary;
	Data* data;
	int length;
}PostData;

typedef struct Request {
	MainHeader head;
	Header *headers;
	int headers_len;
	PARAMS *params;
	int param_len;
	PostData post_data;
}RequestText;

typedef struct ResData {
	int is_file;
	char* data;
}ResData;

//响应
typedef struct Response {
	RESPONSETYPE type;
	STATUSCODE code;
	Header *headers;
	int headers_len;
	ResData data;
}Response;



