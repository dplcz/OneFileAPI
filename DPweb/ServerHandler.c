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
	SOCKET server = socket(AF_INET, SOCK_STREAM, 0);
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
	app->pool = NULL;
	return app;
}

Status RunServer(APP *app, int pool_flag) {
	pthread_t th_server,th_client;
	ClientControlArgs args;
	args.app = app;
	args.pool_flag = pool_flag;
	pthread_create(&th_server, NULL, ServerControl, app);
	pthread_create(&th_client, NULL, ClientControl, &args);
	while (1);

}

void* ServerControl(APP* app) {
	while (1) {
		char temp[20];
		scanf_s("%s",temp,20);
		if (strcmp(temp, "stop") == 0 || strcmp(temp, "STOP") == 0)
		{
			closesocket(app->serverM->server);
			app->serverM->flag = 0;
		}
		else if (strcmp(temp, "show") == 0 || strcmp(temp, "SHOW") == 0) 
		{
			ShowThreadStatus(*app->pool);
		}
	}
}

void ThreadListen(ThreadArgs args) {
	ClientSolver* curTask;
	while (1) {
		if (args.app->serverM->flag == 0)
			break;
		else {
			curTask = GetTask(args.pool, 1);
			if (curTask == NULL)
				continue;
			//printf("\n%d", args.threadId);
			SolveClient(curTask, args.thisThread->_threadId);
			args.thisThread->_taskCount++;
		}
	}
}

void* ClientControl(ClientControlArgs clientArgs) {
	ThreadPool pool;
	//Thread *curThread;
	ThreadArgs args[MAX_THREAD_COUNT];
	
	if (clientArgs.pool_flag) {
		InitThreadPool(&pool);
		for (int i = 0; i < MAX_THREAD_COUNT; i++) {
			args[i].app = clientArgs.app;
			args[i].pool = &pool;
			args[i].thisThread = &pool.pool[i]._thread;
			pthread_create(&pool.pool[i]._thread, NULL, ThreadListen, &args[i]);
		}
		clientArgs.app->pool = &pool;
	}
	while (1) {
		//printf("等待连接中...");
		SOCKET client = accept(clientArgs.app->serverM->server, (sockaddr*)&clientArgs.app->client, &clientArgs.app->cllen);
		if (clientArgs.app->serverM->flag == 0) {
			closesocket(client);
			free(clientArgs.app->serverM);
			free(clientArgs.app);
			exit(0);
		}
		//参数一：表示谁接受连接
		//参数二：谁连接进来
		//参数三：用来保存信息的结构体大小
		//返回值为连接进来的 socket
		merror(client, INVALID_SOCKET, "连接失败\n");
		//printf("连接成功\n");
		ClientSolver* temp = (ClientSolver*)malloc(sizeof(ClientSolver));
		temp->client = client;
		temp->method = clientArgs.app->requestM;
		if (clientArgs.pool_flag) {
			/*curThread = GetThread(&pool);*/
			if (PutTask(&pool, temp, 1)){
				continue;
			}
			else {
				free(temp);
			}
		}
		else {
			pthread_t th;
			pthread_create(&th, NULL, SolveClient, temp);
		}
	}
}

