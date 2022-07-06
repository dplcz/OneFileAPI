#pragma once
#include<stdio.h>
#include<stdlib.h>
#include<WinSock2.h>
#include<WS2tcpip.h>
#include<pthread.h>
#include"Model.h"
#include"Tools.h"

//导入Socket包----------------
#pragma comment(lib,"ws2_32.lib")
//导入多线程包
#pragma comment(lib,"pthreadVC2.lib")
//定义最大连接队列
#define MAX_CONNECTION_QUEUE 8

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in claddr;

typedef struct SERVER {
	SOCKET server;
	Status flag;
}SERVER;

typedef struct app {
	SERVER *serverM;
	claddr client;
	int cllen;
}APP;

//创建一个socket
APP* CreateServerIPV4(ULONG host, int port);
//启动服务
Status RunServer(APP *app);
//监听服务端指令
void* ServerControl(SOCKET *server);
//监听客户端请求
void* ClientControl(APP *app);


