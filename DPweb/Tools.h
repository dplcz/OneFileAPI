#pragma once
#include<stdio.h>
#include<stdlib.h>

#include "Model.h"

//报错机制
int merror(int redata, int error, char* showTips);
//获取状态码字符串
char* GetStatusCode(STATUSCODE code);
//拼接响应头的键值
char* GetHeader(char* key, char* value);
//判断文件类型
int JudgeType(char* path);
//获取任务
ClientSolver* GetTask(ThreadPool* pool,int block);
//放入任务
int PutTask(ThreadPool* pool, ClientSolver* task, int block);
//获取线程
Thread* GetThread(ThreadPool* pool);
//获取队列长度
int TaskLength(TaskQueue* queue);
//初始化线程池
void InitThreadPool(ThreadPool* pool);
//初始化线程
void InitThread(Thread* thread, int threadId);
//线程加锁
int GetThreadLock(ThreadLock* lock, int block);
//线程解锁
int ReleaseThreadLock(ThreadLock* lock, int USER);
//显示线程状态
void ShowThreadStatus(ThreadPool pool);
