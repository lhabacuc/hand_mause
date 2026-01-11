#include ".libsf/libsf.h"
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
    Tool *window;
    Tool *fixed;
    Tool *start_btn;
    Tool *status_label;
    
    /* Entry widgets para configuração */
    Edtext *width_entry;
    Edtext *height_entry;
    Edtext *pinch_entry;
    Edtext *smooth_entry;
    Edtext *click_entry;
    Edtext *drag_entry;
    
    /* Sliders */
    Tool *pinch_scale;
    Tool *smooth_scale;
    Tool *click_scale;
    Tool *drag_scale;
    
    pid_t python_pid;
    int is_running;
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
        
        char key[64], value[64];
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

/* Callbacks para sliders */
static void on_pinch_changed(GtkRange *range, gpointer data)
{
    t_app_data *app = (t_app_data *)data;
    double value = gtk_range_get_value(range);
    app->cfg.pinch_threshold = value;
    
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%.3f", value);
    gtk_entry_set_text(GTK_ENTRY(app->pinch_entry->Ob), buffer);
}

static void on_smooth_changed(GtkRange *range, gpointer data)
{
    t_app_data *app = (t_app_data *)data;
    double value = gtk_range_get_value(range);
    app->cfg.smoothing = value;
    
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%.2f", value);
    gtk_entry_set_text(GTK_ENTRY(app->smooth_entry->Ob), buffer);
}

static void on_click_changed(GtkRange *range, gpointer data)
{
    t_app_data *app = (t_app_data *)data;
    double value = gtk_range_get_value(range);
    app->cfg.click_max_duration = value;
    
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%.1f", value);
    gtk_entry_set_text(GTK_ENTRY(app->click_entry->Ob), buffer);
}

static void on_drag_changed(GtkRange *range, gpointer data)
{
    t_app_data *app = (t_app_data *)data;
    double value = gtk_range_get_value(range);
    app->cfg.drag_min_duration = value;
    
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%.1f", value);
    gtk_entry_set_text(GTK_ENTRY(app->drag_entry->Ob), buffer);
}

/* Callback para salvar */
static void on_save_config(Tool *widget, gpointer data)
{
    (void)widget;
    t_app_data *app = (t_app_data *)data;
    
    const char *width_text = gtk_entry_get_text(GTK_ENTRY(app->width_entry->Ob));
    const char *height_text = gtk_entry_get_text(GTK_ENTRY(app->height_entry->Ob));
    
    app->cfg.screen_width = atoi(width_text);
    app->cfg.screen_height = atoi(height_text);
    
    save_config(app->cfg);
    
    gtk_label_set_text(GTK_LABEL(app->status_label), "✓ Configurações salvas!");
    g_print("Configurações salvas!\n");
}

/* Callback para start/stop */
static void on_toggle_program(Tool *widget, gpointer data)
{
    (void)widget;
    t_app_data *app = (t_app_data *)data;
    
    if (!app->is_running)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            execlp("python3", "python3", "vision_python/socket_client.py", NULL);
            perror("execlp failed");
            exit(1);
        }
        else if (pid > 0)
        {
            app->python_pid = pid;
            app->is_running = 1;
            gtk_button_set_label(GTK_BUTTON(app->start_btn), "⏹ Stop Program");
            gtk_label_set_text(GTK_LABEL(app->status_label), "🟢 Hand Mouse Running");
            g_print("Programa iniciado (PID: %d)\n", pid);
        }
    }
    else
    {
        if (app->python_pid > 0)
        {
            kill(app->python_pid, SIGTERM);
            app->python_pid = 0;
        }
        app->is_running = 0;
        gtk_button_set_label(GTK_BUTTON(app->start_btn), "▶ Start Program");
        gtk_label_set_text(GTK_LABEL(app->status_label), "⭕ Hand Mouse Stopped");
        g_print("Programa parado\n");
    }
}

