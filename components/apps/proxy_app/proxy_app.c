#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <unistd.h>
#include "proxy_app.h"

#define RPC_BUFF_SIZE 512
#define RPC_CHANNEL_READY_TO_CLOSE "rpc_channel_ready_to_close"

#ifndef RPROC_MODULE_NAME
#define RPROC_MODULE_NAME "zynqmp_r5_remoteproc"
#endif

#define SIZE_MODULE_NAME 50

struct _proxy_data {
	int active;
	int rpmsg_proxy_fd;
	struct _sys_rpc *rpc;
	struct _sys_rpc *rpc_response;
	char *firmware_path;
};

static struct _proxy_data *proxy;
char fw_dst_path[] = "/lib/firmware/image_rpc_demo";
char cp_cmd[512];
char module_name[SIZE_MODULE_NAME];

int handle_open(struct _sys_rpc *rpc)
{
	int fd, bytes_written;

	/* Open remote fd */
	fd = open(rpc->sys_call_args.data, rpc->sys_call_args.int_field1,
			rpc->sys_call_args.int_field2);

	/* Construct rpc response */
	proxy->rpc_response->id = OPEN_SYSCALL_ID;
	proxy->rpc_response->sys_call_args.int_field1 = fd;
	proxy->rpc_response->sys_call_args.int_field2 = 0; /*not used*/
	proxy->rpc_response->sys_call_args.data_len = 0; /*not used*/

	/* Transmit rpc response */
	bytes_written = write(proxy->rpmsg_proxy_fd, proxy->rpc_response,
				sizeof(struct _sys_rpc));

	return (bytes_written != sizeof(struct _sys_rpc)) ? -1 : 0;
}

int handle_close(struct _sys_rpc *rpc)
{
	int retval, bytes_written;

	/* Close remote fd */
	retval = close(rpc->sys_call_args.int_field1);

	/* Construct rpc response */
	proxy->rpc_response->id = CLOSE_SYSCALL_ID;
	proxy->rpc_response->sys_call_args.int_field1 = retval;
	proxy->rpc_response->sys_call_args.int_field2 = 0; /*not used*/
	proxy->rpc_response->sys_call_args.data_len = 0; /*not used*/

	/* Transmit rpc response */
	bytes_written = write(proxy->rpmsg_proxy_fd, proxy->rpc_response,
				sizeof(struct _sys_rpc));

	return (bytes_written != sizeof(struct _sys_rpc)) ? -1 : 0;
}

int handle_read(struct _sys_rpc *rpc)
{
	int bytes_read, bytes_written, payload_size;
	char *buff;

	/* Allocate buffer for requested data size */
	buff = malloc(rpc->sys_call_args.int_field2);

	if (rpc->sys_call_args.int_field1 == 0)
		/* Perform read from fd for large size since this is a
		STD/I request */
		bytes_read = read(rpc->sys_call_args.int_field1, buff, 512);
	else
		/* Perform read from fd */
		bytes_read = read(rpc->sys_call_args.int_field1, buff,
					rpc->sys_call_args.int_field2);

	/* Construct rpc response */
	proxy->rpc_response->id = READ_SYSCALL_ID;
	proxy->rpc_response->sys_call_args.int_field1 = bytes_read;
	proxy->rpc_response->sys_call_args.int_field2 = 0; /* not used */
	proxy->rpc_response->sys_call_args.data_len = bytes_read;
	if (bytes_read > 0)
		memcpy(proxy->rpc_response->sys_call_args.data, buff,
			bytes_read);

	payload_size = sizeof(struct _sys_rpc) +
			((bytes_read > 0) ? bytes_read : 0);

	/* Transmit rpc response */
	bytes_written = write(proxy->rpmsg_proxy_fd, proxy->rpc_response,
					payload_size);

	return (bytes_written != payload_size) ? -1 : 0;
}

int handle_write(struct _sys_rpc *rpc)
{
	int bytes_written;

	/* Write to remote fd */
	bytes_written = write(rpc->sys_call_args.int_field1,
				rpc->sys_call_args.data,
				rpc->sys_call_args.int_field2);

	/* Construct rpc response */
	proxy->rpc_response->id = WRITE_SYSCALL_ID;
	proxy->rpc_response->sys_call_args.int_field1 = bytes_written;
	proxy->rpc_response->sys_call_args.int_field2 = 0; /*not used*/
	proxy->rpc_response->sys_call_args.data_len = 0; /*not used*/

	/* Transmit rpc response */
	bytes_written = write(proxy->rpmsg_proxy_fd, proxy->rpc_response,
				sizeof(struct _sys_rpc));

	return (bytes_written != sizeof(struct _sys_rpc)) ? -1 : 0;
}

