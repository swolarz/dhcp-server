#include "server.h"
#include "context.h"
#include "control.h"
#include "dhcp/inet/dhcppkt.h"
#include "dhcp/inet/endpoint.h"
#include "dhcp/handler/request.h"
#include "dhcp/data/store.h"
#include "utils/net.h"
#include "utils/log/log.h"
#include "config/dhcpconf.h"
#include "config/loader.h"

#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <signal.h>
#include <errno.h>


#define MAX_EPOLL_EVENTS 16


static const char* TAG = "SERVER";

static struct logger* loggr() {
	return context_get_logger();
}


typedef struct {
	struct dhcp_config* dhcpconf;
	struct db_connection* db_conn;
	int dhcp_sock;
	struct control_stream* ctl_stream;

} dhcp_server_context;


static int create_server_socket(const char* ifaddr, int port) {
	struct sockaddr_in saddr;
	memset(&saddr, 0, sizeof(struct sockaddr_in));

	int err = parse_inaddr(ifaddr, &(saddr.sin_addr));
	if (err < 0) {
		log_error(loggr(), "SERVER", "Error translating interface address");
		return -1;
	}

	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);

	socklen_t slen = sizeof(struct sockaddr_in);

	return dhcp_socket((struct sockaddr*) &saddr, slen);
}

static int init_dhcp_server(struct server_args* args, dhcp_server_context* server_context) {
	log_info(loggr(), TAG, "Initializing DHCP server...");
	
	memset(server_context, 0, sizeof(dhcp_server_context));

	struct dhcp_config* dhcpconf = dhcp_config_load();
	if (dhcpconf == NULL)
		return -1;

	struct db_connection* db_conn = init_db_connection();
	if (db_conn == NULL) {
		dhcp_config_cleanup(dhcpconf);
		return -1;
	}
	
	int dhcp_sock = create_server_socket(args->server_ifaddr, args->server_port);
	if (dhcp_sock < 0) {
		log_error(loggr(), "SERVER", "Failed to create socket from: ifaddr (%s) and port (%d): %s",
				args->server_ifaddr, args->server_port, strerror(errno));
	
		cleanup_db_connection(db_conn);
		dhcp_config_cleanup(dhcpconf);
		return -1;
	}

	struct control_stream* ctl_stream = register_management_listener();
	if (ctl_stream == NULL) {
		log_error(loggr(), "SERVER", "Failed to attach to management control");

		close(dhcp_sock);
		cleanup_db_connection(db_conn);
		dhcp_config_cleanup(dhcpconf);
		return -1;
	}

	server_context->dhcpconf = dhcpconf;
	server_context->db_conn = db_conn;
	server_context->dhcp_sock = dhcp_sock;
	server_context->ctl_stream = ctl_stream;

	return 0;
}

static void dhcp_server_cleanup(dhcp_server_context* server_context) {
	log_info(loggr(), "SERVER", "Disposing server resources...");

	dhcp_config_cleanup(server_context->dhcpconf);

	log_debug(loggr(), "SERVER", "Disposing database connection...");
	cleanup_db_connection(server_context->db_conn);

	log_debug(loggr(), "SERVER", "Closing server socket...");
	close(server_context->dhcp_sock);

	log_debug(loggr(), "SERVER", "Closing control stream...");
	unregister_management_listener(server_context->ctl_stream);
	
	log_info(loggr(), "SERVER", "Finalized DHCP server");
}

static int control_pipe_fd(dhcp_server_context* server_context) {
	return server_context->ctl_stream->ctl_pipe;
}

static int dhcp_socket_fd(dhcp_server_context* server_context) {
	return server_context->dhcp_sock;
}

static int setup_epoll(dhcp_server_context* server_context) {
	int epfd = epoll_create(1);
	if (epfd < 0) {
		log_error(loggr(), "SERVER", "Failed to init epoll: %s", strerror(errno));
		return -1;
	}

	int ctl_fd = control_pipe_fd(server_context);
	struct epoll_event ctl_event;
	memset(&ctl_event, 0, sizeof(struct epoll_event));

	ctl_event.events = EPOLLIN;
	ctl_event.data.fd = ctl_fd;

	int dhcp_fd = dhcp_socket_fd(server_context);
	struct epoll_event dhcp_event;
	memset(&dhcp_event, 0, sizeof(struct epoll_event));

	dhcp_event.events = EPOLLIN;
	dhcp_event.data.fd = dhcp_fd;

	int err;

	err = epoll_ctl(epfd, EPOLL_CTL_ADD, ctl_fd, &ctl_event);
	if (err < 0) {
		log_error(loggr(), "SERVER", "Failed to bind epoll control input: %s", strerror(errno));
		
		close(epfd);
		return -1;
	}

	err = epoll_ctl(epfd, EPOLL_CTL_ADD, dhcp_fd, &dhcp_event);
	if (err < 0) {
		log_error(loggr(), "SERVER", "Failed to bind epoll dhcp input: %s", strerror(errno));

		close(epfd);
		return -1;
	}

	return epfd;
}

static int control_exit_msg_exists(struct epoll_event events[], int nfds, int ctl_fd) {
	int i;
	for (i = 0; i < nfds; ++i) {
		int fd = events[i].data.fd;
		if (fd == ctl_fd) {
			char ctl_buff[8];
			int bytes = read(fd, ctl_buff, 8);

			if (bytes > 0)
				return 1;
		}
	}

	return 0;
}

static int dhcp_server_loop(dhcp_server_context* server_context, int port) {
	int epfd = setup_epoll(server_context);
	if (epfd < 0)
		return -1;

	struct epoll_event events[MAX_EPOLL_EVENTS];

	int ctl_fd = control_pipe_fd(server_context);
	int dhcp_fd = dhcp_socket_fd(server_context);

	while (1) {
		int nfds = epoll_wait(epfd, events, MAX_EPOLL_EVENTS, -1);

		if (nfds < 0 && errno == EINTR) {
			log_warn(loggr(), TAG, "Epoll wait interrupted");
			continue;
		}
		
		if (nfds < 0) {
			log_error(loggr(), "SERVER", "Epoll error while waiting for events: %s", strerror(errno));
			break;
		}

		if (control_exit_msg_exists(events, nfds, ctl_fd)) {
			log_info(loggr(), "SERVER", "Received control exit message");
			break;
		}

		int i;
		for (i = 0; i < nfds; ++i) {
			int fd = events[i].data.fd;

			if (fd == ctl_fd)
				continue;
			else if (fd == dhcp_fd)
				handle_dhcp_request(fd, port + 1, server_context->dhcpconf);
		}
	}

	close(epfd);
	return 0;
}

void dhcp_server_start(struct server_args* args) {
	int err;
	dhcp_server_context server_context;
	memset(&server_context, 0, sizeof(dhcp_server_context));

	err = init_dhcp_server(args, &server_context);
	if (err != 0) {
		log_error(loggr(), "SERVER", "Failed to initialize server context");
		return;
	}

	log_info(loggr(), "SERVER", "Started DHCP server at %s on port %d...",
			args->server_ifaddr, args->server_port);

	err = dhcp_server_loop(&server_context, args->server_port);
	if (err != 0) {
		log_error(loggr(), "SERVER", "Server main loop exited with failure");
	}
	
	dhcp_server_cleanup(&server_context);
}

