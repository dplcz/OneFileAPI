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

void* test2(PARAMS* params) {
	Response* response = createResponse(TEXT, "123456789", SUCCESS);
	Sleep(100);
	addHeaders(response, "dp", "123");
	addHeaders(response, "lcz", "666");
	return response;
}

void* postSolve(PARAMS* params, PostData* data) {
	for (int i = 0; i < data->length; i++)
		if (strcmp(data->data[i].filename, "") != 0) {

			Response* response = createResponse(TEXT, "dplcz666", SUCCESS);
			FILE* p_file;
			errno_t err = fopen_s(&p_file, data->data[i].filename, "w+");
			if (err != 0)
			{
				perror("Error:");
				return response;
			}
			fwrite(data->data[i].data, strlen(data->data[i].data), 1, p_file);
			fclose(p_file);
			printf("\n%s上传成功", data->data[i].filename);
			return response;
		}
	return createResponse(TEXT, "FAILED", BADREQUEST);
}

void main() {
	APP* app = CreateServerIPV4(INADDR_ANY, 8000);
	addMethod(app, GET, "/index", test, NULL, 0);
	addMethod(app, GET, "/test", test2, NULL, 0);
	addMethod(app, POST, "/upload", postSolve, NULL, 0);
	addStaticFile(app, "/base", "D:/vs_project/layuimini-v2");
	Status status = RunServer(app, 1);
}