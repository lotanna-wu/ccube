#include <signal.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/select.h>

#define SLEEP_MS(ms) usleep((ms) * 1000)

// Some of these are probably unused
#define COLOR_BLACK "\033[30m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN "\033[36m"
#define COLOR_WHITE "\033[37m"
#define COLOR_BRIGHT_RED "\033[91m"
#define COLOR_BRIGHT_GREEN "\033[92m"
#define COLOR_BRIGHT_YELLOW "\033[93m"
#define COLOR_BRIGHT_BLUE "\033[94m"
#define COLOR_BRIGHT_MAGENTA "\033[95m"
#define COLOR_BRIGHT_CYAN "\033[96m"
#define COLOR_BRIGHT_WHITE "\033[97m"
#define COLOR_ORANGE "\033[38;5;208m"
#define COLOR_DEEP_PINK "\033[38;5;198m"
#define COLOR_PURPLE "\033[38;5;129m"
#define COLOR_INDIGO "\033[38;5;63m"
#define COLOR_LIME "\033[38;5;118m"
#define COLOR_VIOLET "\033[38;5;141m"
#define COLOR_RGB_ORANGE "\033[38;2;255;127;0m"

#define BOLD "\033[1m"
#define COLOR_RESET "\033[0m"
#define CLEAR "\033[0m"
#define CLEAR_SCREEN "\033[2J"
#define HIDE_CURSOR "\033[?25l"
#define CURSOR_HOME "\033[H"
#define SHOW_CURSOR "\033[?25h"
#define RESET_ATTRIBUTES "\033[m"


typedef struct Vertex {
    float pos[3];
    float original[3];
} vertex_t;

typedef struct Edge {
    vertex_t *p0;
    vertex_t *p1;
} edge_t;


void kill_cube(int sig);
void handle_resize(int sig);

struct termios orig_termios;
void check_keypress_exit(void);
void disable_input(void);
void restore_input(void);
int kbhit(void);

char *frame_buffer = NULL;
int buffer_size = 0;
int g_rows, g_cols;
void get_terminal_size(int *rows, int *cols);
void init_buffer(int rows, int cols);
void clear_buffer(void);
void buffer_write(int row, int col, int cols, int rows, char c);
void flush_buffer(int rows, int cols);
void cleanup_buffer(void);
void* _malloc(size_t size);

float cos_theta, sin_theta;
float RX_MATRIX[3][3];
float RY_MATRIX[3][3];
float RZ_MATRIX[3][3];
void set_rotation_matrices(float theta);
void rotate_vertex(vertex_t *v, float rmatrix[3][3]);
void rotate_cube(vertex_t *vertices[8]);
void project(const vertex_t *p, int *x, int *y, int size);
void draw_line(int x0, int y0, int x1, int y1, char c);
void draw_cube(edge_t *edges[12], int size, int offset_x, int offset_y, char c);
void init_termstyle(char* color, int bold, int rainbow);
void init_termscreen(void);
void hsv_to_rgb(float h, float s, float v, int *r, int *g, int *b);

void usage(void) {
    printf(" Usage: ccube -[bhnRs] [-c color] [-d delay]\n");
    printf(" -b: Set bold mode\n");
    printf(" -s: Set screensaver mode\n");
    printf(" -n: Set nested mode\n");
    printf(" -R: Set rainbow mode\n");
    printf(" -c [color]: Set cube color\n");
    printf(" -C [character]: Set the ASCII character for the cube\n");
    printf(" -d [delay]: [0-10] default 5\n");
}