void* SolveClient(ClientSolver* client,int threadId) {
	//是否处理成功
	int flag = 404;
	int socket_flag;
	char recvdata[50 * 1024] = "";
	//接收数据，默认为空
	int clientStatus = recv(client->client, recvdata, sizeof(recvdata), 0);
	if (clientStatus <= 0) {
		/*free(client);*/
		return;
}
	//参数一：接收消息的来源
	//参数二：接收消息的指针
	//参数三：接收消息的指针的内存大小
	//参数四：0表示默认的收发方式，一次性收完，等待流传输结束后一次收取

	//printf("共接受%d字节数据\n", strlen(recvdata));
	//printf("%s\n", recvdata);
	RequestText text;
	char postdata[50 * 1024] = "";
	
	strcpy_s(postdata, 50 * 1024, strstr(recvdata, "\r\n\r\n"));
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
		//printf("key: %s value:%s", text.params[text.param_len].name, text.params[text.param_len].data);
		text.param_len++;
		param_1 = NULL;
	} 
	text.headers_len = 0;
	text.headers = NULL;
	text.post_data.boundary = NULL;
	text.post_data.length = 0;
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
		if (strcmp(value, "") == 0) 
		{
			if(strcmp(text.head.method, "POST") == 0)
				text.post_data.boundary = header;
			break;
		}
		else if (strcmp(header, "Content-Length") == 0)
			text.post_data.length = atoi(value);
		text.headers_len++;
	} while (header != NULL);
	if(strcmp(postdata,"\r\n\r\n")==0)
	{

	}
	if (strcmp(text.head.method, "GET") == 0) {
		for (int i = 0; i < client->method.method_len; i++) {
			if (client->method.methods[i].if_static == 0) {
				if (client->method.methods[i].type == GET && strcmp(client->method.methods[i].uri, text.head.url) == 0)
					if (JudgeParam(client->method.methods[i], &text))
					{
						flag = GetMethodSolve(client, &text, i);
						break;
					}
			}
			else
			{
				char temp_url[100] = "";
				char rule[50];
				char temp_path[100] = "";
				char file_path[150] = "";

				strcpy_s(rule, 50, client->method.methods[i].uri);
				strcat_s(rule, 50, "/%s");
				strcpy_s(temp_url, 100, text.head.url);
				sscanf_s(temp_url, rule, temp_path, 100);
				if (strcmp(temp_path, "") != 0) {
					strcat_s(file_path, 150, client->method.methods[i].directory);
					strcat_s(file_path, 150, "/");
					strcat_s(file_path, 150, temp_path);
					flag = GetStaticSolve(client, &text, i, file_path);
				}
			}
		}
		time_t time_seconds = time(0);
		struct tm ptm;
		localtime_s(&ptm, &time_seconds);
		printf("\nTh-%d\t[%02d-%02d-%02d %02d:%02d:%02d]  GET  \"%s  %s\" - %d", threadId, ptm.tm_year + 1900, ptm.tm_mon + 1,
			ptm.tm_mday, ptm.tm_hour, ptm.tm_min, ptm.tm_sec, text.head.url, text.head.version, flag);
	}
	else if (strcmp(text.head.method, "POST") == 0) {
		for (int i = 0; i < client->method.method_len; i++) {
			if (client->method.methods[i].if_static == 0) {
				if (client->method.methods[i].type == POST && strcmp(client->method.methods[i].uri, text.head.url) == 0)
					if (JudgeParam(client->method.methods[i], &text))
					{
						flag = PostMethodSolve(client, &text, i, postdata);
						break;
					}
			}
		}
		time_t time_seconds = time(0);
		struct tm ptm;
		localtime_s(&ptm, &time_seconds);
		printf("\nTh-%d\t[%02d-%02d-%02d %02d:%02d:%02d]  POST \"%s  %s\" - %d", threadId, ptm.tm_year + 1900, ptm.tm_mon + 1,
			ptm.tm_mday, ptm.tm_hour, ptm.tm_min, ptm.tm_sec, text.head.url, text.head.version, flag);
		
	}
	char dont_allow[] = "HTTP/1.1 404 Not Found\r\n\r\nMethod Don't Allow";
	if (flag>=400)
	{
		send(client->client, dont_allow, strlen(dont_allow), 0);
	}
	socket_flag = closesocket(client->client);
	if (text.param_len > 0)
		free(text.params);
	if (text.headers_len > 0)
		free(text.headers);
	if (flag < 400 && text.post_data.length > 0 && strcmp(text.head.method, "POST") == 0)
		free(text.post_data.data);
	free(client);
	return;
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
	Response* res = NULL;
	if (client->method.methods[methodIndex].callback != NULL)
		res = (Response*)client->method.methods[methodIndex].callback(text->params);
	//printf("666");
	int sendTemp = SolveResponse(res, client);
	free(res->headers);
	free(res);
	return sendTemp;
}

