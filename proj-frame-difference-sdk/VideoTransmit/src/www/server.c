#include "server.h"

#define BUFFER_SIZE  8096

static struct extension extensions[] = {
    {"gif",  "image/gif"},
    {"jpg",  "image/jpeg"},
    {"jpeg", "image/jpeg"},
    {"png",  "image/png"},
    {"zip",  "image/zip"},
    {"gz",   "image/gz"},
    {"tar",  "image/tar"},
    {"ico",  "image/x-icon"},
    {"htm",  "text/html"},
    {"html", "text/html"},
    {"php",  "image/php"},
    {"cgi",  "text/cgi"},
    {"asp",  "text/asp"},
    {"jsp",  "image/jsp"},
    {"xml",  "text/xml"},
    {"js",   "text/js"},
    {"css",  "test/css"},
    {"txt",  "test/plain"},
    {0,      0}
};
struct extension ext_unknown = 	{"blob", "octet-stream"};

static uint16_t get_port(uint16_t port) {
	if (port < 1 || port > 31000) {
		fdebug("Illegal port value provided [%d].", port);
		exit(EXIT_FAILURE);
	}
	return port;
}

static bool is_path(char buffer[BUFFER_SIZE + 1], const char *path) {
	return !strncmp(&buffer[4], path, strlen(path) + 1);
}

static void server_serve(struct server_ctx * ctx, int conn_fd, int conn_id) {
	static char buffer[BUFFER_SIZE + 1];
	fdebug("[%6d] Serving for new request.", conn_id);

	long rq_length = read(conn_fd, buffer, BUFFER_SIZE);
	if (rq_length <= 0) {
		fdebug("[%6d] Failed to read request.", conn_id);
		exit(1);
	}
	if (rq_length < BUFFER_SIZE)
		buffer[rq_length] = '\0';

	for (int i = 0; i < rq_length; i++)
		if (buffer[i] == '\r' || buffer[i] == '\n')
			buffer[i] = '*';
	fdebug("[%6d] Request is:\n%s", conn_id, buffer);

	if (strncmp(buffer, "GET ", 4) && strncmp(buffer, "get ", 4)) {
		fdebug("[%6d] Unsupported method.", conn_id);
		exit(1);
	}

	for (int i = 4; i < BUFFER_SIZE; i++) {
		if (buffer[i] == ' ') {
			buffer[i] = 0;
			break;
		}
	}

	for (int j = 0; j < rq_length - 1; j++)
		if (buffer[j] == '.' && buffer[j + 1] == '.') {
			fdebug("[%6d] Unsupported path.", conn_id);
			exit(1);
		}

	if (is_path(buffer, "/vdma\0")) {
		// dump vdma image to file and modify path (lazy impl though)
		char vdma_filename[16];
		snprintf(vdma_filename, 16, "vdma.%d.png", conn_id);
		fdebug("[%6d] Writing vdma to temporary file [%s].", conn_id, vdma_filename);
		dump_png_to_file(&vdma_filename[0], ctx->video_transmit);
		snprintf(buffer, BUFFER_SIZE, "GET /%s", vdma_filename);
	}

	if (is_path(buffer, "/\0"))
		(void) strcpy(buffer, "GET /index.html");

	fdebug("[%6d] Asked for file [%s].", conn_id, &buffer[5]);
	size_t buflen = strlen(buffer);
	char *fstr = ext_unknown.filetype;
	for (int i = 0; extensions[i].ext != 0; i++) {
		size_t len = strlen(extensions[i].ext);
		if (is_path(&buffer[buflen - len], extensions[i].ext)) {
			fstr = extensions[i].filetype;
			break;
		}
	}

	char file_path[120];
	strcpy(file_path, ctx->dir);
	strcat(file_path, &buffer[5]);
	int file_fd;
	if ((file_fd = open(file_path, O_RDONLY)) == -1) {
		fdebug("[%6d] Failed to open file.", conn_id);
		exit(1);
	}
	sprintf(buffer, "HTTP/1.0 200 OK\r\nContent-Type: %s\r\n\r\n", fstr);
	write(conn_fd, buffer, strlen(buffer));

	size_t res_len;
	while ((res_len = (size_t) read(file_fd, buffer, BUFFER_SIZE)) > 0)
		write(conn_fd, buffer, res_len);

	fdebug("[%6d] Response served, killing myself...", conn_id);
	exit(1);
}

struct server_ctx* server_init(struct video_transmit *video_transmit, char server_dir[80], uint16_t server_port) {
	struct server_ctx *ctx = (struct server_ctx*) malloc(sizeof(struct server_ctx));

	ctx->video_transmit = video_transmit;

	ctx->port = get_port(server_port);
	strcpy(ctx->dir, server_dir);
	ctx->listening = false;

	if (ctx->dir[strlen(ctx->dir)] != '\n')
		strcat(ctx->dir, "/");

	return ctx;
}

void server_stop(struct server_ctx *ctx) {
	debug("Stopping server.");
	ctx->listening = false;
}


void server_listen(struct server_ctx *ctx) {
	static struct sockaddr_in client_addr;
	static struct sockaddr_in serv_addr;
	int conn_id, conn_fd;
	size_t length;

	fdebug("Will start server on port [%d] to serve [%s] directory.",
	ctx->port, ctx->dir);
	fdebug("Ctrl-C or kill pid [%d] to stop.", getpid());

	if ((ctx->listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fdebug("Error building socket. errno=%d", errno);
	} else {
		fdebug("Socket listen fd is [%d].", ctx->listen_fd);
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(ctx->port);

	if (bind(ctx->listen_fd, (struct sockaddr *) &(serv_addr),
			sizeof(serv_addr)) < 0) {
		debug("Bind failed.");
		exit(EXIT_FAILURE);
	}

	if (listen(ctx->listen_fd, 64) < 0) {
		debug("Cannot start listening.");
		exit(EXIT_FAILURE);
	}

	ctx->listening = true;
	debug("Listening...");

	for (conn_id = 1; ctx->listening; conn_id++) {
		length = sizeof(client_addr);
		conn_fd = accept(ctx->listen_fd, (struct sockaddr *) &client_addr, &length);

		fdebug("[%6d] Connection accepted with connection fd %d.", conn_id, conn_fd);
		if (conn_fd < 0) {
			fdebug("[%6d] Connection acceptance failed with error [%d].", conn_id, errno);
			continue;
		}
		int pid = fork();
		if (pid < 0) {
			fdebug("[%6d] Failed to fork process!", conn_id);
		} else {
			if (pid == 0) { // child
				close(ctx->listen_fd);
				server_serve(ctx, conn_fd, conn_id);
			} else { // parent
				close(conn_fd);
			}
		}
	}
}
