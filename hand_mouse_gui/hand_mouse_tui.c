#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

#define CONFIG_PATH "config/hand_mouse.conf"

/* ================= CONFIG STRUCT ================= */

typedef struct s_config
{
	int		screen_width;
	int		screen_height;
	float	pinch_threshold;
	float	smoothing;
	float	click_max_duration;
	float	drag_min_duration;
}	t_config;

/* ================= PARAM TABLE ================= */

typedef enum e_type
{
	T_INT,
	T_FLOAT
}	t_type;

typedef struct s_param
{
	const char	*name;
	void		*ptr;
	t_type		type;
	const char	*fmt;
}	t_param;

/* ================= GLOBAL ================= */

static pid_t	g_python_pid = 0;

/* ================= CONFIG ================= */

static t_config	load_config(void)
{
	t_config	cfg = {1920, 1080, 0.04f, 0.5f, 0.7f, 0.8f};
	FILE		*f;
	char		line[256];
	char		key[64];
	char		val[64];

	f = fopen(CONFIG_PATH, "r");
	if (!f)
		return (cfg);
	while (fgets(line, sizeof(line), f))
	{
		if (line[0] == '#' || line[0] == '\n')
			continue ;
		if (sscanf(line, "%63[^=]=%63s", key, val) != 2)
			continue ;
		if (!strcmp(key, "screen_width"))
			cfg.screen_width = atoi(val);
		else if (!strcmp(key, "screen_height"))
			cfg.screen_height = atoi(val);
		else if (!strcmp(key, "pinch_threshold"))
			cfg.pinch_threshold = atof(val);
		else if (!strcmp(key, "smoothing"))
			cfg.smoothing = atof(val);
		else if (!strcmp(key, "click_max_duration"))
			cfg.click_max_duration = atof(val);
		else if (!strcmp(key, "drag_min_duration"))
			cfg.drag_min_duration = atof(val);
	}
	fclose(f);
	return (cfg);
}

static void	save_config(t_config *c)
{
	FILE	*f;

	f = fopen(CONFIG_PATH, "w");
	if (!f)
		return ;
	fprintf(f, "screen_width=%d\n", c->screen_width);
	fprintf(f, "screen_height=%d\n", c->screen_height);
	fprintf(f, "pinch_threshold=%.3f\n", c->pinch_threshold);
	fprintf(f, "smoothing=%.2f\n", c->smoothing);
	fprintf(f, "click_max_duration=%.1f\n", c->click_max_duration);
	fprintf(f, "drag_min_duration=%.1f\n", c->drag_min_duration);
	fclose(f);
}

/* ================= PROCESS ================= */

static void	start_program(void)
{
	pid_t	pid;

	if (g_python_pid > 0)
		return ;
	pid = fork();
	if (pid == 0)
	{
		execlp("python3", "python3",
			"vision_python/socket_client.py", NULL);
		exit(1);
	}
	if (pid > 0)
		g_python_pid = pid;
}

static void	stop_program(void)
{
	if (g_python_pid > 0)
	{
		kill(g_python_pid, SIGTERM);
		g_python_pid = 0;
	}
}

/* ================= UI ================= */

static void	edit_param(t_param *p)
{
	char	buf[64];

	printf("\nEnter new value for %s: ", p->name);
	fflush(stdout);
	if (!fgets(buf, sizeof(buf), stdin))
		return ;
	if (p->type == T_INT)
		*(int *)p->ptr = atoi(buf);
	else
		*(float *)p->ptr = atof(buf);
}

static void	display_menu(t_param *params, int count)
{
	int	i;

	printf("\n═══════════════════════════════════════════════\n");
	printf("        HAND MOUSE CONTROL PANEL\n");
	printf("═══════════════════════════════════════════════\n\n");

	if (g_python_pid > 0)
		printf("  Status: \033[32m● RUNNING\033[0m (PID: %d)\n\n",
			g_python_pid);
	else
		printf("  Status: \033[31m○ STOPPED\033[0m\n\n");

	printf("  [1] %s\n\n",
		g_python_pid > 0 ? "Stop Program" : "Start Program");

	printf("  Configuration:\n");
	i = 0;
	while (i < count)
	{
		printf("  [%d] ", i + 2);
		if (params[i].type == T_INT)
			printf(params[i].fmt, *(int *)params[i].ptr);
		else
			printf(params[i].fmt, *(float *)params[i].ptr);
		printf("\n");
		i++;
	}
	printf("\n  [%d] Save Configuration\n", count + 2);
	printf("  [%d] Reload Configuration\n", count + 3);
	printf("  [0] Exit\n");
	printf("\n═══════════════════════════════════════════════\n");
	printf("Select option: ");
	fflush(stdout);
}

/* ================= MAIN ================= */

int	main(void)
{
	t_config	cfg;
	t_param		params[] = {
		{"Screen Width", &cfg.screen_width, T_INT,
			"Screen Width:         %d"},
		{"Screen Height", &cfg.screen_height, T_INT,
			"Screen Height:        %d"},
		{"Pinch Threshold", &cfg.pinch_threshold, T_FLOAT,
			"Pinch Threshold:      %.3f"},
		{"Smoothing", &cfg.smoothing, T_FLOAT,
			"Smoothing:            %.2f"},
		{"Click Max Duration", &cfg.click_max_duration, T_FLOAT,
			"Click Max Duration:   %.1f"},
		{"Drag Min Duration", &cfg.drag_min_duration, T_FLOAT,
			"Drag Min Duration:    %.1f"}
	};
	int			count;
	char		buf[32];
	int			choice;

	cfg = load_config();
	count = sizeof(params) / sizeof(params[0]);
	while (1)
	{
		display_menu(params, count);
		if (!fgets(buf, sizeof(buf), stdin))
			break ;
		choice = atoi(buf);
		if (choice == 0)
			break ;
		else if (choice == 1)
			(g_python_pid > 0) ? stop_program() : start_program();
		else if (choice >= 2 && choice < count + 2)
			edit_param(&params[choice - 2]);
		else if (choice == count + 2)
			save_config(&cfg);
		else if (choice == count + 3)
			cfg = load_config();
		printf("\nPress ENTER to continue...");
		fgets(buf, sizeof(buf), stdin);
	}
	if (g_python_pid > 0)
		stop_program();
	return (0);
}
