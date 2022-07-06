#include "Tools.h"

int merror(int redata, int error, char* showTips) {
	if (redata == error)
	{
		perror(showTips);
		printf("\n");
		getchar();
		exit(-1);
	}
	return 1;
}