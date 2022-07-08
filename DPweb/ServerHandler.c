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
	app->requestM.method_len = 0;
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
		ClientSolver* temp = (ClientSolver*)malloc(sizeof(ClientSolver));
		temp->client = client;
		temp->method = app->requestM;
		pthread_t th;
		pthread_create(&th, NULL, SolveClient, temp);
	}
}

void* SolveClient(ClientSolver* client) {
	//是否处理成功
	int flag = 0;
	char recvdata[1024 * 5] = "";
	//接收数据，默认为空
	recv(client->client, recvdata, sizeof(recvdata), 0);	
	//参数一：接收消息的来源
	//参数二：接收消息的指针
	//参数三：接收消息的指针的内存大小
	//参数四：0表示默认的收发方式，一次性收完，等待流传输结束后一次收取

	printf("共接受%d字节数据\n\n", strlen(recvdata));
	RequestText text;
	char* temp = NULL;
	//分解报文头
	char* temp_head = strtok_s(recvdata, "\r\n", &temp);
	text.head.method = strtok_s(NULL, " ", &temp_head);
	text.head.url = strtok_s(NULL, " ", &temp_head);
	text.head.version = temp_head;
	//分解请求参数
	text.param_len = 0;
	char* param_1 = NULL;
	char* param_2 = NULL;
	text.head.url = strtok_s(text.head.url, "?", &param_1);
	param_2 = param_1;
	while (param_2 != NULL && strcmp(param_2, "") != 0)
	{
		
		if (text.param_len == 0)
			//注意释放内存
			text.params = (PARAMS*)malloc(sizeof(PARAMS));
		else
		{
			PARAMS* tmp = NULL;
			tmp = (PARAMS*)realloc(text.params, sizeof(PARAMS) * (text.param_len + 1));
			if (tmp != NULL)
				text.params = tmp;
			else
			{
				printf("内存分配失败！");
				free(text.params);
				exit(-1);
			}
		}
		param_1 = strtok_s(param_1, "&", &param_2);
		text.params[text.param_len].name = strtok_s(NULL,"=",&param_1);
		text.params[text.param_len].data = param_1;
		printf("key: %s value:%s", text.params[text.param_len].name, text.params[text.param_len].data);
		text.param_len++;
		param_1 = NULL;
	} 
	text.headers_len = 0;
	text.headers = NULL;
	char* header = NULL;
	char* value = NULL;
	//分解请求头
	do
	{
		if (text.headers_len == 0)
			//注意释放内存
			text.headers = (Header*)malloc(sizeof(Header));
		else
		{
			Header* tmp = NULL;
			tmp = (Header*)realloc(text.headers, sizeof(Header) * (text.headers_len + 1));
			if (tmp != NULL)
				text.headers = tmp;
			else
			{
				printf("内存分配失败！");
				free(text.headers);
				exit(-1);
			}
		}
		header = strtok_s(NULL, "\r\n", &temp);
		header = strtok_s(header, ":", &value);
		text.headers[text.headers_len].key = header;
		text.headers[text.headers_len].value = value;
		text.headers_len++;
	} while (header != NULL);
	if (strcmp(text.head.method, "GET") == 0)
		for (int i = 0; i < client->method.method_len; i++) {
			if (client->method.methods[i].type == GET && strcmp(client->method.methods[i].uri, text.head.url) == 0)
				if (JudgeParam(client->method.methods[i], &text))
					flag = GetMethodSolve(client, &text, i);
		}
	else if (strcmp(text.head.method, "POST") == 0) {

	}
	char dont_allow[] = "HTTP/1.1 404 OK\r\n\r\nNot Found";
	if (!flag)
	{
		send(client->client, dont_allow, strlen(dont_allow), 0);
	}
	closesocket(client->client);
	free(text.params);
	free(text.headers);
	free(client);

}

int JudgeParam(Method method, RequestText* text) {
	int num = 0;
	for (int i = 0; i < method.param_len; i++)
		for (int k = 0; k < text->param_len; k++)
			if (strcmp(method.params[i].name, text->params[k].name) == 0)
				num++;
	if (num >= method.param_len)
		return 1;
	return 0;
}

int GetMethodSolve(ClientSolver* client, RequestText* text,int methodIndex) {
	Response *res = (Response*)client->method.methods[methodIndex].callback(text->params);
	printf("666");
	free(res->headers);
	free(res);
	return 1;
}
