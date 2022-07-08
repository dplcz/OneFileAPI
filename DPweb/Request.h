#include "Model.h"
#include <string.h>




Status addMethod(APP* app, METHODTYPE type, URI uri, CALLBACKFUNC callback, PARAMS *params, int param_len);

Status addStaticFile(APP* app, URI uri, char* directory);

Response* createResponse(RESPONSETYPE type, char* data, STATUSCODE code);

Status addHeaders(Response* origin, char* key, char* value);
