#pragma once
#include<stdio.h>
#include<stdlib.h>

#include "Model.h"

//报错机制
int merror(int redata, int error, char* showTips);
//获取状态码字符串
char* GetStatusCode(STATUSCODE code);
//拼接响应头的键值
char* GetHeader(char* key, char* value);
//判断文件类型
int JudgeType(char* path);
