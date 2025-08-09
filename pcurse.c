#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BEGIN_ROW (LINES - num_opts) / 2

typedef struct Timer {
	int minutes;
	int seconds;
	int session;
	int status;
}
Timer;

static Timer *create_timer(void);
static void draw_menu(int choice);
static int select_choice(int num_opts);
static void run(Timer *t);
static void show_info(void);
// static void set_time(Timer *t);
static void setup(void);

static const char title[] = "== POMODORO TIMER ==";
static const char *opts[] = {
	"Start Session.", "Set Timer.", "About pomodoro.", "Exit."
};
static const int num_opts = sizeof(opts) / sizeof(opts[0]);

static Timer
*create_timer(void)
{
	Timer *t = malloc(sizeof(Timer));
	if (!t)
		return NULL;
	t->minutes = 25;
	t->seconds = 0;
	t->session = 4;
	t->status = 1;

	return t;
}

static void
draw_menu(int choice)
{
	mvprintw(BEGIN_ROW - 2, (COLS - strlen(title)) / 2, "%s", title);
	for (int i = 0; i < num_opts; i++) {
		if (i == choice) {
			mvprintw(BEGIN_ROW + i, ((COLS - strlen(opts[i])) / 2) - 2, "> %s", opts[i]);
		} else {
			mvprintw(BEGIN_ROW + i, ((COLS - strlen(opts[i])) / 2) - 2, "  %s", opts[i]);
		}
	}
}

static int
select_choice(int num_opts)
{
	int ch, choice = 0;
	while (1) {
		draw_menu(choice);
		ch = getch();

		switch (ch) {
			case KEY_UP:
				choice = (choice - 1 + num_opts) % num_opts;
				break;
			case KEY_DOWN:
				choice = (choice+1) % num_opts;
				break;
			case '\n': 
				return choice;
			case 'q':
				return -1;
			
		}
	}
}

static void
run(Timer *t)
{
	clear();
	int session_count = 1;
	char time[32];
	char session[32];
	while (session_count != t->session + 1) {
		if (t->seconds < 0) {
			t->seconds = 59;
			t->minutes--;
			if (t->minutes < 0) {
				t->status = 0;
			}
		}
		if (t->status) {
			mvprintw(BEGIN_ROW - 2, (COLS - strlen(title)) / 2, "%s", title);
			snprintf(time, sizeof(time), "Time: %02d:%02d", t->minutes, t->seconds);
			mvprintw(BEGIN_ROW, (COLS - strlen("SESSION: POMODORO")) / 2, "SESSION: POMODORO");
			mvprintw(BEGIN_ROW + 1, (COLS - strlen(time)) / 2, "%s", time);
		}

		snprintf(session, sizeof(session), "%d/%d", session_count, t->session);
		mvprintw(BEGIN_ROW + 3, (COLS - strlen(session)) / 2, "%s", session);
		t->seconds--;
		refresh();
		sleep(1);
	}
}

// static void
// set_time(Timer *t)
// {
// }

static void
show_info(void)
{
	clear();
	const char *info[] = {
		"== ABOUT THE POMODORO ==",
		"",
		"The Pomodoro Technique is a simple method to improve",
		"focus and productivity.",

		"- Work for 25 minutes (1 Pomodoro)",
		"- Take a short 5-minute break",
		"- Every 4 cycles, take a long 15-minute break",
		"",
		"This helps keep your mind fresh and focused.",
		"",
		"Press any key to return to the menu.",
	};
	int num_lines = sizeof(info) / sizeof(info[0]);
	for (int i = 0; i < num_lines; i++) {
		mvprintw(((LINES - num_lines) / 2) + i, (COLS - strlen(info[i])) / 2, "%s", info[i]);
	}
	refresh();
	getch();
}

static void
setup(void)
{
	Timer *t = NULL;
	int running = 1;

	while (running) {
		clear();
		int choice = select_choice(num_opts);
		switch (choice) {
			case 0:
				if (!t)
					t = create_timer();
				run(t);
				break;
			case 1:
				// if (!t)
				// 	t = create_timer();
				// set_time(t);
				break;
			case 2:
				show_info();
				break;
			case 3:
			case -1:
				running = 0;
				break;
		}
	}
	if (t)
		free(t);
}

int
main()
{
	initscr();
    noecho();
    cbreak();
	curs_set(FALSE);
    keypad(stdscr, TRUE);
	start_color();
	setup();
    endwin();
    return EXIT_SUCCESS;
}
