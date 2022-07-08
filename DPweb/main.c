#include"Model.h"
#include"Server.h"
#include"Tools.h"
#include"Request.h"

void* test(PARAMS *params) {
	printf("123456");
	Response* response = createResponse(HTML, "base.html");
	addHeaders(response, "dp", "123");
	addHeaders(response, "lcz", "666");
	return response;
}

void main() {
	APP* app = CreateServerIPV4(INADDR_ANY, 80);
	addMethod(app, GET, &("/index"), test, NULL, 0);


	Status status = RunServer(app);
}