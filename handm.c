#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(void)
{
    if (fork() == 0)
    {
        system("./hand_mouse_core/hand_mouse_core");
        perror("system failed");
        exit(EXIT_FAILURE);
    }
    system("./hand_mouse_gui/hand_mouse_gui_bin");
    perror("system failed");
    exit(EXIT_FAILURE);
}
