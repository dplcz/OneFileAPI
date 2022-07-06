#include"Model.h"
#include"Server.h"
#include"Tools.h"
void main() {
	APP* app = CreateServerIPV4(INADDR_ANY, 80);
	Status status = RunServer(app);
}