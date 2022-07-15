#include"Model.h"
#include"Server.h"
#include"Tools.h"
#include"Request.h"

void* test(PARAMS *params) {
	Response* response = createResponse(HTML, "base.html", SUCCESS);
	addHeaders(response, "dp", "123");
	addHeaders(response, "lcz", "666");
	return response;
}

void main() {
	APP* app = CreateServerIPV4(INADDR_ANY, 80);
	addMethod(app, GET, "/index", test, NULL, 0);
	addStaticFile(app, "/base", "E:/c_code/DPweb/static");
	Status status = RunServer(app);
}