/* Criar interface */
int main(int argc, char **argv)
{
    sf_init(argc, argv);
    
    t_app_data *app = g_malloc(sizeof(t_app_data));
    app->cfg = load_config();
    app->python_pid = 0;
    app->is_running = 0;
    
    /* Criar janela */
    app->window = create_window_s("Hand Mouse Control Panel", 700, 600);
    app->fixed = add_fixed_layout_to_window(app->window);
    
    /* === TÍTULO === */
    Tool *title = create_quick_label("HAND MOUSE CONTROLLER");
    gtk_fixed_put(GTK_FIXED(app->fixed), title, 200, 20);
    set_size(title, 300, 30);
    
    /* === BOTÃO START/STOP === */
    app->start_btn = create_button_with_position(app->fixed, "▶ Start Program",
                                                  150, 70, on_toggle_program, app);
    set_size(app->start_btn, 400, 50);
    
    /* Status label */
    app->status_label = create_label_with_position(app->fixed, 
                                                    "⭕ Hand Mouse Stopped",
                                                    200, 130, NULL);
    set_size(app->status_label, 300, 25);
    
    /* === CONFIGURAÇÕES DE TELA === */
    Tool *screen_title = create_quick_label("Screen Configuration");
    gtk_fixed_put(GTK_FIXED(app->fixed), screen_title, 50, 180);
    
    Tool *width_label = create_quick_label("Width:");
    gtk_fixed_put(GTK_FIXED(app->fixed), width_label, 50, 210);
    
    char width_str[16];
    snprintf(width_str, sizeof(width_str), "%d", app->cfg.screen_width);
    app->width_entry = create_entry1(GTK_FIXED(app->fixed), 150, 210, NULL, NULL);
    gtk_entry_set_text(GTK_ENTRY(app->width_entry->Ob), width_str);
    set_size(app->width_entry->Ob, 150, 30);
    
    Tool *height_label = create_quick_label("Height:");
    gtk_fixed_put(GTK_FIXED(app->fixed), height_label, 350, 210);
    
    char height_str[16];
    snprintf(height_str, sizeof(height_str), "%d", app->cfg.screen_height);
    app->height_entry = create_entry1(GTK_FIXED(app->fixed), 450, 210, NULL, NULL);
    gtk_entry_set_text(GTK_ENTRY(app->height_entry->Ob), height_str);
    set_size(app->height_entry->Ob, 150, 30);
    
    /* === PARÂMETROS DE CONTROLE === */
    Tool *params_title = create_quick_label("Control Parameters");
    gtk_fixed_put(GTK_FIXED(app->fixed), params_title, 50, 260);
    
    /* Pinch Threshold */
    Tool *pinch_label = create_quick_label("Pinch Threshold:");
    gtk_fixed_put(GTK_FIXED(app->fixed), pinch_label, 50, 300);
    
    app->pinch_scale = create_scale_with_position(app->fixed, 200, 300, 10, 150, 1);
    gtk_range_set_value(GTK_RANGE(app->pinch_scale), app->cfg.pinch_threshold * 1000);
    conectar(app->pinch_scale, "value-changed", on_pinch_changed, app);
    set_size(app->pinch_scale, 300, 30);
    
    char pinch_str[16];
    snprintf(pinch_str, sizeof(pinch_str), "%.3f", app->cfg.pinch_threshold);
    app->pinch_entry = create_entry1(GTK_FIXED(app->fixed), 520, 300, NULL, NULL);
    gtk_entry_set_text(GTK_ENTRY(app->pinch_entry->Ob), pinch_str);
    set_size(app->pinch_entry->Ob, 80, 30);
    
    /* Smoothing */
    Tool *smooth_label = create_quick_label("Smoothing:");
    gtk_fixed_put(GTK_FIXED(app->fixed), smooth_label, 50, 350);
    
    app->smooth_scale = create_scale_with_position(app->fixed, 200, 350, 0, 100, 1);
    gtk_range_set_value(GTK_RANGE(app->smooth_scale), app->cfg.smoothing * 100);
    conectar(app->smooth_scale, "value-changed", on_smooth_changed, app);
    set_size(app->smooth_scale, 300, 30);
    
    char smooth_str[16];
    snprintf(smooth_str, sizeof(smooth_str), "%.2f", app->cfg.smoothing);
    app->smooth_entry = create_entry1(GTK_FIXED(app->fixed), 520, 350, NULL, NULL);
    gtk_entry_set_text(GTK_ENTRY(app->smooth_entry->Ob), smooth_str);
    set_size(app->smooth_entry->Ob, 80, 30);
    
    /* Click Max Duration */
    Tool *click_label = create_quick_label("Click Max Duration:");
    gtk_fixed_put(GTK_FIXED(app->fixed), click_label, 50, 400);
    
    app->click_scale = create_scale_with_position(app->fixed, 200, 400, 10, 200, 10);
    gtk_range_set_value(GTK_RANGE(app->click_scale), app->cfg.click_max_duration * 100);
    conectar(app->click_scale, "value-changed", on_click_changed, app);
    set_size(app->click_scale, 300, 30);
    
    char click_str[16];
    snprintf(click_str, sizeof(click_str), "%.1f", app->cfg.click_max_duration);
    app->click_entry = create_entry1(GTK_FIXED(app->fixed), 520, 400, NULL, NULL);
    gtk_entry_set_text(GTK_ENTRY(app->click_entry->Ob), click_str);
    set_size(app->click_entry->Ob, 80, 30);
    
    /* Drag Min Duration */
    Tool *drag_label = create_quick_label("Drag Min Duration:");
    gtk_fixed_put(GTK_FIXED(app->fixed), drag_label, 50, 450);
    
    app->drag_scale = create_scale_with_position(app->fixed, 200, 450, 10, 300, 10);
    gtk_range_set_value(GTK_RANGE(app->drag_scale), app->cfg.drag_min_duration * 100);
    conectar(app->drag_scale, "value-changed", on_drag_changed, app);
    set_size(app->drag_scale, 300, 30);
    
    char drag_str[16];
    snprintf(drag_str, sizeof(drag_str), "%.1f", app->cfg.drag_min_duration);
    app->drag_entry = create_entry1(GTK_FIXED(app->fixed), 520, 450, NULL, NULL);
    gtk_entry_set_text(GTK_ENTRY(app->drag_entry->Ob), drag_str);
    set_size(app->drag_entry->Ob, 80, 30);
    
    /* Botão Salvar */
    Tool *save_btn = create_button_with_position(app->fixed, "💾 Save Configuration",
                                                  200, 520, on_save_config, app);
    set_size(save_btn, 300, 40);
    
    /* Mostrar tudo */
    gtk_widget_show_all(app->window);
    gtk_main();
    
    /* Cleanup */
    if (app->is_running && app->python_pid > 0)
        kill(app->python_pid, SIGTERM);
    
    g_free(app);
    return 0;
}
