#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BEGIN_ROW (LINES - num_opts) / 2

typedef struct Timer {
	int minutes;
	int seconds;
	int minbreak;
	int secbreak;
	int session;
	int status;
	char timebuf[32];
	char sessionbuf[32];
}
Timer;

static Timer *create_timer(void);
static void clear_screen(void);
static void draw_menu(int choice);
static int select_choice(int num_opts);
static void display_timer(Timer *t, int sessioncount, int min, int sec);
static void update_time(Timer *t, int *min, int *sec, int *status, int sessioncount);
static void run_timer(Timer *t);
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
	t->minbreak = 5;
	t->secbreak = 0;
	t->session = 4;
	t->status = 1;

	return t;
}

static void
clear_screen(void)
{
	clear();
	mvprintw(BEGIN_ROW - 2, (COLS - strlen(title)) / 2, "%s", title);
}

static void
draw_menu(int choice)
{
	clear_screen();
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
display_timer(Timer *t, int sessioncount, int min, int sec)
{
	clear_screen();
	if (t->status)
		mvprintw(BEGIN_ROW, (COLS - strlen("SESSION: POMODORO")) / 2, "SESSION: POMODORO");
	else
		mvprintw(BEGIN_ROW, (COLS - strlen("SESSION: PAUSE")) / 2, "SESSION: PAUSE");
	snprintf(t->timebuf, sizeof(t->timebuf), "Time: %02d:%02d", min, sec);
	mvprintw(BEGIN_ROW + 1, (COLS - strlen(t->timebuf)) / 2, "%s", t->timebuf);

	snprintf(t->sessionbuf, sizeof(t->sessionbuf), "%d/%d", sessioncount, t->session);
	mvprintw(BEGIN_ROW + 3, (COLS - strlen(t->sessionbuf)) / 2, "%s", t->sessionbuf);
	refresh();
}

static void
update_time(Timer *t, int *min, int *sec, int *status, int sessioncount)
{
	while (1) {
		display_timer(t, sessioncount, *min, *sec);
		sleep(1);
		if (*min <= 0 && *sec <= 0) {
			*status = (*status == 1) ? 0 : 1;
			return;
		}
		if (*sec == 0) {
			*sec = 60;
			(*min)--;
		}
		(*sec)--;
	}
}

static void
run_timer(Timer *t)
{
	int sessioncount = 1;
	for (int i = 0; i < t->session; i++) {
		int work_min = t->minutes;
		int work_sec = t->seconds;
		update_time(t, &work_min, &work_sec, &t->status, sessioncount);
		int break_min = t->minbreak;
		int break_sec = t->secbreak;
		update_time(t, &break_min, &break_sec, &t->status, sessioncount);
		sessioncount++;
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
		int choice = select_choice(num_opts);
		switch (choice) {
			case 0:
				if (!t)
					t = create_timer();
				run_timer(t);
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
