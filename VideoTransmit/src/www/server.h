#ifndef SRC_SERVER_H_
#define SRC_SERVER_H_

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>

#include "dump_png.h"
#include "../debug.h"
#include "../video_transmit.h"

struct extension {
  char *ext;
  char *filetype;
};

struct server_ctx {
  uint16_t port;
  char dir[80];
  int listen_fd;
  volatile bool listening;

  struct video_transmit *video_transmit;
};

struct server_ctx* server_init(struct video_transmit *video_transmit, char server_dir[80], uint16_t server_port);

void server_listen(struct server_ctx* ctx);

void server_stop(struct server_ctx* ctx);

#endif /* SRC_SERVER_H_ */
