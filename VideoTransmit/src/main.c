#define VDMA_BASE_ADDR				0x43100000
#define THRESHOLD_BASE_ADDR			0x13370000
#define HPD_BASE_ADDR				0x43200000

#include <getopt.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "axi/axi.h"
#include "debug.h"
#include "video_transmit.h"
#include "www/server.h"

static struct {
	int image_width;
	int image_height;

	uint16_t server_port;
	char server_dir[80];

	volatile bool run;
} config;

void interruptHandler(int sig) {
	fdebug("Signal %d received.", sig);
	config.run = false;
}

void* server_spawn(void *ctx) {
	server_listen((struct server_ctx *) ctx);
	return NULL;
}

void* overwrite_image(void *ctx) {
	while (1) {
//		debug_on_frame(*(((struct video_transmit*)ctx)->vdma));
		usleep(2000);
	}
	return NULL;
}

static void help() {
	debug("Options:");
	debug("\t--server-port      -p PORT");
	debug("\t--server-directory -d DIRECTORY_TO_SERVE");
	debug("\t--image-width 	    -x IMAGE_WIDTH_PIXELS");
	debug("\t--image-height     -y IMAGE_HEIGHT_PIXELS");
	debug("\t--help             -h");
}

void parse_arguments(int argc, char * argv[]) {
	static struct option long_options[] = {
	  {"server-port",      required_argument, NULL, 'p'},
	  {"server-directory", required_argument, NULL, 'd'},
	  {"help",      	   no_argument,       NULL, 'h'},
	  {"image-width", 	   required_argument, NULL, 'x'},
	  {"image-height",     required_argument, NULL, 'y'}
	};

	int c;
	while ((c = getopt_long(argc, argv, "p:d:x:y:h", long_options, NULL)) != -1) {
	switch (c) {
		case 'x':
			config.image_width = atoi(optarg);
			break;
		case 'y':
			config.image_height = atoi(optarg);
			break;
		case 'p':
			config.server_port = atoi(optarg);
			break;
		case 'd':
			strcpy(config.server_dir, optarg);
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

int main2(int argc, char * argv[])
{
	config.image_width = 1280;
	config.image_height = 720;
	config.run = true;
	strcpy(config.server_dir, ".");
	config.server_port = 80;

	parse_arguments(argc, argv);

	struct video_transmit *video_transmit = setup(VDMA_BASE_ADDR, HPD_BASE_ADDR, THRESHOLD_BASE_ADDR);

	struct server_ctx *server_ctx = server_init(video_transmit, config.server_dir, config.server_port);
	pthread_t server_tid, writer_tid;

	if (pthread_create(&server_tid, NULL, &server_spawn, server_ctx) != 0) {
		perror("Failed to spawn server thread.");
		exit(1);
	}

	if (pthread_create(&writer_tid, NULL, &overwrite_image, video_transmit) != 0) {
		perror("Failed to spawn writer thread.");
		exit(1);
	}

	initialize(video_transmit, config.image_width, config.image_height);

	if (config.run)
		signal(SIGINT, interruptHandler);
	while (config.run) {
		debug_video(*video_transmit);
		usleep(50000);
	}
	cleanup(video_transmit);
	server_stop(server_ctx);

	free(server_ctx);
}
