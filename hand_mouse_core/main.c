#include <stdio.h>
#include "cautogui/cautogui.h"

void	socket_loop(void);

int	main(void)
{
	init();
	printf("hand_mouse_core iniciado\n");
	socket_loop();
	return (0);
}
