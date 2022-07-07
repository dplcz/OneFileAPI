#include "Model.h"
#include <string.h>




int addMethod(APP* app, METHODTYPE type, URI uri, CALLBACKFUNC callback, PARAMS *params, int param_len);

void addHeaders(Header originHeader, char* key, char* value);

Response response(RESPONSETYPE type, Header headers[], char* filename);