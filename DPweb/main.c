#include"Model.h"
#include"Server.h"
#include"Tools.h"
#include"Request.h"

void* test(void* args) {
	
}

void main() {
	APP* app = CreateServerIPV4(INADDR_ANY, 80);
	addMethod(app, GET, "/index", NULL, NULL, 0);
	Status status = RunServer(app);
}