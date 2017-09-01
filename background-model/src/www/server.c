#include "server.h"

#include "../algorithm/labeling.h"

#define BUFFER_SIZE  8096

static struct extension extensions[] = {
    {(char *)"gif",  (char *)"image/gif"},
    {(char *)"jpg",  (char *)"image/jpeg"},
    {(char *)"jpeg", (char *)"image/jpeg"},
    {(char *)"png",  (char *)"image/png"},
    {(char *)"zip",  (char *)"image/zip"},
    {(char *)"gz",   (char *)"image/gz"},
    {(char *)"tar",  (char *)"image/tar"},
    {(char *)"ico",  (char *)"image/x-icon"},
    {(char *)"htm",  (char *)"text/html"},
    {(char *)"html", (char *)"text/html"},
    {(char *)"php",  (char *)"image/php"},
    {(char *)"cgi",  (char *)"text/cgi"},
    {(char *)"asp",  (char *)"text/asp"},
    {(char *)"jsp",  (char *)"image/jsp"},
    {(char *)"xml",  (char *)"text/xml"},
    {(char *)"js",   (char *)"text/js"},
    {(char *)"css",  (char *)"test/css"},
    {(char *)"txt",  (char *)"test/plain"},
    {0,      0}
};
struct extension ext_unknown = 	{(char *)"blob", (char *)"octet-stream"};

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

static void dump_vdma(int conn_id, char filename[32], struct video_config config, struct axi_vdma vdma) {
	fdebug("[%6d] Writing vdma to file [%s].", conn_id, filename);
	dump_png_to_file(&filename[0], config, vdma);
}

void algo_to_file(int conn_id, char filename[32], struct server_ctx ctx) {
	fdebug("[%6d] Writing algorithm result to file [%s]. [MOCK]", conn_id, filename);
	int width = ctx.video_transmit->config->video.width;
	int height = ctx.video_transmit->config->video.height;

	long buffer_size = width * height * ctx.video_transmit->config->video.pixel_size;

	unsigned char buffer_in[buffer_size], buffer_out[buffer_size];

	memcpy(buffer_in, ctx.video_transmit->vdma_frame_buffer->framebuffers[0].virtual_addr, buffer_size);

	label_buffer(buffer_in, buffer_out, width, height);

	dump_buffer_to_file(filename, buffer_out, width, height);
}

static void update_index_file(int conn_id, struct video_config config) {
	fdebug("[%6d] Updating index file.", conn_id);
	static const char format[] = ""
			"<html>"
			"<head>"
			" <meta charset=\"UTF-8\">"
			"</head>"
			"<body>"
			"<table class=\"tg\">"
			"  <tr>"
			"    <td colspan=\"2\">Współczynnik ruchu: %d (0-255)</td>"
			"    <td colspan=\"2\">Współczynnik tła: %d (0-255)</td>"
			"    <td colspan=\"2\">Współczynnik bezwładności: %f (0.0-1.0)</td>"
			"  </tr>"
			"  <tr>"
			"    <td colspan=\"3\">"
			"    	<div style=\"text-align:center;\">Ramka obrazu</div>"
			"		<img src=\"vdma/frame-buffer\" width=\"600px\"/>"
			"	</td>"
			"    <td colspan=\"3\">"
			"    	<div style=\"text-align:center;\">Model tła</div>"
			"    	<img src=\"vdma/background-buffer\" width=\"600px\"/>"
			"	</td>"
			"  </tr>"
			"  <tr>"
			"    <td colspan=\"3\">"
			"    	<div style=\"text-align:center;\">Maska obiektów pierwszoplanowych</div>"
			"    	<img src=\"vdma/result-frame\" width=\"600px\"/>"
			"	</td>"
			"    <td colspan=\"3\">"
			"    	<div style=\"text-align:center;\">Wynik działania algorytmu</div>"
			"    	<img src=\"algorithm-result\" width=\"600px\"/>"
			"	</td>"
			"  </tr>"
			"</table></body>"
			"</html>";

	FILE * f = fopen("index.html", "w");
	fprintf(f, format, config.algo.bg_th, config.algo.fd_th, config.algo.alpha);

	fclose(f);
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

	if (is_path(buffer, "/vdma/frame-buffer\0")) {
		char vdma_filename[32];
		snprintf(vdma_filename, 32, "vdma.frame-buffer.%d.png", conn_id);

		dump_vdma(conn_id, vdma_filename, *ctx->video_transmit->config, *ctx->video_transmit->vdma_frame_buffer);
		snprintf(buffer, BUFFER_SIZE, "GET /%s", vdma_filename);
	}

	if (is_path(buffer, "/vdma/background-buffer\0")) {
		char vdma_filename[32];
		snprintf(vdma_filename, 32, "vdma.background-buffer.%d.png", conn_id);

		dump_vdma(conn_id, vdma_filename, *ctx->video_transmit->config, *ctx->video_transmit->vdma_background_buffer);
		snprintf(buffer, BUFFER_SIZE, "GET /%s", vdma_filename);
	}

	if (is_path(buffer, "/vdma/result-frame\0")) {
		char vdma_filename[32];
		snprintf(vdma_filename, 32, "vdma.result.%d.png", conn_id);

		dump_vdma(conn_id, vdma_filename, *ctx->video_transmit->config, *ctx->video_transmit->vdma_result_frame);
		snprintf(buffer, BUFFER_SIZE, "GET /%s", vdma_filename);
	}

	if (is_path(buffer, "/algorithm-result\0")) {
		char filename[32];
		snprintf(filename, 32, "algorithm.result.%d.png", conn_id);

		algo_to_file(conn_id, filename, *ctx);
		snprintf(buffer, BUFFER_SIZE, "GET /%s", filename);
	}


	if (is_path(buffer, "/\0")) {
		(void) strcpy(buffer, "GET /index.html");
		update_index_file(conn_id, *ctx->video_transmit->config);
	}

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
