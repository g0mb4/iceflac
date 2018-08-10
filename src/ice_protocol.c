#include "ice_protocol.h"

ice_clinet_t *ice_init(void) {
	ice_clinet_t* ice = (ice_clinet_t*)malloc(sizeof(ice_clinet_t));
	memset(ice, 0, sizeof(ice_clinet_t));

	WSADATA wsaData;
	int res;

	res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (res != 0) {
		fprintf(stderr, "ice: WSAStartup() failed with error: %d\n", res);
		return NULL;
	}

	ice->socket = INVALID_SOCKET;
	ice->server = NULL;
	ice->port = NULL;
	ice->user = NULL;
	ice->pass = NULL;
	ice->mount = NULL;

	ice->ice_public = 0;
	ice->ice_name = NULL;
	ice->ice_bitrate_raw = 0;
	ice->ice_samplerate = 0;

	return ice;
}

void ice_destroy(ice_clinet_t * ice) {
	if (ice->socket != INVALID_SOCKET) {
		int res = shutdown(ice->socket, SD_SEND);
		if (res == SOCKET_ERROR) {
			fprintf(stderr, "ice: shutdown() failed with error: %d\n", WSAGetLastError());
		}

		closesocket(ice->socket);
		WSACleanup();
	}

	FREE_POINTER( ice->server );
	FREE_POINTER( ice->port );
	FREE_POINTER( ice->user );
	FREE_POINTER( ice->pass );
	FREE_POINTER( ice->mount );
	FREE_POINTER( ice->ice_name );

	FREE_POINTER( ice->artist );
	FREE_POINTER( ice->title );
}

void ice_set_server(ice_clinet_t * ice, char *server) {
	FREE_POINTER( ice->server );

	ice->server = (char*)malloc(strlen(server) + 1);
	strcpy(ice->server, server);
}

void ice_set_port(ice_clinet_t * ice, char *port) {
	FREE_POINTER( ice->port );

	ice->port = (char*)malloc(strlen(port) + 1);
	strcpy(ice->port, port);
}

void ice_set_user(ice_clinet_t * ice, char *user) {
	FREE_POINTER( ice->user );

	ice->user = (char*)malloc(strlen(user) + 1);
	strcpy(ice->user, user);
}

void ice_set_pass(ice_clinet_t * ice, char *pass) {
	FREE_POINTER( ice->pass );

	ice->pass = (char*)malloc(strlen(pass) + 1);
	strcpy(ice->pass, pass);
}

void ice_set_mount(ice_clinet_t * ice, char *mount) {
	FREE_POINTER( ice->mount );

	ice->mount = (char*)malloc(strlen(mount) + 1);
	strcpy(ice->mount, mount);
}

void ice_set_ice_public(ice_clinet_t * ice, bool p) {
	ice->ice_public = p;
}

void ice_set_ice_name(ice_clinet_t * ice, char *name) {
	FREE_POINTER( ice->ice_name );

	ice->ice_name = (char*)malloc(strlen(name) + 1);
	strcpy(ice->ice_name, name);
}

void ice_set_artist(ice_clinet_t * ice, char *artist) {
	FREE_POINTER( ice->artist );

	ice->artist = (char*)malloc(strlen(artist) + 1);
	strcpy(ice->artist, artist);
}

void ice_set_title(ice_clinet_t * ice, char *title) {
	FREE_POINTER( ice->title );

	ice->title = (char*)malloc(strlen(title) + 1);
	strcpy(ice->title, title);
}

int ice_connect(ice_clinet_t * ice){
	if (ice->server == NULL || ice->port == NULL) {
		fprintf(stderr, "ice: connection paramteres are not set\n");
		return -1;
	}

	struct addrinfo *result = NULL, *ptr = NULL, hints;
	int res;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	res = getaddrinfo(ice->server, ice->port, &hints, &result);
	if (res != 0) {
		fprintf(stderr, "ice: getaddrinfo() failed with error: %d\n", res);
		WSACleanup();
		return -2;
	}

	/* Attempt to connect to an address until one succeeds */
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		ice->socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ice->socket == INVALID_SOCKET) {
			fprintf(stderr, "ice: socket() failed with error: %d\n", WSAGetLastError());
			WSACleanup();
			return -3;
		}

		res = connect(ice->socket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (res == SOCKET_ERROR) {
			closesocket(ice->socket);
			ice->socket = INVALID_SOCKET;
			continue;
		}
		break;
	}
	freeaddrinfo(result);

	if (ice->socket == INVALID_SOCKET) {
		fprintf(stderr, "ice: connect() failed\n");
		WSACleanup();
		return -4;
	}

	return 0;
}

