#include <getopt.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "application_config.h"
#include "axi/axi.h"
#include "debug.h"
#include "video_transmit.h"
#include "www/server.h"

struct application_config config;

void interruptHandler(int sig) {
	fdebug("Signal %d received.", sig);
	config.run = false;
}

void* server_spawn(void *ctx) {
	server_listen((struct server_ctx *) ctx);
	return NULL;
}

static void help() {
	debug("Options:");
	debug("\t--server-disable   -n");
	debug("\t--server-port      -p PORT");
	debug("\t--server-directory -d DIRECTORY_TO_SERVE");
	debug("\t--image-width      -x IMAGE_WIDTH_PIXELS");
	debug("\t--image-height     -y IMAGE_HEIGHT_PIXELS");
	debug("\t--algo-alpha       -a ALPHA_PARAMETER");
	debug("\t--algo-fd          -f FRAME_DIFFERENCE_THRESHOLD");
	debug("\t--algo-bg          -b BACKGROUND_THRESHOLD");
	debug("\t--axi-debug        -v");
	debug("\t--mode             -m setup|algo");
	debug("\t--help             -h");
}

void parse_arguments(int argc, char * argv[]) {
	static struct option long_options[] = {
	  {"server-disable",   no_argument,       NULL, 'n'},
	  {"server-port",      required_argument, NULL, 'p'},
	  {"server-directory", required_argument, NULL, 'd'},
	  {"help",      	   no_argument,       NULL, 'h'},
	  {"image-width", 	   required_argument, NULL, 'x'},
	  {"image-height",     required_argument, NULL, 'y'},
	  {"algo-alpha",       required_argument, NULL, 'a'},
	  {"algo-fd",          required_argument, NULL, 'f'},
	  {"algo-bg",          required_argument, NULL, 'b'},
	  {"axi-debug",        no_argument,       NULL, 'v'},
	  {"mode",             required_argument, NULL, 'm'}
	};

	int c;
	while ((c = getopt_long(argc, argv, "vnp:d:x:y:a:f:b:m:h", long_options, NULL)) != -1) {
	switch (c) {
		case 'x':
			config.image_width = atoi(optarg);
			break;
		case 'y':
			config.image_height = atoi(optarg);
			break;
		case 'n':
			config.server_run = false;
			break;
		case 'p':
			config.server_port = atoi(optarg);
			break;
		case 'a':
			config.alpha = atof(optarg);
			break;
		case 'f':
			config.fd_th = atoi(optarg);
			break;
		case 'b':
			config.bg_th = atoi(optarg);
			break;
		case 'd':
			strcpy(config.server_dir, optarg);
			break;
		case 'v':
			set_verbose(true);
			break;
		case 'm':
			if (strcmp("algo", optarg) == 0)
				config.algo_mode = true;
			else if (strcmp("setup", optarg) == 0)
				config.algo_mode = false;
			else {
				help();
				exit(EXIT_FAILURE);
			}
			break;
		case 'h':
			help();
			exit(EXIT_SUCCESS);
		default:
			help();
			exit(EXIT_FAILURE);
		}
	}
}

void config_defaults() {
	config.image_width = 1280;
	config.image_height = 720;
	config.run = true;
	config.server_run = true;
	strcpy(config.server_dir, ".");
	config.server_port = 80;
	config.alpha = 0.1;
	config.fd_th = 30;
	config.bg_th = 30;
	config.algo_mode = true;
}

int main(int argc, char * argv[])
{
	config_defaults();
	parse_arguments(argc, argv);

	struct video_transmit *video_transmit = setup(config);

	struct server_ctx *server_ctx;
	if (config.algo_mode && config.server_run) {
		server_ctx = server_init(video_transmit, config.server_dir, config.server_port);
		pthread_t server_tid;

		if (pthread_create(&server_tid, NULL, &server_spawn, server_ctx) != 0) {
			perror("Failed to spawn server thread.");
			exit(1);
		}
	}

	if (!config.algo_mode)
		initialize(video_transmit);

	if (config.algo_mode && config.run)
		signal(SIGINT, interruptHandler);
	while (config.algo_mode && config.run) {
		debug_video(*video_transmit);
		usleep(1000000);
	}

	//cleanup(video_transmit);

	if (config.algo_mode && config.server_run) {
		server_stop(server_ctx);
		free(server_ctx);
	}
}