int handle_rpc(struct _sys_rpc *rpc)
{
	int retval;
	char *data = (char *)rpc;
	if (!strcmp(data, RPC_CHANNEL_READY_TO_CLOSE)) {
		proxy->active = 0;
		return 0;
	}

	/* Handle RPC */
	switch ((int)(rpc->id)) {
	case OPEN_SYSCALL_ID:
	{
		retval = handle_open(rpc);
		break;
	}
	case CLOSE_SYSCALL_ID:
	{
		retval = handle_close(rpc);
		break;
	}
	case READ_SYSCALL_ID:
	{
		retval = handle_read(rpc);
		break;
	}
	case WRITE_SYSCALL_ID:
	{
		retval = handle_write(rpc);
		break;
	}
	default:
	{
		printf("\r\nMaster>Err:Invalid RPC sys call ID: %d:%d! \r\n", rpc->id,WRITE_SYSCALL_ID);
		retval = -1;
		break;
	}
	}

	return retval;
}

int terminate_rpc_app()
{
	int bytes_written;
	int msg = TERM_SYSCALL_ID;
	printf ("Master> sending shutdown signal.\n");
	bytes_written = write(proxy->rpmsg_proxy_fd, &msg, sizeof(int));
	return bytes_written;
}

void exit_action_handler(int signum)
{
	proxy->active = 0;
}

void kill_action_handler(int signum)
{
	printf("\r\nMaster>RPC service killed !!\r\n");

	/* Send shutdown signal to remote application */
	terminate_rpc_app();

	/* wait for a while to let the remote finish cleanup */
	sleep(1);
	/* Close proxy rpmsg device */
	close(proxy->rpmsg_proxy_fd);

	/* Free up resources */
	free(proxy->rpc);
	free(proxy->rpc_response);
	free(proxy);

	/* Unload drivers */
	system("modprobe -r rpmsg_proxy_dev_driver");
	system("modprobe -r virtio_rpmsg_bus");
	sprintf(cp_cmd, "modprobe -r %s", module_name);
	system(cp_cmd);
	system("modprobe -r remoteproc");
	system("modprobe -r virtio_ring");
	system("modprobe -r virtio");
}

void display_help_msg()
{
	printf("\r\nLinux proxy application.\r\n");
	printf("-v	 Displays proxy application version.\n");
	printf("-f	 Accepts path of firmware to load on remote core.\n");
	printf("-r       Which core 0|1\n");
	printf("-m	 rproc module name.\n");
	printf("-h	 Displays this help message.\n");
}