int main(int argc, char *argv[]) {
    signal(SIGINT, kill_cube);
    signal(SIGTERM, kill_cube);
    signal(SIGTSTP, kill_cube);
    signal(SIGWINCH, handle_resize);
    
    int opt;
    char* color = NULL;
    int screensaver = 0;
    int bold = 0;
    int rainbow = 0;
    int nested = 0;
    int delay = 5;
    char ascii = '#';
    while((opt = getopt(argc, argv, "bhnRsc:C:d:")) != -1) {
        switch(opt){
            case 'h':
                usage();
                return 0;
            case 'b':
                bold = 1;
                break;
            case 'n':
                nested = 1;
                break;
            case 's':
                screensaver = 1;
                break;
            case 'C':
                ascii = optarg[0];
                break;
            case 'c':
                if (strcmp(optarg, "black") == 0){
                    color = COLOR_BLACK;
                } else if (strcmp(optarg, "red") == 0) {
                    color = COLOR_RED;
                } else if (strcmp(optarg, "green") == 0) {
                    color = COLOR_GREEN;
                } else if (strcmp(optarg, "yellow") == 0) {
                    color = COLOR_YELLOW;
                } else if (strcmp(optarg, "orange") == 0) {
                    color = COLOR_RGB_ORANGE;
                } else if (strcmp(optarg, "blue") == 0) {
                    color = COLOR_BLUE;
                } else if (strcmp(optarg, "magenta") == 0) {
                    color = COLOR_MAGENTA;
                } else if (strcmp(optarg, "cyan") == 0) {
                    color = COLOR_CYAN;
                } else if (strcmp(optarg, "white") == 0) {
                    color = COLOR_WHITE;
                } else if (strcmp(optarg, "bright-red") == 0) {
                    color = COLOR_BRIGHT_RED;
                } else if (strcmp(optarg, "bright-green") == 0) {
                    color = COLOR_BRIGHT_GREEN;
                } else if (strcmp(optarg, "bright-yellow") == 0) {
                    color = COLOR_BRIGHT_YELLOW;
                } else if (strcmp(optarg, "bright-blue") == 0) {
                    color = COLOR_BRIGHT_BLUE;
                } else if (strcmp(optarg, "bright-magenta") == 0) {
                    color = COLOR_BRIGHT_MAGENTA;
                } else if (strcmp(optarg, "bright-cyan") == 0) {
                    color = COLOR_BRIGHT_CYAN;
                } else if (strcmp(optarg, "bright-white") == 0) {
                    color = COLOR_BRIGHT_WHITE;
                } else if (strcmp(optarg, "pink") == 0) {
                    color = COLOR_DEEP_PINK;
                } else if (strcmp(optarg, "purple") == 0) {
                    color = COLOR_PURPLE;
                } else if (strcmp(optarg, "indigo") == 0) {
                    color = COLOR_INDIGO;
                } else if (strcmp(optarg, "lime") == 0) {
                    color = COLOR_LIME;
                } else if (strcmp(optarg, "violet") == 0) {
                    color = COLOR_VIOLET;
                } else {
                    color = COLOR_WHITE;
                }
                break;
            case 'R':
                rainbow = 1;
                break;
            case 'd':
                delay = atoi(optarg);
                if(delay <= 0 ) {
                    delay = 1;
                } else if (delay >= 10) {
                    delay = 10;
                }
                break;
            default:
                usage();
                return 1;
        }
    }

    vertex_t p0 = {{ 1, 1, 1}, { 1, 1, 1}};
    vertex_t p1 = {{-1,-1,-1}, {-1,-1,-1}};
    vertex_t p2 = {{-1,-1, 1}, {-1,-1, 1}};
    vertex_t p3 = {{-1, 1,-1}, {-1, 1,-1}};
    vertex_t p4 = {{-1, 1, 1}, {-1, 1, 1}};
    vertex_t p5 = {{ 1,-1,-1}, { 1,-1,-1}};
    vertex_t p6 = {{ 1,-1, 1}, { 1,-1, 1}};
    vertex_t p7 = {{ 1, 1,-1}, { 1, 1,-1}};

    edge_t p04 = {&p0, &p4};
    edge_t p06 = {&p0, &p6};
    edge_t p07 = {&p0, &p7};
    edge_t p12 = {&p1, &p2};
    edge_t p13 = {&p1, &p3};
    edge_t p15 = {&p1, &p5};
    edge_t p24 = {&p2, &p4};
    edge_t p26 = {&p2, &p6};
    edge_t p34 = {&p3, &p4};
    edge_t p37 = {&p3, &p7};
    edge_t p56 = {&p5, &p6};
    edge_t p57 = {&p5, &p7};

    vertex_t *vertices[8] = {
        &p0, &p1, &p2, &p3, 
        &p4, &p5, &p6, &p7
    };

    edge_t *edges[12] = {
        &p04, &p06, &p07, 
        &p12, &p13, &p15,
        &p24, &p26, &p34,
        &p37, &p56, &p57 
    };

    get_terminal_size(&g_rows, &g_cols);
    init_buffer(g_rows, g_cols);
    init_termscreen();
    init_termstyle(color, bold, rainbow);
    disable_input();

    delay += 15;
    float theta = 0;
    float hue = 0;
    while(1) {        
        set_rotation_matrices(theta);
        rotate_cube(vertices);
        clear_buffer();
        
        int size = (g_rows > g_cols) ? g_cols / 4 : g_rows / 4;
        int offset_x = g_cols / 2;
        int offset_y = g_rows / 2;
        draw_cube(edges, size, offset_x, offset_y, ascii);
        
        if(nested && size >= 7) {
            draw_cube(edges, size / 1.5, offset_x, offset_y, ascii);
        }

        flush_buffer(g_rows, g_cols);
        
        if(rainbow) {
            int r, g, b;
            hsv_to_rgb(hue, 1.0, 1.0, &r, &g, &b);
            printf("\033[38;2;%d;%d;%dm", r, g, b);
            hue = fmod(hue + 2, 360);
        }

        if(screensaver) check_keypress_exit();
        theta += 0.04;
        SLEEP_MS(delay);
    }
    
    return 0;
}