int PostMethodSolve(ClientSolver* client, RequestText* text, int methodIndex, char* postData) {
	Response* res = NULL;
	char data[50 * 1024];
	//char main_data_temp[50 * 1024];
	char main_data[50 * 1024];
	if (client->method.methods[methodIndex].callback != NULL) {
		if (text->post_data.boundary != NULL && postData != NULL)
		{
			int start_pos = strlen(text->post_data.boundary) + 6;
			int end_pos = strlen(postData) - 2 * strlen(text->post_data.boundary) - 12;
			memcpy(data, postData + start_pos, end_pos);
			data[end_pos] = '\0';
			if (data == NULL)
				return 0;
			char* strstr_temp = strstr(data, "\r\n\r\n");
			if (strstr_temp == NULL)
				return 0;
			strcpy_s(main_data, 50 * 1024, strstr_temp);
			memcpy(main_data, &main_data[4], strlen(main_data));
			if (data != NULL)
				text->post_data.length = 0;
			if (text->param_len != 0) {
				PARAMS* tmp = NULL;
				tmp = (PARAMS*)realloc(text->params, sizeof(PARAMS) * (text->param_len + 1));
				if (tmp != NULL)
					text->params = tmp;
				else
				{
					printf("内存分配失败！");
					free(text->params);
					exit(-1);
				}
			}
			else
				text->params = (PARAMS*)malloc(sizeof(PARAMS));
			char* temp = NULL;
			if (data != NULL)
			{
				if (temp == NULL) 
				{
					char* temp_head = NULL;
					do {
						temp_head = strtok_s(data, "\r\n", &temp);
						char* key = NULL;
						char* key_temp = NULL;
						char* value = NULL;
						char* value_temp = NULL;
						key = strtok_s(temp_head, ":", &value);

						if (text->post_data.length == 0)
							//注意释放内存
							text->post_data.data = (Data*)malloc(sizeof(Data));
						else
						{
							Data* tmp = NULL;
							tmp = (Data*)realloc(text->post_data.data, sizeof(Data) * (text->post_data.length + 1));
							if (tmp != NULL)
								text->post_data.data = tmp;
							else
							{
								printf("内存分配失败！");
								free(text->post_data.data);
								exit(-1);
							}
						}
						strcpy_s(text->post_data.data[text->post_data.length].filename, 50, "");
						//text->post_data.data[text->post_data.length].type = NULL;
						while (strcmp("", value) != 0)
						{
							key_temp = strtok_s(NULL, "; ", &value);
							value_temp = strtok_s(NULL, "=", &key_temp);
							if (strcmp(key_temp, "") == 0)
								strcpy_s(text->post_data.data[text->post_data.length].type, 50, value_temp);
							else
							{
								if (strcmp("name", value_temp) == 0)
								{
									char tmp[20];
									sscanf_s(key_temp, "\"%[^\"]", tmp, 20);
									strcpy_s(text->post_data.data[text->post_data.length].name, 20, tmp);
								}
								else if (strcmp("filename", value_temp) == 0)
								{
									char tmp[50];
									sscanf_s(key_temp, "\"%[^\"]", tmp, 50);
									strcpy_s(text->post_data.data[text->post_data.length].filename, 50, tmp);
								}
							}
						}
						rsize_t size = 50 * 1024;
						char rule_str[200]="";
						sprintf_s(rule_str, 200, "%s%s", "%s", text->post_data.boundary);	
						sscanf_s(main_data, rule_str, text->post_data.data[text->post_data.length].data, strlen(text->post_data.boundary));
						char* str_str_temp = strstr(main_data, "\r\n\r\n");
						if (str_str_temp != NULL) {
							int start_pos = strlen(text->post_data.data[text->post_data.length].data) + strlen(text->post_data.boundary) + 4;
							memcpy(temp_head, &main_data[start_pos], strlen(main_data));
							strcpy_s(main_data, size, str_str_temp);
							memcpy(main_data, &main_data[4], strlen(main_data));
							text->post_data.length++;
						}
						else {
							text->post_data.length++;
							break;
						}
					} while (temp_head != NULL);
				}
			}
			
		}
		//for (int i = 0; i < text->post_data.length; i++) {
		//	printf("name: %s data: %s\n", text->post_data.data[i].name, text->post_data.data[i].data);
		//}
		res = (Response*)client->method.methods[methodIndex].callback(text->params, text->post_data);
	}
	int sendTemp = SolveResponse(res, client);
	free(res->headers);
	free(res);
	return sendTemp;
}

