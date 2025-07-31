#include <ncurses.h>
#include <unistd.h>

static WINDOW *create_newwin(int height, int width, int starty, int startx);
static void destroy_win(WINDOW *local_win);
static void draw_digit(WINDOW *win, int y, int x, int digit, int color_on, int color_off);
static void draw_pomo_timer(WINDOW *win, int min_dec, int min, int sec_dec, int sec);

static const int digits[11][5][3] = {
                        /* digits */
    { {1, 1, 1}, {1, 0, 1}, {1, 0, 1}, {1, 0, 1}, {1, 1, 1}, },
    { {0, 1, 0}, {1, 1, 0}, {0, 1, 0}, {0, 1, 0}, {1, 1, 1}, },
    { {1, 1, 1}, {0, 0, 1}, {1, 1, 1}, {1, 0, 0}, {1, 1, 1}, },
    { {1, 1, 1}, {0, 0, 1}, {0, 1, 1}, {0, 0, 1}, {1, 1, 1}, },
    { {1, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 0, 1}, {0, 0, 1}, },
    { {1, 1, 1}, {1, 0, 0}, {1, 1, 1}, {0, 0, 1}, {1, 1, 1}, },
    { {1, 1, 1}, {1, 0, 0}, {1, 1, 1}, {1, 0, 1}, {1, 1, 1}, },
    { {1, 1, 1}, {0, 0, 1}, {0, 1, 0}, {1, 0, 0}, {1, 0, 0}, },
    { {1, 1, 1}, {1, 0, 1}, {1, 1, 1}, {1, 0, 1}, {1, 1, 1}, },
    { {1, 1, 1}, {1, 0, 1}, {1, 1, 1}, {0, 0, 1}, {1, 1, 1}, },
    { {0, 0, 1}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 1}, },
};
static const int height = 12;
static const int width = 45;
static const int wheight = (height - 2) / 3;
static const int wwidth = (width / 4);

static WINDOW
*create_newwin(int height, int width, int starty, int startx)
{
    WINDOW *local_win;
    local_win = newwin(height, width, starty, startx);
    box(local_win, 0, 0);
    wrefresh(local_win);
    return local_win;
}

static void
destroy_win(WINDOW *local_win)
{
    wborder(local_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    wrefresh(local_win);
    delwin(local_win);
}

static void
draw_digit(WINDOW *win, int y, int x, int digit, int color_on, int color_off) {
    for (int i = 0; i < 5; i++) {
        for (int j  = 0; j < 3; j++) {
            int pixel = digits[digit][i][j];
            int pair = pixel ? color_on : color_off;
            wattron(win, COLOR_PAIR(pair));
            mvwaddch(win, y + i, x + j, ' ');
            wattroff(win, COLOR_PAIR(pair));
        }
    }
}

static void
draw_pomo_timer(WINDOW *win, int min_dec, int min, int sec_dec, int sec)
{
    while (1) {
        draw_digit(win, wheight, wwidth + 2, min_dec, 1, 0);
        draw_digit(win, wheight, wwidth + 6, min, 1, 0);
        draw_digit(win, wheight, wwidth + 9, 10, 1, 0); /* draw colon */
        draw_digit(win, wheight, wwidth + 14, sec_dec, 1, 0);
        draw_digit(win, wheight, wwidth + 18, sec, 1, 0);
        wrefresh(win);
        sec--;
        if (sec < 0) {
            sec_dec--;
            if (sec_dec < 0) {
                min--;
                if (min < 0) {
                    min_dec--;
                    if (min_dec < 0) {
                        break;
                    }
                    min = 9;
                }
                sec_dec = 5;
            }
            sec = 9;
        }
        sleep(1);
    }

}

int
main()
{

    int min_dec = 2, min = 5, sec_dec = 0, sec = 0;
    int pause_min_dec = 0, pause_min = 5, pause_sec_dec = 0, pause_sec = 0;

    int session = 1;
    WINDOW *win = NULL;

    initscr();
    noecho();
    curs_set(0);
    cbreak();
    keypad(stdscr, TRUE);
    start_color();
    nodelay(win, 1);
    init_pair(1, COLOR_BLACK, COLOR_GREEN);
    init_pair(0, COLOR_BLACK, COLOR_BLACK);
    int starty = (LINES - height) / 2;
    int startx = (COLS - width) / 2;

    refresh();
    win = create_newwin(height, width, starty, startx);

    while (session < 5) {
        wattron(win, COLOR_PAIR(1));
        mvwprintw(win, height - 3, (width - 10) / 2, "Session: %d", session);
        wattroff(win, COLOR_PAIR(1));

        wattron(win, COLOR_PAIR(1));
        mvwprintw(win, 0, (width - 4) / 2, "FOCUS");
        wattroff(win, COLOR_PAIR(1));
        draw_pomo_timer(win, min_dec, min, sec_dec, sec);

        wattron(win, COLOR_PAIR(1));
        mvwprintw(win, 0, (width - 4) / 2, "BREAK");
        wattroff(win, COLOR_PAIR(1));
        draw_pomo_timer(win, pause_min_dec, pause_min, pause_sec_dec, pause_sec);
        session++;
    }
    destroy_win(win);
    endwin();
    return 0;
}
