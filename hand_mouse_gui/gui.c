#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#define CONFIG_PATH "config/hand_mouse.conf"

typedef struct s_config
{
    int screen_width;
    int screen_height;
    double pinch_threshold;
    double smoothing;
    double click_max_duration;
    double drag_min_duration;
}   t_config;

typedef struct s_app_data
{
    t_config cfg;
    GtkWidget *window;
    GtkWidget *start_btn;
    GtkWidget *status_label;
    
    /* Widgets para configuração */
    GtkWidget *width_entry;
    GtkWidget *height_entry;
    GtkWidget *pinch_scale;
    GtkWidget *smooth_scale;
    GtkWidget *click_scale;
    GtkWidget *drag_scale;
    
    /* Entry fields que mostram valores dos sliders */
    GtkWidget *pinch_entry;
    GtkWidget *smooth_entry;
    GtkWidget *click_entry;
    GtkWidget *drag_entry;
    
    pid_t python_pid;
    gboolean is_running;
}   t_app_data;

/* Funções utilitárias */
static t_config load_config(void)
{
    t_config cfg = {1920, 1080, 0.04, 0.5, 0.7, 0.8};
    FILE *f = fopen(CONFIG_PATH, "r");
    if (!f)
        return cfg;

    char line[256];
    while (fgets(line, sizeof(line), f))
    {
        if (line[0] == '#' || line[0] == '\n')
            continue;
        
        char key[64];
        char value[64];
        if (sscanf(line, "%63[^=]=%63s", key, value) == 2)
        {
            if (strcmp(key, "SCREEN_WIDTH") == 0)
                cfg.screen_width = atoi(value);
            else if (strcmp(key, "SCREEN_HEIGHT") == 0)
                cfg.screen_height = atoi(value);
            else if (strcmp(key, "PINCH_THRESHOLD") == 0)
                cfg.pinch_threshold = atof(value);
            else if (strcmp(key, "SMOOTHING") == 0)
                cfg.smoothing = atof(value);
            else if (strcmp(key, "CLICK_MAX_DURATION") == 0)
                cfg.click_max_duration = atof(value);
            else if (strcmp(key, "DRAG_MIN_DURATION") == 0)
                cfg.drag_min_duration = atof(value);
        }
    }
    fclose(f);
    return cfg;
}

static void save_config(t_config cfg)
{
    FILE *f = fopen(CONFIG_PATH, "w");
    if (!f)
    {
        g_print("Erro ao abrir arquivo de configuração!\n");
        return;
    }
    fprintf(f, "# Configurações do hand_mouse\n");
    fprintf(f, "SCREEN_WIDTH=%d\n", cfg.screen_width);
    fprintf(f, "SCREEN_HEIGHT=%d\n", cfg.screen_height);
    fprintf(f, "PINCH_THRESHOLD=%f\n", cfg.pinch_threshold);
    fprintf(f, "SMOOTHING=%f\n", cfg.smoothing);
    fprintf(f, "CLICK_MAX_DURATION=%f\n", cfg.click_max_duration);
    fprintf(f, "DRAG_MIN_DURATION=%f\n", cfg.drag_min_duration);
    fclose(f);
}

/* Callbacks para atualizar entries quando sliders mudam */
static void on_pinch_scale_changed(GtkRange *range, gpointer data)
{
    t_app_data *app = (t_app_data *)data;
    double value = gtk_range_get_value(range);
    app->cfg.pinch_threshold = value;
    
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%.3f", value);
    gtk_entry_set_text(GTK_ENTRY(app->pinch_entry), buffer);
}

static void on_smooth_scale_changed(GtkRange *range, gpointer data)
{
    t_app_data *app = (t_app_data *)data;
    double value = gtk_range_get_value(range);
    app->cfg.smoothing = value;
    
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%.2f", value);
    gtk_entry_set_text(GTK_ENTRY(app->smooth_entry), buffer);
}

