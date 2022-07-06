#include "Request.h"
int addMethod(APP* app, METHODTYPE type, URI uri, CALLBACKFUNC callback, PARAMS* params, int param_len) {
	app->requestM.methods[app->requestM.method_len].type = type;

	strcpy_s(app->requestM.methods[app->requestM.method_len].uri, 20, uri);
	app->requestM.methods[app->requestM.method_len].callback = callback;
	app->requestM.methods[app->requestM.method_len].params = params;
	app->requestM.methods[app->requestM.method_len].param_len = param_len;
	app->requestM.method_len++;
	printf("装载方法 %s 成功\n", uri);
}