int ice_auth(ice_clinet_t * ice, uint8_t method) {
	if (ice->user == NULL || ice->pass == NULL || ice->mount == NULL || ice->ice_name == NULL) {
		fprintf(stderr, "ice: authentication paramteres are not set\n");
		return -1;
	}

	int res;
	char user_pass_64[256];
	char auth_message[1024];

	char recvbuf[512];
	int recvbuflen = 512;

	bool auth_ok = false;

	int len = strlen(ice->user) + 1 + strlen(ice->pass) + 1;
	char *user_pass = (char *)malloc(len);
	sprintf_s(user_pass, len, "%s:%s", ice->user, ice->pass);

	if (!base64_encode(user_pass_64, user_pass)) {
		fprintf(stderr, "ice: base64_encode() failed\n");
		return -1;
	}

	if (method == AUTH_SOURCE) {
		sprintf_s(auth_message, 1024,
			"SOURCE /%s HTTP/1.0\r\n"
			"Authorization: Basic %s\r\n"
			"User-Agent: %s\r\n"
			"Content-Type: application/ogg\r\n"
			"Ice-Public: %d\r\n"
			"Ice-Name: %s\r\n"
			"Ice-bitrate: %u\r\n"
			"Ice-audio-info: samplerate=%u;channels=%d\r\n"
			"\r\n",
			ice->mount, user_pass_64, USER_AGENT, ice->ice_public, ice->ice_name, ice->ice_bitrate_raw, ice->ice_samplerate, ice->channels);

	}
	else if (method == AUTH_PUT) {
		sprintf_s(auth_message, 1024,
			"PUT /%s HTTP/1.0\r\n"
			"Authorization: Basic %s\r\n"
			"User-Agent: %s\r\n"
			"Content-Type: application/ogg\r\n"
			"Ice-Public: %d\r\n"
			"Ice-Name: %s\r\n"
			"Ice-bitrate: %u\r\n"
			"Ice-audio-info: samplerate=%u;channels=%d\r\n"
			"Expect: 100-continue\r\n"
			"\r\n",
			ice->mount, user_pass_64, USER_AGENT, ice->ice_public, ice->ice_name, ice->ice_bitrate_raw, ice->ice_samplerate, ice->channels);
	}
	else {
		fprintf(stderr, "ice: unknown authentication method\n");
		return -2;
	}

	res = send(ice->socket, auth_message, (int)strlen(auth_message), 0);
	if (res == SOCKET_ERROR) {
		printf("ice: send() failed with error: %d\n", WSAGetLastError());
		closesocket(ice->socket);
		WSACleanup();
		return -3;
	}

	// Receive until the peer closes the connection
	do {
		res = recv(ice->socket, recvbuf, recvbuflen, 0);
		if (res > 0) {
			//printf("%s", recvbuf);
			if (method == AUTH_SOURCE) {
				if (strncmp(recvbuf, "HTTP/1.0 200 OK\r\n", strlen("HTTP/1.0 200 OK\r\n")) == 0) {
					auth_ok = true;
					break;
				}
			}
			else if (method == AUTH_PUT) {
				if (strncmp(recvbuf, "HTTP/1.1 100 Continue\r\n", strlen("HTTP/1.1 100 Continue\r\n")) == 0) {
					auth_ok = true;
					break;
				}
			}
			memset(recvbuf, 0, recvbuflen);
		}
		else if (res == 0) {
			fprintf(stderr, "ice: connection closed\n");
		}
		else {
			fprintf(stderr, "ice: recv() failed with error: %d\n", WSAGetLastError());
		}

	} while (res > 0);

	return (auth_ok ? 0 : -4);
}

/* source: https://github.com/littlstar/b64.c
 */
bool base64_encode(char *b64, char *text) {
	/* source:hackme -> c291cmNlOmhhY2ttZQ== */
	int i = 0;
	int j = 0;
	char *enc = NULL;
	size_t size = 0;
	unsigned char buf[4];
	unsigned char tmp[3];

	// alloc
	enc = (char *)malloc(sizeof(char));
	if (!enc) { 
		return false; 
	}

	// parse until end of source
	while (*text) {
		// read up to 3 bytes at a time into `tmp'
		tmp[i++] = *text++;

		// if 3 bytes read then encode into `buf'
		if (3 == i) {
			buf[0] = (tmp[0] & 0xfc) >> 2;
			buf[1] = ((tmp[0] & 0x03) << 4) + ((tmp[1] & 0xf0) >> 4);
			buf[2] = ((tmp[1] & 0x0f) << 2) + ((tmp[2] & 0xc0) >> 6);
			buf[3] = tmp[2] & 0x3f;

			// allocate 4 new byts for `enc` and
			// then translate each encoded buffer
			// part by index from the base 64 index table
			// into `enc' unsigned char array
			enc = (char *)realloc(enc, size + 4);
			for (i = 0; i < 4; ++i) {
				enc[size++] = b64_table[buf[i]];
			}

			// reset index
			i = 0;
		}
	}

	// remainder
	if (i > 0) {
		// fill `tmp' with `\0' at most 3 times
		for (j = i; j < 3; ++j) {
			tmp[j] = '\0';
		}

		// perform same codec as above
		buf[0] = (tmp[0] & 0xfc) >> 2;
		buf[1] = ((tmp[0] & 0x03) << 4) + ((tmp[1] & 0xf0) >> 4);
		buf[2] = ((tmp[1] & 0x0f) << 2) + ((tmp[2] & 0xc0) >> 6);
		buf[3] = tmp[2] & 0x3f;

		// perform same write to `enc` with new allocation
		for (j = 0; (j < i + 1); ++j) {
			enc = (char *)realloc(enc, size + 1);
			enc[size++] = b64_table[buf[j]];
		}

		// while there is still a remainder append '=' 
		while ((i++ < 3)) {
			enc = (char *)realloc(enc, size + 1);
			enc[size++] = '=';
		}
	}

	enc = (char *)realloc(enc, size + 1);
	enc[size] = '\0';

	strcpy(b64, enc);

	return true;
}

int ice_send_data(ice_clinet_t * ice, uint8_t *data, size_t len) {
	int res;
	char recvbuf[512];
	int recvbuflen = 512;

	res = send(ice->socket, data, (int)len, 0);
	if (res == SOCKET_ERROR) {
		fprintf(stderr, "ice: send() failed with error: %d\n", WSAGetLastError());
		return -1;
	}

	return 0;
}


