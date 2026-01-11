#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "protocol.h"

#define SOCK_PATH "/tmp/hand_mouse.sock"
#define BUF_SIZE 256

static int	create_socket(void)
{
	int					fd;
	struct sockaddr_un	addr;

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0)
		return (-1);
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, SOCK_PATH,
		sizeof(addr.sun_path) - 1);
	unlink(SOCK_PATH);
	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		return (-1);
	if (listen(fd, 1) < 0)
		return (-1);
	return (fd);
}

void	socket_loop(void)
{
	int		server;
	int		client;
	char	buf[BUF_SIZE];
	char	line[BUF_SIZE];
	ssize_t	n;
	int		line_pos = 0;
	int		i;

	server = create_socket();
	if (server < 0)
		return ;
	client = accept(server, NULL, NULL);
	if (client < 0)
		return ;
	while ((n = read(client, buf, BUF_SIZE - 1)) > 0)
	{
		i = 0;
		while (i < n)
		{
			if (buf[i] == '\n')
			{
				line[line_pos] = '\0';
				handle_command(line);
				line_pos = 0;
			}
			else if (line_pos < BUF_SIZE - 1)
			{
				line[line_pos++] = buf[i];
			}
			i++;
		}
	}
	close(client);
	close(server);
	unlink(SOCK_PATH);
}