void get_terminal_size(int *rows, int *cols) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    *rows = w.ws_row;
    *cols = w.ws_col;
}

void init_buffer(int rows, int cols) {
    buffer_size = rows * cols;
    frame_buffer = (char*)_malloc(buffer_size);
}

void init_termscreen(void){
    printf(CLEAR_SCREEN HIDE_CURSOR);
    int res = system("clear");
    (void)res;
    fflush(stdout);
}

void init_termstyle(char* color, int bold, int rainbow){
    printf(CLEAR_SCREEN HIDE_CURSOR RESET_ATTRIBUTES "%s", color);
    if(bold)    printf(BOLD);
    if(rainbow) printf(COLOR_RED);
    int res = system("clear");
    (void)res;
    fflush(stdout);
}

void clear_buffer(void) {
    memset(frame_buffer, ' ', buffer_size);
}

void buffer_write(int row, int col, int cols, int rows, char c) {
    (void)rows;
    if (row >= 0 && col >= 0) {
        int idx = row * cols + col;
        if (idx < buffer_size) {
            frame_buffer[idx] = c;
        }
    }
}

void flush_buffer(int rows, int cols) {
    printf(CURSOR_HOME);
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            putchar(frame_buffer[r * cols + c]);
        }
        if (r < rows - 1) putchar('\n');
    }
    fflush(stdout);
}

void cleanup_buffer(void) {
    free(frame_buffer);
}

void kill_cube(int sig) {
    (void)sig;
    cleanup_buffer();
    printf(SHOW_CURSOR RESET_ATTRIBUTES);
    restore_input();
    int res = system("clear");
    (void)res;
    exit(0);
}

void check_keypress_exit(void) {
    if (kbhit()) {
        char c;
        int n = read(STDIN_FILENO, &c, 1);
        (void)n;
        kill_cube(0);
    }
}

void handle_resize(int sig) {
    (void)sig;
    int new_rows, new_cols;
    get_terminal_size(&new_rows, &new_cols);
    cleanup_buffer();
    g_rows = new_rows;
    g_cols = new_cols;
    init_buffer(g_rows, g_cols);
    init_termscreen();
}

void draw_line(int x0, int y0, int x1, int y1, char c) {
    int dx = x1 - x0;
    int dy = y1 - y0;
    int x_dist = abs(dx);
    int y_dist = abs(dy);

    int steps = (x_dist > y_dist) ? x_dist : y_dist;

    if (steps == 0) {
        buffer_write(y0, x0, g_cols, g_rows, c);
        return;
    }

    float x_step = (float)dx / steps;
    float y_step = (float)dy / steps;

    for(int i = 0; i <= steps; i++) {
        buffer_write((int)(y0 + i * y_step + 0.5f), 
                     (int)(x0 + i * x_step + 0.5f), g_cols, g_rows, c);
    }
}

void rotate_vertex(vertex_t *v, float rmatrix[3][3]) {
    float temp[3];
    for(int i = 0; i < 3; i++) {
        temp[i] = v->pos[0] * rmatrix[i][0] + 
                  v->pos[1] * rmatrix[i][1] + 
                  v->pos[2] * rmatrix[i][2];
    }
    v->pos[0] = temp[0];
    v->pos[1] = temp[1];
    v->pos[2] = temp[2];
}