int GetStaticSolve(ClientSolver* client, RequestText* text, int methodIndex, char* path) {
	Response* res = (Response*)malloc(sizeof(Response));
	int file_type = JudgeType(path);
	if (file_type == 0)
		res->type = TEXT;
	else if (file_type == 1)
		res->type = JPG;
	else if (file_type == 2)
		res->type = PNG;
	else if (file_type == 3)
		res->type = HTML;
	else if (file_type == 4)
		res->type = JS;
	else if (file_type == 5)
		res->type = CSS;
	else if (file_type == 6)
		res->type = XML;
	res->code = SUCCESS;
	res->headers = (Header*)malloc(sizeof(Header));
	res->headers[0].key = "Server";
	res->headers[0].value = "DPweb";
	res->headers_len = 1;
	res->data.is_file = 1;
	res->data.data = path;
	int sendTemp = SolveResponse(res, client);
	free(res->headers);
	free(res);
	return sendTemp;
}

int SolveResponse(Response* orgin, ClientSolver* client) {
	if (orgin->data.is_file) {
		if (orgin->type < JPEG) {
			FILE* p_file;
			errno_t err = fopen_s(&p_file, orgin->data.data, "rb");
			int if_first = 1;
			int p_pos = 0;
			int size_num;
			if (err != 0)
			{
				//printf("文件打开失败");
				char temp[] = "HTTP/1.1 404 Not Found\r\n\r\nFile Don't Find";
				send(client->client, temp, sizeof(temp), 0);
				return NOTFOUND;
			}
			char head[1024] = "";
			char temp_code[50] = "";
			strcpy_s(temp_code, 50, GetStatusCode(orgin->code));

			strcat_s(head, 1024, temp_code);
			if(orgin->type==HTML)
				strcat_s(head, 1024, "Content-Type: text/html; charset=utf-8\r\n");
			else if(orgin->type == JS)
				strcat_s(head, 1024, "Content-Type: application/javascript; charset=utf-8\r\n");
			else if (orgin->type == CSS)
				strcat_s(head, 1024, "Content-Type: text/css; charset=utf-8\r\n");
			else if (orgin->type == XML)
				strcat_s(head, 1024, "Content-Type: text/xml; charset=utf-8\r\n");
			strcat_s(head, 1024, "Content-Length: ");
			if (fseek(p_file, 0, SEEK_END) == 0) 
			{
				size_num = ftell(p_file);
				char size_str[10];
				//_itoa_s(size_num, size_str, 10, 10);
				sprintf_s(size_str, 10, "%d", size_num);
				strcat_s(head, 1024, size_str);
				strcat_s(head, 1024, "\r\n");
				rewind(p_file);
			}
			for (int i = 0; i < orgin->headers_len; i++)
			{
				char temp[50] = "";
				strcpy_s(temp, 50, GetHeader(orgin->headers[i].key, orgin->headers[i].value));
				strcat_s(head, 1024, temp);
			}
			strcat_s(head, 1024, "\r\n");
			/*printf("%s", head);*/
			send(client->client, head, strlen(head), 0);
			char* tempdata = (char*)malloc(sizeof(char) * 1024 * 10);
			int temp;
			size_t bytes_read;
			do
			{
				bytes_read = fread(tempdata, sizeof(char), 1024 * 10, p_file);
				//fgets(tempdata, 1024, p_file);
				if (bytes_read > 0)
					temp = send(client->client, tempdata, bytes_read, 0);
			} while (!feof(p_file));
			fclose(p_file);
			free(tempdata);
		}
		// 图片文件
		else if (orgin->type >= JPEG && orgin->type < 80)
		{
			FILE* p_file;
			errno_t err = fopen_s(&p_file, orgin->data.data, "rb");
			int p_pos = 0;
			int size_num;
			if (err != 0)
			{
				//printf("文件打开失败");
				char temp[] = "HTTP/1.1 404 Not Found\r\n\r\nFile Don't Find";
				send(client->client, temp, sizeof(temp), 0);
				return NOTFOUND;
			}
			char head[1024] = "";
			char temp_code[50] = "";
			strcpy_s(temp_code, 50, GetStatusCode(orgin->code));
			strcat_s(temp_code, 50, "Content-Length: ");
			strcat_s(head, 1024, temp_code);
			if (fseek(p_file, 0, SEEK_END) == 0)
			{
				size_num = ftell(p_file);
				char size_str[10];
				//_itoa_s(size_num, size_str, 10, 10);
				sprintf_s(size_str, 10, "%d", size_num);
				strcat_s(head, 1024, size_str);
				strcat_s(head, 1024, "\r\n");
				if (orgin->type == JPEG)
					strcat_s(head, 1024, "Content-Type: image/jpeg\r\n");
				else
					strcat_s(head, 1024, "Content-Type: image/png\r\n");
				for (int i = 0; i < orgin->headers_len; i++)
				{
					char temp[50] = "";
					strcpy_s(temp, 50, GetHeader(orgin->headers[i].key, orgin->headers[i].value));
					strcat_s(head, 1024, temp);
				}
				strcat_s(head, 1024, "\r\n");
				rewind(p_file);
				send(client->client, head, strlen(head), 0);
			}
			char* tempdata = (char*)malloc(sizeof(char) * 1024);
			do {
				fgets(tempdata, 1024, p_file);
				p_pos += strlen(tempdata);
				fseek(p_file, p_pos, SEEK_SET);
				int f_pos = ftell(p_file);
				if (strlen(tempdata) == 0)
				{
					send(client->client, "\x00", 1, 0);
					p_pos += 1;
					fseek(p_file, p_pos, SEEK_SET);
				}
				else
					send(client->client, tempdata, strlen(tempdata), 0);
				if (size_num == p_pos)
					break;
			} while (!feof(p_file));
			fclose(p_file);
			free(tempdata);
		}
	}
	else
	{
		char head[1024] = "";
		char temp_code[50] = "";
		strcpy_s(temp_code, 50, GetStatusCode(orgin->code));
		strcat_s(head, 1024, temp_code);
		strcat_s(head, 1024, "Content-Type: text/*;charset=utf-8\r\n");
		strcat_s(head, 1024, "Content-Length: ");
		int size_num = strlen(orgin->data.data);
		char size_str[10];
		//_itoa_s(size_num, size_str, 10, 10);
		sprintf_s(size_str, 10, "%d", size_num);
		strcat_s(head, 1024, size_str);
		strcat_s(head, 1024, "\r\n");
		for (int i = 0; i < orgin->headers_len; i++)
		{
			char temp[50] = "";
			strcpy_s(temp, 50, GetHeader(orgin->headers[i].key, orgin->headers[i].value));
			strcat_s(head, 1024, temp);
		}
		strcat_s(head, 1024, "\r\n");
		send(client->client, head, strlen(head), 0);
		send(client->client, orgin->data.data, size_num, 0);
	}
	return orgin->code;
}

