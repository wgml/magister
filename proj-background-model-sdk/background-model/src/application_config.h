#ifndef SRC_APPLICATION_CONFIG_H_
#define SRC_APPLICATION_CONFIG_H_

#include <stdint.h>
#include <stdbool.h>

struct application_config {
	int image_width;
	int image_height;
	unsigned char bg_th;
	unsigned char fd_th;
	double alpha;

	bool server_run;
	uint16_t server_port;
	char server_dir[80];

	volatile bool run;

	bool algo_mode;
};



#endif /* SRC_APPLICATION_CONFIG_H_ */
