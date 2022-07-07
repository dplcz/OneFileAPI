#include "Request.h"
int addMethod(APP* app, METHODTYPE type, URI uri, CALLBACKFUNC callback, PARAMS *params, int param_len) {
	app->requestM.methods[app->requestM.method_len].type = type;

	app->requestM.methods[app->requestM.method_len].uri = uri;
	app->requestM.methods[app->requestM.method_len].callback = callback;
	app->requestM.methods[app->requestM.method_len].params = params;
	app->requestM.methods[app->requestM.method_len].param_len = param_len;
	app->requestM.method_len++;
	printf("装载方法 %s 成功\n", uri);
}

void addHeaders(Header *originHeader, char* key, char* value) {
	int len_header = sizeof(originHeader) / sizeof(Header);
	if (len_header == 0)
		originHeader = (Header*)malloc(sizeof(Header*));
	else
	{
		int 
	}
}		

Response response(RESPONSETYPE type, Header headers[], char* filename) {
	return;
}