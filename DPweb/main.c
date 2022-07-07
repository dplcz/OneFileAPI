#include"Model.h"
#include"Server.h"
#include"Tools.h"
#include"Request.h"

void* test(PARAMS *params) {
	printf("123456");
	int num = 123;
	int* a = &num;
	return ;
}

void main() {
	APP* app = CreateServerIPV4(INADDR_ANY, 80);
	addMethod(app, GET, &("/index"), test, NULL, 0);
	Status status = RunServer(app);
}