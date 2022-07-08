#include "Request.h"
Status addMethod(APP* app, METHODTYPE type, URI uri, CALLBACKFUNC callback, PARAMS *params, int param_len) {
	app->requestM.methods[app->requestM.method_len].type = type;
	app->requestM.methods[app->requestM.method_len].uri = uri;
	app->requestM.methods[app->requestM.method_len].callback = callback;
	app->requestM.methods[app->requestM.method_len].params = params;
	app->requestM.methods[app->requestM.method_len].param_len = param_len;
	app->requestM.method_len++;
	printf("装载方法 %s 成功\n", uri);
}

Response* createResponse(RESPONSETYPE type, char* data,STATUSCODE code) {
	//注意释放内存
	Response* response = (Response*)malloc(sizeof(Response));
	//response.headers = (Header*)malloc(sizeof(Header));
	response->type = type;
	response->headers_len = 0;
	response->data.if_file = 1;
	response->data.data = data;
	response->code = code;
	addHeaders(response, "Server", "DPweb");
	switch (type)
	{
	case TEXT:
		addHeaders(response, "Content-Type", "text/plain");
		break;
	case HTML:
		addHeaders(response, "Content-Type", "text/html");
		break;
	case XML:
		addHeaders(response, "Content-Type", "text/xml");
		break;
	case JS:
		addHeaders(response, "Content-Type", "text/javascript");
		break;
	case JSON:
		addHeaders(response, "Content-Type", "application/Json");
		break;
	case JPG:
		addHeaders(response, "Content-Type", "image/jpeg");
		break;
	case PNG:
		addHeaders(response, "Content-Type", "image/png");
		break;
	default:
		printf("还未适配该格式，将使用默认text格式");
		addHeaders(response, "Content-Type", "text/plain");
		break;
	}
	return response;
}

Status addHeaders(Response *origin, char* key, char* value) {
	int len_header = origin->headers_len;
	if (len_header == 0) 
	{
		origin->headers = (Header*)malloc(sizeof(Header));
		origin->headers[0].key = key;
		origin->headers[0].value = value;
		origin->headers_len++;
		return OK;
	}
	else
	{
		Header* temp = NULL;
		temp = (Header*)realloc(origin->headers, sizeof(Header) * (len_header + 1));
		if (temp != NULL) {
			origin->headers = temp;
			origin->headers[len_header].key = key;
			origin->headers[len_header].value = value;
			origin->headers_len++;
			return OK;
		}
		else
		{
			free(origin->headers);
			printf("添加头失败");
			return ERROR;
		}
	}
}		