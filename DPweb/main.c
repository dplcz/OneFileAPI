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

void* postSolve(PARAMS* params, PostData* data) {
	Response* response = createResponse(TEXT, "dplcz666", SUCCESS);
	FILE* p_file;
	errno_t err = fopen_s(&p_file, data->data[0].filename, "w+");
	if (err != 0)
	{
		perror("Error:");
		return response;
	}
	fwrite(data->data[0].data, strlen(data->data[0].data), 1, p_file);
	fclose(p_file);
	printf("\n%s上传成功", data->data[0].filename);
	return response;
}

void main() {
	APP* app = CreateServerIPV4(INADDR_ANY, 8000);
	addMethod(app, GET, "/index", test, NULL, 0);
	addMethod(app, POST, "/upload", postSolve, NULL, 0);
	addStaticFile(app, "/base", "E:/c_code/DPweb/static");
	Status status = RunServer(app);
}