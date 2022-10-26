#pragma once

#include<sys/socket.h>

#include"coroutines.h"

namespace xiangrpc {
	int accept_hook(int sockfd, struct sockaddr* addr, socklen_t* addrlen);

	ssize_t recv_hook(int fd, void* buf, size_t count);

	ssize_t send_hook(int fd, const void* buf, size_t count);

	int connect_hook(int sockfd, const struct sockaddr* addr, socklen_t addrlen);

}