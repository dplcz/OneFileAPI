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

void* SolveClient(ClientSolver* client);


