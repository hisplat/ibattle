
#include "windowmode.h"
#include <curses.h>
#include <string>
#include <vector>
#include <list>

const int kMode_Normal = 0;
const int kMode_Command = 1;

static void log(const std::string& s);
class Window {
public:
    Window(const char * title, int xx, int yy, int ww, int hh) : visible(true), x(xx), y(yy), w(ww), h(hh), mTitle(title) {
        win = newwin(h, w, y, x);
    }
    ~Window() {
        delwin(win);
    }
    void refresh() {
        wrefresh(win);
    }
    void clear() {
        for (int i = x; i < x + w; i++) {
            for (int j = y; j < y + h; j++) {
                mvprintw(j, i, " ");

                // char temp[100];
                // sprintf(temp, "%s: %d, %d", mTitle.c_str(), y, x);
                // log(temp);
            }
        }
    }
    void drawborder() {
        clear();
        mvaddch(y, x, '+'); // top-left
        mvaddch(y, x + w, '+'); // top-right
        mvaddch(y + h, x, '+'); // bottom-left
        mvaddch(y + h, x + w, '+'); // bottom-right
        mvhline(y, x + 1, '-', w - 1); // top
        mvhline(y + h, x + 1, '-', w - 1); // bottom
        mvvline(y + 1, x, '|', h - 1); // left
        mvvline(y + 1, x + w, '|', h - 1); // right
        mvprintw(y, x + 3, "%s", mTitle.c_str());
    }
    WINDOW* win;
    bool visible;
    int x;
    int y;
    int w;
    int h;
    std::string mTitle;
};

class ServerStat {
public:
    int loading;
    bool online;
    std::string mName;
};

static Window * server = NULL;
static Window * tips = NULL;
static Window * console = NULL;
static Window * input = NULL;

static std::list<std::string> console_logs;
static std::list<ServerStat> servers;
static int selected_server = 0;
static std::string input_string;
static int current_mode = 0;

static void log(const std::string& s)
{
    if (input_string.length() == 0) {
        return;
    }
    console_logs.push_back(s);
    while (console_logs.size() > 100) {
        console_logs.pop_front();
    }
}

static void create_windows()
{
    server = new Window("server", 0, 0, 120, 30);
    tips = new Window("help", 3, 3, 114, 35);
    console = new Window("console", 0, 30, 120, 10);
    input = new Window("input", 0, 40, 120, 2);

    tips->visible = false;
}

static void destroy_windows()
{
    delete server;
    delete tips;
    delete console;
    delete input;
}

static void draw_server()
{
    if (!server->visible) {
        return;
    }
    server->drawborder();

    int index = 0;
    for (auto& s: servers) {
        mvprintw(server->y + index + 1, server->x + 2, "%c %2d. [++++++....] %3d%% NET [  online ] ServerName ", (selected_server == index ? '>' : ' '), index + 1, s.loading);
        index++;
        if (index >= server->h) {
            break;
        }
    }
}

static void draw_tips()
{
    if (!tips->visible) {
        return;
    }
    tips->drawborder();
    mvprintw(tips->y + 2, tips->x + 2, "Normal Mode:");
    mvprintw(tips->y + 3, tips->x + 2, "? - Help      q - Quit    / - CommandMode   j - Down    k - Up");
    mvprintw(tips->y + 5, tips->x + 2, "Command Mode:");
    mvprintw(tips->y + 6, tips->x + 2, "help: show this window.");
}

static void draw_console()
{
    if (!console->visible) {
        return;
    }
    console->drawborder();

    int start = console_logs.size() - console->h + 1;
    if (start < 0) {
        start = 0;
    }
    int line = 1;
    for (std::list<std::string>::iterator it = console_logs.begin(); it != console_logs.end(); it++, start--) {
        if (start > 0) {
            continue;
        }
        mvprintw(console->y + line, console->x + 1, "%s", (*it).c_str());
        line++;
    }
}

static void draw_input()
{
    if (!input->visible) {
        return;
    }
    input->drawborder();
    if (current_mode == kMode_Command) {
        mvprintw(input->y + 1, input->x + 1, "> %s", input_string.c_str());
    } else {
        char s[1024] = {0};
        for (int i = 0; i < input->w - 1 && i < sizeof(s); i++) {
            s[i] = ' ';
        }
        mvprintw(input->y + 1, input->x + 1, "%s", s);
        mvprintw(input->y + 1, input->x + 1, "? - Help, q - Quit, / - Command Mode, j - Down, k - Up, t - online/offline");
        // mvhline(input->y + 1, input->x + 1, ' ', input->w - 1);
    }
}


static void handle_command()
{
    log(input_string);
    if (input_string == "help") {
        tips->visible = true;
    }
}

static bool handle_normal_key(int c)
{
    if (c == 'q') {
        return false;
    } else if (c == 27) { // ESC
        tips->visible = false;
    } else if (c == '?') {
        tips->visible = true;
    } else if (c == '/') {
        tips->visible = false;
        input_string = "";
        current_mode = kMode_Command;
    } else if (c == 'j' || c == KEY_DOWN) {
        selected_server++;
        if (selected_server >= servers.size() - 1) {
            selected_server = servers.size() - 1;
        }
    } else if (c == 'k' || c == KEY_UP) {
        selected_server--;
        if (selected_server < 0) {
            selected_server = 0;
        }
    }
    return true;
}

static bool handle_command_key(int c)
{
    if (c == 27) { // ESC
        current_mode = kMode_Normal;
        return true;
    } else if (c == 13 || c == 10) { // Enter
        handle_command();
        current_mode = kMode_Normal;
        return true;
    } else if (c == 8) { // Backtrace
        if (input_string.length() > 0) {
            input_string.erase(input_string.length() - 1);
        }
        return true;
    }
    char s[10];
    sprintf(s, "%c", c);
    input_string.append(s);

    return true;
}

static void onexit()
{
    destroy_windows();
    echo();
    nocbreak();
    // noraw();
    keypad(stdscr, false);
    // curs_set(1);
    endwin();
}

static void demo_data()
{
    ServerStat ss;
    servers.push_back(ss);
    servers.push_back(ss);
    servers.push_back(ss);
    servers.push_back(ss);
    servers.push_back(ss);
    servers.push_back(ss);
    servers.push_back(ss);
    servers.push_back(ss);
    servers.push_back(ss);
    servers.push_back(ss);
}

int windowmode()
{
    atexit(onexit);
    demo_data();
    initscr();
    noecho();
    cbreak();
    // raw();
    keypad(stdscr, true);
    // curs_set(0);
    create_windows();
    int c = 0;
    while (true) {
        clear();
        draw_server();
        draw_console();
        draw_input();
        draw_tips();
        // mvprintw(10, 10, "%d", c);
        // mvprintw(11, 10, "%d", input_string.length());
        refresh();
        c = getch();
        if (current_mode == kMode_Normal) {
            if (!handle_normal_key(c)) {
                break;
            }
        } else if (current_mode == kMode_Command) {
            if (!handle_command_key(c)) {
                break;
            }
        }
    }
    // destroy_windows();
    // endwin();
    // echo();
    return 0;
}