static void on_click_scale_changed(GtkRange *range, gpointer data)
{
    t_app_data *app = (t_app_data *)data;
    double value = gtk_range_get_value(range);
    app->cfg.click_max_duration = value;
    
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%.1f", value);
    gtk_entry_set_text(GTK_ENTRY(app->click_entry), buffer);
}

static void on_drag_scale_changed(GtkRange *range, gpointer data)
{
    t_app_data *app = (t_app_data *)data;
    double value = gtk_range_get_value(range);
    app->cfg.drag_min_duration = value;
    
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%.1f", value);
    gtk_entry_set_text(GTK_ENTRY(app->drag_entry), buffer);
}

/* Callback para salvar configurações */
static void on_save_config(GtkWidget *widget, gpointer data)
{
    t_app_data *app = (t_app_data *)data;
    (void)widget; /* Evitar warning */
    
    /* Atualizar valores dos campos de texto */
    const char *width_text = gtk_entry_get_text(GTK_ENTRY(app->width_entry));
    const char *height_text = gtk_entry_get_text(GTK_ENTRY(app->height_entry));
    
    app->cfg.screen_width = atoi(width_text);
    app->cfg.screen_height = atoi(height_text);
    
    save_config(app->cfg);
    
    gtk_label_set_text(GTK_LABEL(app->status_label), "✓ Configurações salvas!");
    g_print("Configurações salvas com sucesso!\n");
}

/* Callback para iniciar/parar o programa Python */
static void on_toggle_program(GtkWidget *widget, gpointer data)
{
    t_app_data *app = (t_app_data *)data;
    (void)widget; /* Evitar warning */
    
    if (!app->is_running)
    {
        /* Iniciar o programa Python */
        pid_t pid = fork();
        if (pid == 0)
        {
            /* Processo filho - executar Python */
            /* Executar do diretório raiz do projeto, não mudar para vision_python */
            execlp("python3", "python3", "vision_python/socket_client.py", NULL);
            perror("execlp failed");
            exit(1);
        }
        else if (pid > 0)
        {
            app->python_pid = pid;
            app->is_running = TRUE;
            gtk_button_set_label(GTK_BUTTON(app->start_btn), "⏹ Stop Program");
            gtk_label_set_text(GTK_LABEL(app->status_label), "🟢 Hand Mouse Running");
            g_print("Programa iniciado (PID: %d)\n", pid);
        }
    }
    else
    {
        /* Parar o programa */
        if (app->python_pid > 0)
        {
            kill(app->python_pid, SIGTERM);
            app->python_pid = 0;
        }
        app->is_running = FALSE;
        gtk_button_set_label(GTK_BUTTON(app->start_btn), "▶ Start Program");
        gtk_label_set_text(GTK_LABEL(app->status_label), "⭕ Hand Mouse Stopped");
        g_print("Programa parado\n");
    }
}

