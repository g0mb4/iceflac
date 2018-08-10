#ifndef __ICE_PROTOCOL_H__
#define __ICE_PROTOCOL_H__

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define USER_AGENT		"iceflac/1.0"

#define FREE_POINTER( x )	{ if( x ) { free( x ); } }

static const char b64_table[] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3',
	'4', '5', '6', '7', '8', '9', '+', '/'
};

enum {
	AUTH_SOURCE = 0,
	AUTH_PUT
};

typedef struct S_ICE_CLIENT {
	SOCKET socket;

	char *server;
	char *port;
	char *user;
	char *pass;
	char *mount;

	bool ice_public;
	char *ice_name;
	char *artist;
	char *title;

	uint32_t ice_bitrate_raw;
	uint32_t ice_samplerate;
	uint8_t channels;
}ice_clinet_t;

ice_clinet_t *ice_init(void);
void ice_destroy(ice_clinet_t * ice);

void ice_set_server(ice_clinet_t * ice, char *server);
void ice_set_port(ice_clinet_t * ice, char *port);
void ice_set_user(ice_clinet_t * ice, char *user);
void ice_set_pass(ice_clinet_t * ice, char *pass);
void ice_set_mount(ice_clinet_t * ice, char *mount);
void ice_set_ice_public(ice_clinet_t * ice, bool p);
void ice_set_ice_name(ice_clinet_t * ice, char *name);
void ice_set_artist(ice_clinet_t * ice, char *artist);
void ice_set_title(ice_clinet_t * ice, char *title);

int ice_connect(ice_clinet_t * ice);

int ice_auth(ice_clinet_t * ice, uint8_t method);

bool base64_encode(char *b64, char *text);

int ice_send_data(ice_clinet_t * ice, uint8_t *data, size_t len);

#endif
