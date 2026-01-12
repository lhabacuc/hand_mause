#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

#define CONFIG_PATH "config/hand_mouse.conf"

typedef struct s_config
{
	int		screen_width;
	int		screen_height;
	float	pinch_threshold;
	float	smoothing;
	float	click_max_duration;
	float	drag_min_duration;
}	t_config;

static pid_t	g_python_pid = 0;

/* Load configuration from file */
static t_config	load_config(void)
{
	t_config	cfg = {1920, 1080, 0.04f, 0.5f, 0.7f, 0.8f};
	FILE		*f;
	char		key[64], value[64], line[256];

	f = fopen(CONFIG_PATH, "r");
	if (!f)
		return (cfg);
	while (fgets(line, sizeof(line), f))
	{
		if (line[0] == '#' || line[0] == '\n')
			continue;
		if (sscanf(line, "%63[^=]=%63s", key, value) == 2)
		{
			if (strcmp(key, "screen_width") == 0)
				cfg.screen_width = atoi(value);
			else if (strcmp(key, "screen_height") == 0)
				cfg.screen_height = atoi(value);
			else if (strcmp(key, "pinch_threshold") == 0)
				cfg.pinch_threshold = atof(value);
			else if (strcmp(key, "smoothing") == 0)
				cfg.smoothing = atof(value);
			else if (strcmp(key, "click_max_duration") == 0)
				cfg.click_max_duration = atof(value);
			else if (strcmp(key, "drag_min_duration") == 0)
				cfg.drag_min_duration = atof(value);
		}
	}
	fclose(f);
	return (cfg);
}

/* Save configuration to file */
static void	save_config(t_config *cfg)
{
	FILE	*f;

	f = fopen(CONFIG_PATH, "w");
	if (!f)
	{
		perror("Failed to save config");
		return ;
	}
	fprintf(f, "# Hand Mouse Configuration\n");
	fprintf(f, "screen_width=%d\n", cfg->screen_width);
	fprintf(f, "screen_height=%d\n", cfg->screen_height);
	fprintf(f, "pinch_threshold=%.3f\n", cfg->pinch_threshold);
	fprintf(f, "smoothing=%.2f\n", cfg->smoothing);
	fprintf(f, "click_max_duration=%.1f\n", cfg->click_max_duration);
	fprintf(f, "drag_min_duration=%.1f\n", cfg->drag_min_duration);
	fclose(f);
}

/* Start Python program */
static void	start_program(void)
{
	pid_t	pid;

	if (g_python_pid > 0)
	{
		printf("Program already running!\n");
		return ;
	}
	pid = fork();
	if (pid == 0)
	{
		execlp("python3", "python3", "vision_python/socket_client.py", NULL);
		perror("execlp failed");
		exit(1);
	}
	else if (pid > 0)
	{
		g_python_pid = pid;
		printf("✓ Hand Mouse started (PID: %d)\n", pid);
	}
	else
		perror("fork failed");
}

/* Stop Python program */
static void	stop_program(void)
{
	if (g_python_pid <= 0)
	{
		printf("No program running!\n");
		return ;
	}
	kill(g_python_pid, SIGTERM);
	printf("✓ Hand Mouse stopped (PID: %d)\n", g_python_pid);
	g_python_pid = 0;
}

/* Display menu */
static void	display_menu(t_config *cfg)
{
	printf("\n");
	printf("═══════════════════════════════════════════════\n");
	printf("        HAND MOUSE CONTROL PANEL\n");
	printf("═══════════════════════════════════════════════\n");
	printf("\n");
	
	if (g_python_pid > 0)
		printf("  Status: \033[32m● RUNNING\033[0m (PID: %d)\n", g_python_pid);
	else
		printf("  Status: \033[31m○ STOPPED\033[0m\n");
	
	printf("\n");
	printf("  [1] %s\n", g_python_pid > 0 ? "Stop Program" : "Start Program");
	printf("\n");
	printf("  Configuration:\n");
	printf("  [2] Screen Width:         %d\n", cfg->screen_width);
	printf("  [3] Screen Height:        %d\n", cfg->screen_height);
	printf("  [4] Pinch Threshold:      %.3f\n", cfg->pinch_threshold);
	printf("  [5] Smoothing:            %.2f\n", cfg->smoothing);
	printf("  [6] Click Max Duration:   %.1f\n", cfg->click_max_duration);
	printf("  [7] Drag Min Duration:    %.1f\n", cfg->drag_min_duration);
	printf("\n");
	printf("  [8] Save Configuration\n");
	printf("  [9] Reload Configuration\n");
	printf("  [0] Exit\n");
	printf("\n");
	printf("═══════════════════════════════════════════════\n");
	printf("Select option: ");
	fflush(stdout);
}

/* Edit configuration value */
static void	edit_value(const char *name, void *ptr, int is_int)
{
	char	input[64];
	
	printf("\nEnter new value for %s: ", name);
	fflush(stdout);
	if (fgets(input, sizeof(input), stdin))
	{
		if (is_int)
			*(int *)ptr = atoi(input);
		else
			*(float *)ptr = atof(input);
		printf("✓ Updated %s\n", name);
	}
}

/* Main program */
int	main(void)
{
	t_config	cfg;
	char		input[64];
	int			choice;

	cfg = load_config();
	
	while (1)
	{
		display_menu(&cfg);
		
		if (!fgets(input, sizeof(input), stdin))
			break;
		
		choice = atoi(input);
		
		switch (choice)
		{
			case 1:
				if (g_python_pid > 0)
					stop_program();
				else
					start_program();
				break;
			case 2:
				edit_value("Screen Width", &cfg.screen_width, 1);
				break;
			case 3:
				edit_value("Screen Height", &cfg.screen_height, 1);
				break;
			case 4:
				edit_value("Pinch Threshold", &cfg.pinch_threshold, 0);
				break;
			case 5:
				edit_value("Smoothing", &cfg.smoothing, 0);
				break;
			case 6:
				edit_value("Click Max Duration", &cfg.click_max_duration, 0);
				break;
			case 7:
				edit_value("Drag Min Duration", &cfg.drag_min_duration, 0);
				break;
			case 8:
				save_config(&cfg);
				printf("✓ Configuration saved to %s\n", CONFIG_PATH);
				break;
			case 9:
				cfg = load_config();
				printf("✓ Configuration reloaded\n");
				break;
			case 0:
				if (g_python_pid > 0)
				{
					printf("\nStopping program before exit...\n");
					stop_program();
				}
				printf("Goodbye!\n");
				return (0);
			default:
				printf("Invalid option!\n");
		}
		
		printf("\nPress ENTER to continue...");
		fgets(input, sizeof(input), stdin);
	}
	
	if (g_python_pid > 0)
		stop_program();
	
	return (0);
}
