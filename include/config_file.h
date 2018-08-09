#ifndef __CONFIG_FILE_H__
#define __CONFIG_FILE_H__

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#define XML_STATIC

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "mxml/mxml.h"

typedef struct S_CF_PAIR {
	char *key;
	char *value;
	struct S_CF_PAIR *next;
} cf_pair_t;

int cf_load(const char *fname);
void cf_destroy(void);

void _cf_add_pair(const char *key, const char *value);

bool cf_get_value(const char *key, char *value);

void cf_print(void);

#endif