/* Criar janela GTK */
int main(int argc, char **argv)
{
    gtk_init(&argc, &argv);

    t_app_data *app = g_malloc(sizeof(t_app_data));
    app->cfg = load_config();
    app->python_pid = 0;
    app->is_running = FALSE;

    /* Janela principal */
    app->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app->window), "Hand Mouse Control Panel");
    gtk_window_set_default_size(GTK_WINDOW(app->window), 600, 500);
    gtk_window_set_position(GTK_WINDOW(app->window), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(app->window), 15);

    g_signal_connect(app->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    /* Container principal */
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(app->window), vbox);

    /* === SEÇÃO DE CONTROLE === */
    GtkWidget *control_frame = gtk_frame_new("Program Control");
    gtk_box_pack_start(GTK_BOX(vbox), control_frame, FALSE, FALSE, 0);
    
    GtkWidget *control_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(control_box), 10);
    gtk_container_add(GTK_CONTAINER(control_frame), control_box);
    
    app->start_btn = gtk_button_new_with_label("▶ Start Program");
    gtk_widget_set_size_request(app->start_btn, -1, 50);
    g_signal_connect(app->start_btn, "clicked", G_CALLBACK(on_toggle_program), app);
    gtk_box_pack_start(GTK_BOX(control_box), app->start_btn, FALSE, FALSE, 0);
    
    app->status_label = gtk_label_new("⭕ Hand Mouse Stopped");
    gtk_box_pack_start(GTK_BOX(control_box), app->status_label, FALSE, FALSE, 0);

    /* === SEÇÃO DE CONFIGURAÇÕES DE TELA === */
    GtkWidget *screen_frame = gtk_frame_new("Screen Configuration");
    gtk_box_pack_start(GTK_BOX(vbox), screen_frame, FALSE, FALSE, 0);
    
    GtkWidget *screen_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(screen_grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(screen_grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(screen_grid), 10);
    gtk_container_add(GTK_CONTAINER(screen_frame), screen_grid);
    
    GtkWidget *width_label = gtk_label_new("Screen Width:");
    gtk_widget_set_halign(width_label, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(screen_grid), width_label, 0, 0, 1, 1);
    
    app->width_entry = gtk_entry_new();
    char width_str[16];
    snprintf(width_str, sizeof(width_str), "%d", app->cfg.screen_width);
    gtk_entry_set_text(GTK_ENTRY(app->width_entry), width_str);
    gtk_grid_attach(GTK_GRID(screen_grid), app->width_entry, 1, 0, 1, 1);
    
    GtkWidget *height_label = gtk_label_new("Screen Height:");
    gtk_widget_set_halign(height_label, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(screen_grid), height_label, 0, 1, 1, 1);
    
    app->height_entry = gtk_entry_new();
    char height_str[16];
    snprintf(height_str, sizeof(height_str), "%d", app->cfg.screen_height);
    gtk_entry_set_text(GTK_ENTRY(app->height_entry), height_str);
    gtk_grid_attach(GTK_GRID(screen_grid), app->height_entry, 1, 1, 1, 1);

    /* === SEÇÃO DE PARÂMETROS DE CONTROLE === */
    GtkWidget *params_frame = gtk_frame_new("Control Parameters");
    gtk_box_pack_start(GTK_BOX(vbox), params_frame, TRUE, TRUE, 0);
    
    GtkWidget *params_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(params_grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(params_grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(params_grid), 10);
    gtk_container_add(GTK_CONTAINER(params_frame), params_grid);
    
    /* Pinch Threshold */
    GtkWidget *pinch_label = gtk_label_new("Pinch Threshold:");
    gtk_widget_set_halign(pinch_label, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(params_grid), pinch_label, 0, 0, 1, 1);
    
    app->pinch_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.01, 0.15, 0.001);
    gtk_range_set_value(GTK_RANGE(app->pinch_scale), app->cfg.pinch_threshold);
    gtk_widget_set_hexpand(app->pinch_scale, TRUE);
    gtk_scale_set_draw_value(GTK_SCALE(app->pinch_scale), FALSE);
    g_signal_connect(app->pinch_scale, "value-changed", G_CALLBACK(on_pinch_scale_changed), app);
    gtk_grid_attach(GTK_GRID(params_grid), app->pinch_scale, 1, 0, 1, 1);
    
    app->pinch_entry = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(app->pinch_entry), 8);
    char pinch_str[16];
    snprintf(pinch_str, sizeof(pinch_str), "%.3f", app->cfg.pinch_threshold);
    gtk_entry_set_text(GTK_ENTRY(app->pinch_entry), pinch_str);
    gtk_grid_attach(GTK_GRID(params_grid), app->pinch_entry, 2, 0, 1, 1);
    
    /* Smoothing */
    GtkWidget *smooth_label = gtk_label_new("Smoothing:");
    gtk_widget_set_halign(smooth_label, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(params_grid), smooth_label, 0, 1, 1, 1);
    
    app->smooth_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.0, 1.0, 0.01);
    gtk_range_set_value(GTK_RANGE(app->smooth_scale), app->cfg.smoothing);
    gtk_widget_set_hexpand(app->smooth_scale, TRUE);
    gtk_scale_set_draw_value(GTK_SCALE(app->smooth_scale), FALSE);
    g_signal_connect(app->smooth_scale, "value-changed", G_CALLBACK(on_smooth_scale_changed), app);
    gtk_grid_attach(GTK_GRID(params_grid), app->smooth_scale, 1, 1, 1, 1);
    
    app->smooth_entry = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(app->smooth_entry), 8);
    char smooth_str[16];
    snprintf(smooth_str, sizeof(smooth_str), "%.2f", app->cfg.smoothing);
    gtk_entry_set_text(GTK_ENTRY(app->smooth_entry), smooth_str);
    gtk_grid_attach(GTK_GRID(params_grid), app->smooth_entry, 2, 1, 1, 1);
    
    /* Click Max Duration */
    GtkWidget *click_label = gtk_label_new("Click Max Duration:");
    gtk_widget_set_halign(click_label, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(params_grid), click_label, 0, 2, 1, 1);
    
    app->click_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.1, 2.0, 0.1);
    gtk_range_set_value(GTK_RANGE(app->click_scale), app->cfg.click_max_duration);
    gtk_widget_set_hexpand(app->click_scale, TRUE);
    gtk_scale_set_draw_value(GTK_SCALE(app->click_scale), FALSE);
    g_signal_connect(app->click_scale, "value-changed", G_CALLBACK(on_click_scale_changed), app);
    gtk_grid_attach(GTK_GRID(params_grid), app->click_scale, 1, 2, 1, 1);
    
    app->click_entry = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(app->click_entry), 8);
    char click_str[16];
    snprintf(click_str, sizeof(click_str), "%.1f", app->cfg.click_max_duration);
    gtk_entry_set_text(GTK_ENTRY(app->click_entry), click_str);
    gtk_grid_attach(GTK_GRID(params_grid), app->click_entry, 2, 2, 1, 1);
    
    /* Drag Min Duration */
    GtkWidget *drag_label = gtk_label_new("Drag Min Duration:");
    gtk_widget_set_halign(drag_label, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(params_grid), drag_label, 0, 3, 1, 1);
    
    app->drag_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.1, 3.0, 0.1);
    gtk_range_set_value(GTK_RANGE(app->drag_scale), app->cfg.drag_min_duration);
    gtk_widget_set_hexpand(app->drag_scale, TRUE);
    gtk_scale_set_draw_value(GTK_SCALE(app->drag_scale), FALSE);
    g_signal_connect(app->drag_scale, "value-changed", G_CALLBACK(on_drag_scale_changed), app);
    gtk_grid_attach(GTK_GRID(params_grid), app->drag_scale, 1, 3, 1, 1);
    
    app->drag_entry = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(app->drag_entry), 8);
    char drag_str[16];
    snprintf(drag_str, sizeof(drag_str), "%.1f", app->cfg.drag_min_duration);
    gtk_entry_set_text(GTK_ENTRY(app->drag_entry), drag_str);
    gtk_grid_attach(GTK_GRID(params_grid), app->drag_entry, 2, 3, 1, 1);

    /* Botão de salvar */
    GtkWidget *save_btn = gtk_button_new_with_label("💾 Save Configuration");
    gtk_widget_set_size_request(save_btn, -1, 40);
    g_signal_connect(save_btn, "clicked", G_CALLBACK(on_save_config), app);
    gtk_box_pack_start(GTK_BOX(vbox), save_btn, FALSE, FALSE, 0);

    gtk_widget_show_all(app->window);
    gtk_main();

    /* Cleanup */
    if (app->is_running && app->python_pid > 0)
        kill(app->python_pid, SIGTERM);
    
    g_free(app);
    return 0;
}
