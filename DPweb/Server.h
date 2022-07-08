#pragma once

#include"Model.h"
//创建一个socket
APP* CreateServerIPV4(ULONG host, int port);
//启动服务
Status RunServer(APP *app);
//监听服务端指令
void* ServerControl(SERVER *server);
//监听客户端请求
void* ClientControl(APP *app);
//处理客户端报文
void* SolveClient(ClientSolver* client);
//判断参数类型是否匹配
int JudgeParam(Method method, RequestText* text);
//处理静态文件请求
int GetStaticSolve(ClientSolver* client, RequestText* text, int methodIndex, char* path);
//处理GET请求
int GetMethodSolve(ClientSolver* client, RequestText* text, int methodIndex);
//发送响应报文
int SolveResponse(Response* orgin, ClientSolver* client);
//获取状态码字符串
char* GetStatusCode(STATUSCODE code);
//拼接响应头的键值
char* GetHeader(char* key, char* value);

