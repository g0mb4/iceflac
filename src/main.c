#include "ice_protocol.h"
#include "flac_handler.h"
#include "config_file.h"
#include "playlist.h"

int main(int argc, char **argv) {
	ice_clinet_t* ice = NULL;
	flac_handler_t* flac = NULL;
	playlist_t *pl = NULL;

	char config_file[128] = "iceflac.xml";
	char play_list[128] = "playlist.m3u";
	bool cmd_playlist = false;

	char config_val[64];
	int err = 0, a;
	bool verbose = true, silent = false;

	for (a = 0; a < argc; a++) {
		if (!strcmp(argv[a], "-c") || !strcmp(argv[a], "--config")) {
			strcpy(config_file, argv[++a]);
		} else if (!strcmp(argv[a], "-p") || !strcmp(argv[a], "--playlist")) {
			strcpy(play_list, argv[++a]);
			cmd_playlist = true;
		} else if (!strcmp(argv[a], "-v") || !strcmp(argv[a], "--version")) {
			printf("%s\n", USER_AGENT);
			return 0;
		} else if (!strcmp(argv[a], "-h") || !strcmp(argv[a], "--help")) {
			printf("%s\n\n"
				   "usage: iceflac <options>\n\n"
				   "options:\n"
				   " -c <file>, --config <file>    : specify custom configuration file (default: iceflac.xml)\n"
				   " -p <file>, --playlist <file>  : specify playlist file,\n "
				   "                                 this will be used regardless of the configuration file\n"
				   " -v, --version                 : version information\n"
				   " -h, --help                    : this screen\n"
				, USER_AGENT);
			return 0;
		}
	}

	if (cf_load(config_file) < 0) {
		fprintf(stderr, "cannot load configuration file '%s'\n", config_file);
		return 1;
	}

	if (cf_get_value("verbose", config_val)) {
		verbose = config_val[0] != '0';
	}

	if (cf_get_value("silent", config_val)) {
		silent = config_val[0] != '0';
	}

	if (!silent) {
		printf("%s\n\n", USER_AGENT);
	}

	// cf_print();

	if (!cmd_playlist) {
		if (!cf_get_value("playlist", play_list)) {
			fprintf(stderr, "cannot find property 'playlist'\n");
			return 2;
		}
	}

	pl = pl_init(play_list);
	if (!pl) {
		fprintf(stderr, "cannot load playlist\n");
		return 3;
	}

	if (cf_get_value("loop", config_val)) {
		pl_set_loop(pl, config_val[0] != '0');
	}

	if (cf_get_value("random", config_val)) {
		pl_set_random(pl, config_val[0] != '0');
	}

	pl_set_verbose(pl, verbose);
	pl_set_silent(pl, silent);

	pl_print(pl);

	ice = ice_init();
	bool first_track = true;

	if (ice) {

		if (cf_get_value("server", config_val)) {
			ice_set_server(ice, config_val);
		} else {
			fprintf(stderr, "cannot find property 'server'\n");
		}

		if (cf_get_value("port", config_val)) {
			ice_set_port(ice, config_val);
		} else {
			fprintf(stderr, "cannot find property 'port'\n");
		}

		if (cf_get_value("user", config_val)) {
			ice_set_user(ice, config_val);
		} else {
			fprintf(stderr, "cannot find property 'user'\n");
		}

		if (cf_get_value("password", config_val)) {
			ice_set_pass(ice, config_val);
		} else {
			fprintf(stderr, "cannot find property 'password'\n");
		}

		if (cf_get_value("mount", config_val)) {
			ice_set_mount(ice, config_val);
		} else {
			fprintf(stderr, "cannot find property 'mount'\n");
		}

		if (cf_get_value("public", config_val)) {
			ice_set_ice_public(ice, config_val[0] != '0');
		}

		if (cf_get_value("name", config_val)) {
			ice_set_ice_name(ice, config_val);
		}

		ice_set_verbose(ice, verbose);
		ice_set_silent(ice, silent);

		flac = fh_init(ice);
		if (flac) {
			err = ice_connect(ice);
			if (err == 0) {
				char file_name[1024];
				while(pl_get_next(pl, file_name)) {
					err = fh_decode_file(flac, file_name);
					if (err == 0) {
						if (first_track) {
							err = ice_auth(ice, AUTH_PUT);
							if (err == 0) {
								if (!ice->silent) {
									printf("connected to: %s:%s/%s as %s\n", ice->server, ice->port, ice->mount, ice->user);
								}
							}
							else {
								fprintf(stderr, "authentication with PUT method failed with error: %d\n", err);
								err = ice_auth(ice, AUTH_SOURCE);
								if (err == 0) {
									if (!ice->silent) {
										printf("connected to: %s:%s/%s as %s\n", ice->server, ice->port, ice->mount, ice->user);
									}
								} else {
									fprintf(stderr, "authentication with SOURCE method failed with error: %d\n", err);
								}
							}

							first_track = false;
						}
						if (err == 0) {
							if (!ice->silent) {
								printf("\nplaying: %s - %s (%s)\n", ice->artist, ice->title, file_name);
							}
							err = fh_encode_stream(flac);
							if (err == 0) {
								fprintf(stderr, "streaming done\n");
							}
							else {
								fprintf(stderr, "cannot encode stream: %d\n", err);
							}
						}
						else {
							fprintf(stderr, "authentication failed\n");
						}
					}
					else {
						fprintf(stderr, "cannot decode file: %d\n", err);
					}
				}
			}
			else {
				fprintf(stderr, "cannot connect to the server: %d\n", err);
			}
		}
		else {
			fprintf(stderr, "flac initialization failed: %d\n", err);
		}
	}
	else {
		fprintf(stderr, "initialization failed %d\n", err);
	}

	printf("done.\n");
	cf_destroy();
	fh_destroy(flac);
	ice_destroy(ice);
	pl_destroy(pl);
	return 0;
}
