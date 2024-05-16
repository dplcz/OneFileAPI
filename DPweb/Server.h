#pragma once

#include"Model.h"
//创建一个socket
APP* CreateServerIPV4(ULONG host, int port);
//启动服务
Status RunServer(APP *app, int pool_flag);
//监听服务端指令
void* ServerControl(APP* app);
//监听客户端请求
void* ClientControl(ClientControlArgs clientArgs);
#if defined(_WIN32)&&defined(DEV)
void* PreSolveClient(PTP_CALLBACK_INSTANCE Instance, ClientThreadArgs args, PTP_WORK Work);
#endif // defined(_WIN32)&&defined(DEV)

//处理客户端报文
void* SolveClient(ClientSolver* client, int threadId);
//判断参数类型是否匹配
int JudgeParam(Method method, RequestText* text);
//处理静态文件请求
int GetStaticSolve(ClientSolver* client, RequestText* text, int methodIndex, char* path);
//处理GET请求
int GetMethodSolve(ClientSolver* client, RequestText* text, int methodIndex);
//处理POST请求
int PostMethodSolve(ClientSolver* client, RequestText* text, int methodIndex, char* postData);
//发送响应报文
int SolveResponse(Response* orgin, ClientSolver* client);
//线程池监听任务
void ThreadListen(ThreadArgs* args);
