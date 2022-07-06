#include "Server.h"
#include "Model.h"
#include "Tools.h"


APP* CreateServerIPV4(ULONG host, int port) {
	/// <summary>
	/// 默认配置
	/// IPV4 TCP 流传输
	/// 
	/// host
	/// INADDR_ANY	监听所有
	/// INADDR_LOOPBACK	监听本地
	/// </summary>
	/// <param name="addr">绑定ip地址</param>
	/// <param name="port"></param>
	/// <returns></returns>

	WSADATA wsa_data;
	// 服务端
	struct sockaddr_in seraddr;
	//异步套接字启动函数
	int isok = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	merror(isok, WSAEINVAL, "socket请求失败\n");
	// 默认配置
	// IPV4 TCP 流传输
	SOCKET server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	merror(server, INVALID_SOCKET, "socket创建失败\n");
	// IPV4
	seraddr.sin_family = AF_INET;
	// 绑定端口
	seraddr.sin_port = htons(port);
	// 绑定ip地址
	seraddr.sin_addr.s_addr = (host);
	// 绑定函数
	isok = bind(server, (sockaddr*)&seraddr, sizeof(seraddr));
	merror(isok, SOCKET_ERROR, "bind绑定信息失败\n");

	APP* app = (APP*)malloc(sizeof(APP));
	app->serverM = (SERVER*)malloc(sizeof(SERVER));

	//监听客户端，第一个参数为监听者，第二个连接请求队列的最大长度
	isok = listen(server, MAX_CONNECTION_QUEUE);
	app->serverM->server= server;
	app->serverM->flag = 1;
	app->cllen = sizeof(app->client);
	return app;
}

Status RunServer(APP *app) {
	pthread_t th_server,th_client;
	pthread_create(&th_server, NULL, ServerControl, app->serverM);
	pthread_create(&th_client, NULL, ClientControl, app);
	while (1);

}

void* ServerControl(SERVER* serverM) {
	while (1) {
		char temp[20];
		scanf_s("%s",temp,20);
		if (strcmp(temp, "stop") == 0 || strcmp(temp, "STOP") == 0)
		{
			closesocket(serverM->server);
			serverM->flag = 0;
		}
	}
}

void* ClientControl(APP* app) {
	while (1) {
		printf("等待连接中...");
		SOCKET client = accept(app->serverM->server, (sockaddr*)&app->client, &app->cllen);
		if (app->serverM->flag == 0) {
			closesocket(client);
			free(app->serverM);
			free(app);
			exit(0);
		}
		//参数一：表示谁接受连接
		//参数二：谁连接进来
		//参数三：用来保存信息的结构体大小
		//返回值为连接进来的 socket
		merror(client, INVALID_SOCKET, "连接失败\n");
		printf("连接成功\n");
		/*pthread_t th;
		pthread_create(&th, NULL, solve_client, client);*/
		printf("1");
		closesocket(client);
	}
}
