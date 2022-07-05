#pragma once
#include<stdio.h>
#include<stdlib.h>
#include<WinSock2.h>
#include<WS2tcpip.h>

//导入Socket包----------------
#pragma comment(lib,"ws2_32.lib")
//定义最大连接队列
#define MAX_CONNECTION_QUEUE 8

typedef struct sockaddr sockaddr;

//报错机制
int merror(int redata, int error, char* showTips);

//创建一个socket
SOCKET createServer(WSADATA wsaData, int v1, int v2, int internetwork, int transmission, int protocol);

//确定socket版本信息，存放windows socket初始化信息
WSADATA wsa_data;








