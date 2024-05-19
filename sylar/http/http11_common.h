/*
 * @Author       : wenwneyuyu
 * @Date         : 2024-05-14 15:34:31
 * @LastEditors  : wenwenyuyu
 * @LastEditTime : 2024-05-14 15:34:34
 * @FilePath     : /sylar/http/http11_common.h
 * @Description  : 
 * Copyright 2024 OBKoro1, All Rights Reserved. 
 * 2024-05-14 15:34:31
 */
#ifndef _http11_common_h
#define _http11_common_h

#include <sys/types.h>

typedef void (*element_cb)(void *data, const char *at, size_t length);
typedef void (*field_cb)(void *data, const char *field, size_t flen, const char *value, size_t vlen);

#endif