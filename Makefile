CC      := gcc
CFLAGS  := -Wall -Wextra -O2 -Ihand_mouse_core -Ihand_mouse_core/cautogui
XFLAGS   := $(shell pkg-config --cflags --libs x11 xtst)

CORE_BIN := hand_mouse_core/hand_mouse_core
GUI_BIN  := hand_mouse_gui/hand_mouse_gui_bin
CLI_BIN  := hand_mouse_cli/hand_mouse_cli
MAIN := handm.c
NAME := handmouse

CORE_SRC := hand_mouse_core/main.c \
            hand_mouse_core/socket_server.c \
            hand_mouse_core/protocol.c \
            hand_mouse_core/cautogui/cautogui.c

GUI_SRC  := hand_mouse_gui/gui.c
CLI_SRC  := hand_mouse_cli/cli.c

all: $(CORE_BIN) $(GUI_BIN) $(CLI_BIN) $(NAME) $(PRMISSIONS)

$(CORE_BIN): $(CORE_SRC)
	$(CC) $(CFLAGS) $^ -o $@ $(XFLAGS)

$(GUI_BIN): $(GUI_SRC)
	$(CC) $(CFLAGS) $^ -o $@

$(CLI_BIN): $(CLI_SRC)
	$(CC) $(CFLAGS) $^ -o $@

$(NAME): $(MAIN)
	$(CC) $(CFLAGS) $^ -o $@ $(XFLAGS)

$(PRMISSIONS):
	chmod +x $(NAME) $(CORE_BIN) $(GUI_BIN) $(CLI_BIN)

clean fclean:
	rm $(CORE_BIN) $(GUI_BIN) $(CLI_BIN) $(NAME)

.PHONY: all clean

