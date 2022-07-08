#include "Model.h"
#include <string.h>




Status addMethod(APP* app, METHODTYPE type, URI uri, CALLBACKFUNC callback, PARAMS *params, int param_len);

Response* createResponse(RESPONSETYPE type, char* data);

Status addHeaders(Response* origin, char* key, char* value);