void rotate_cube(vertex_t *vertices[8]) {
    for(int i = 0; i < 8; i++) {
        vertex_t *v = vertices[i];
        v->pos[0] = v->original[0];
        v->pos[1] = v->original[1];
        v->pos[2] = v->original[2];
        
        rotate_vertex(v, RX_MATRIX);
        rotate_vertex(v, RY_MATRIX);
        rotate_vertex(v, RZ_MATRIX);
    }
}

void draw_cube(edge_t *edges[12], int size, int offset_x, int offset_y, char c) {
    for(int i = 0; i < 12; i++) {
        edge_t *edge = edges[i];
        vertex_t *p0 = edge->p0;
        vertex_t *p1 = edge->p1;

        int x0, y0;
        int x1, y1;
        project(p0, &x0, &y0, size);
        project(p1, &x1, &y1, size);

        x0 += offset_x;
        y0 += offset_y;
        x1 += offset_x;
        y1 += offset_y;
        draw_line(x0, y0, x1, y1, c);
    }
}

void project(const vertex_t *p, int *x, int *y, int size) {
    float distance = 20;
    float factor = distance / (distance + p->pos[2]);
    *x = (int)(size * p->pos[0] * factor * 2);
    *y = (int)(size * p->pos[1] * factor);
}

void set_rotation_matrices(float theta) {
    cos_theta = cos(theta);
    sin_theta = sin(theta);
    
    RX_MATRIX[0][0] = 1; RX_MATRIX[0][1] = 0;         RX_MATRIX[0][2] = 0;
    RX_MATRIX[1][0] = 0; RX_MATRIX[1][1] = cos_theta; RX_MATRIX[1][2] = -sin_theta;
    RX_MATRIX[2][0] = 0; RX_MATRIX[2][1] = sin_theta; RX_MATRIX[2][2] = cos_theta;
    
    RY_MATRIX[0][0] = cos_theta;  RY_MATRIX[0][1] = 0; RY_MATRIX[0][2] = sin_theta;
    RY_MATRIX[1][0] = 0;          RY_MATRIX[1][1] = 1; RY_MATRIX[1][2] = 0;
    RY_MATRIX[2][0] = -sin_theta; RY_MATRIX[2][1] = 0; RY_MATRIX[2][2] = cos_theta;
    
    RZ_MATRIX[0][0] = cos_theta; RZ_MATRIX[0][1] = -sin_theta; RZ_MATRIX[0][2] = 0;
    RZ_MATRIX[1][0] = sin_theta; RZ_MATRIX[1][1] = cos_theta;  RZ_MATRIX[1][2] = 0;
    RZ_MATRIX[2][0] = 0;         RZ_MATRIX[2][1] = 0;          RZ_MATRIX[2][2] = 1;
}

void* _malloc(size_t size) {
    void* ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "Failed to allocate %zu bytes of memory\n", size);
        restore_input();
        printf(SHOW_CURSOR RESET_ATTRIBUTES);
        exit(1);
    }
    return ptr;
}

void hsv_to_rgb(float h, float s, float v, int *r, int *g, int *b) {
    float c = v * s;
    float x = c * (1 - fabs(fmod(h / 60.0, 2) - 1));
    float m = v - c;
    
    float r1, g1, b1;
    if (h < 60) { r1 = c; g1 = x; b1 = 0; }
    else if (h < 120) { r1 = x; g1 = c; b1 = 0; }
    else if (h < 180) { r1 = 0; g1 = c; b1 = x; }
    else if (h < 240) { r1 = 0; g1 = x; b1 = c; }
    else if (h < 300) { r1 = x; g1 = 0; b1 = c; }
    else { r1 = c; g1 = 0; b1 = x; }
    
    *r = (int)((r1 + m) * 255);
    *g = (int)((g1 + m) * 255);
    *b = (int)((b1 + m) * 255);
}

void disable_input(void) {
    struct termios raw;
    
    tcgetattr(STDIN_FILENO, &orig_termios);
    raw = orig_termios; 
    
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void restore_input(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

int kbhit(void) {
    struct timeval tv = {0, 0};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDOUT_FILENO, &fds, NULL, NULL, &tv) > 0;
}