int main(int argc, char *argv[])
{
	struct sigaction exit_action;
	struct sigaction kill_action;
	unsigned int bytes_rcvd;
	int i = 0;
	int opt = 0;
	int ret = 0;
	int r5_id = 0;
	char *user_fw_path = 0;
        
        strcpy(module_name, RPROC_MODULE_NAME);

	/* Initialize signalling infrastructure */
	memset(&exit_action, 0, sizeof(struct sigaction));
	memset(&kill_action, 0, sizeof(struct sigaction));
	exit_action.sa_handler = exit_action_handler;
	kill_action.sa_handler = kill_action_handler;
	sigaction(SIGTERM, &exit_action, NULL);
	sigaction(SIGINT, &exit_action, NULL);
	sigaction(SIGKILL, &kill_action, NULL);
	sigaction(SIGHUP, &kill_action, NULL);

	while ((opt = getopt(argc, argv, "vhf:r:m:")) != -1) {
		switch (opt) {
		case 'f':
			user_fw_path = optarg;
			break;
		case 'r':
			r5_id = atoi(optarg);
			if (r5_id != 0 && r5_id != 1) {
				display_help_msg();
				return -1;
			}
			break;
		case 'm':
		 	strncpy(module_name, optarg, SIZE_MODULE_NAME);
		 	module_name[SIZE_MODULE_NAME - 1] = '\0';
			break;
		case 'v':
			printf("\r\nLinux proxy application version 1.0\r\n");
			return 0;
		case 'h':
			display_help_msg();
			return 0;
		default:
			printf("getopt return unsupported option: -%c\n",opt);
			break;
		} 
	}
	/* Parse and process input args */
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-v") == 0) {
			printf("\r\nLinux proxy application version 1.0\r\n");
			return 0;
		} else if (strcmp(argv[i], "-h") == 0) {
			display_help_msg();
			return 0;
		} else if (strcmp(argv[i], "-f") == 0) {
			if (i+1 < argc)
				/* Construct file copy command string */
				sprintf(cp_cmd , "cp %s %s", argv[i+1],
						fw_dst_path);
		}
	}

	/* Bring up remote firmware */
	printf("\r\nMaster>Loading remote firmware\r\n");
	if (user_fw_path) {
		sprintf(cp_cmd, "cp %s %s", user_fw_path, fw_dst_path);
		system(cp_cmd);
	}
	sprintf(cp_cmd, "modprobe %s firmware", module_name);
	if (r5_id == 1)
		sprintf(&cp_cmd[strlen(cp_cmd)],"1=image_rpc_demo");
	else
		sprintf(&cp_cmd[strlen(cp_cmd)],"=image_rpc_demo");
	system(cp_cmd);

	/* Create rpmsg proxy device */
	printf("\r\nMaster>Create rpmsg proxy device\r\n");
	system("modprobe rpmsg_proxy_dev_driver");

	/* Allocate memory for proxy data structure */
	proxy = malloc(sizeof(struct _proxy_data));
	if (proxy == 0) {
		printf("\r\nMaster>Failed to allocate memory.\r\n");
		return -1;
	}
	proxy->active = 1;

	/* Open proxy rpmsg device */
	printf("\r\nMaster>Opening rpmsg proxy device\r\n");
	i = 0;
	do {
		proxy->rpmsg_proxy_fd = open("/dev/rpmsg_proxy0", O_RDWR);
		sleep(1);
	} while (proxy->rpmsg_proxy_fd < 0 && (i++ < 2));

	if (proxy->rpmsg_proxy_fd < 0) {
		printf("\r\nMaster>Failed to open rpmsg proxy driver device file.\r\n");
		ret = -1;
		goto error0;
	}

	/* Allocate memory for rpc payloads */
	proxy->rpc = malloc(RPC_BUFF_SIZE);
	proxy->rpc_response = malloc(RPC_BUFF_SIZE);

	/* RPC service starts */
	printf("\r\nMaster>RPC service started !!\r\n");
	while (proxy->active) {
		/* Block on read for rpc requests from remote context */
		do {
			bytes_rcvd = read(proxy->rpmsg_proxy_fd, proxy->rpc,
					RPC_BUFF_SIZE);
			if (!proxy->active)
				break;
		} while(bytes_rcvd <= 0);

		/* User event, break! */
		if (!proxy->active)
			break;

		/* Handle rpc */
		if (handle_rpc(proxy->rpc)) {
			printf("\r\nMaster>Err:Handling remote procedure");
			printf(" call!\r\n");
			printf("\r\nrpc id %d\r\n", proxy->rpc->id);
			printf("\r\nrpc int field1 %d\r\n",
				proxy->rpc->sys_call_args.int_field1);
			printf("\r\nrpc int field2 %d\r\n",
				proxy->rpc->sys_call_args.int_field2);
			break;
		}
	}

	printf("\r\nMaster>RPC service exiting !!\r\n");

	/* Send shutdown signal to remote application */
	terminate_rpc_app();

	/* Need to wait here for sometime to allow remote application to
 	complete its unintialization */
	sleep(1);

	/* Close proxy rpmsg device */
	close(proxy->rpmsg_proxy_fd);

	/* Free up resources */
	free(proxy->rpc);
	free(proxy->rpc_response);

error0:
	free(proxy);

	/* Unload drivers */
	system("modprobe -r rpmsg_proxy_dev_driver");
	system("modprobe -r virtio_rpmsg_bus");
	sprintf(cp_cmd, "modprobe -r %s", module_name);
	system(cp_cmd);
	system("modprobe -r remoteproc");
	system("modprobe -r virtio_ring");
	system("modprobe -r virtio");

	return ret;
}

