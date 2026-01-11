#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCK_PATH "/tmp/hand_mouse.sock"

int main(void)
{
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket");
        return 1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path)-1);

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        perror("connect");
        return 1;
    }

    char line[256];
    printf("Digite comandos (MOVE x y, CLICK, DOWN, UP) ou 'exit'\n");
    while (1)
    {
        printf("> ");
        if (!fgets(line, sizeof(line), stdin))
            break;
        if (strncmp(line, "exit", 4) == 0)
            break;
        write(sock, line, strlen(line));
    }

    close(sock);
    return 0;
}
