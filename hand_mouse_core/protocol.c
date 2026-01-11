#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "protocol.h"
#include "cautogui/cautogui.h"

static void	handle_move(char *line)
{
	int	x;
	int	y;

	if (sscanf(line, "MOVE %d %d", &x, &y) == 2)
		moveTo(x, y);
}

static void	handle_simple(char *cmd)
{
	printf("cmd %s", cmd);
	if (strcmp(cmd, "CLICK") == 0)
	{
		click();
	}
	else if (strcmp(cmd, "DOWN") == 0)
		XTestFakeButtonEvent(display, 1, True, 0);
	else if (strcmp(cmd, "UP") == 0)
		XTestFakeButtonEvent(display, 1, False, 0);
}

void	handle_command(char *line)
{
	if (!line || !*line)
		return ;
	if (strncmp(line, "MOVE", 4) == 0)
		handle_move(line);
	else
		handle_simple(line);
	XFlush(display);
}
