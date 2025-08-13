#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BEGIN_ROW (LINES - numopts) / 2
#define CENTER_COL(str) (COLS - strlen(str)) / 2

typedef struct Timer {
	int focusmin;
	int focussec;
	int pausemin;
	int pausesec;
	int sessions;
	int state;
	char timebuf[32];
	char sessionbuf[32];
}
Timer;

static void initpairs(void);
static Timer *createtimer(void);
static void clearscreen(void);
static void drawmenu(int choice);
static void drawbar(int totalsec, int remainsec);
static int selchoice(int numopts);
static void drawtimer(Timer *t, int sesscount, int min, int sec);
static void updtimer(Timer *t, int *min, int *sec, int *status, int sesscount);
static void startpomo(Timer *t);
static void settime(int *focus, int *pause, int *sessions);
static void showinfo(void);
static void runmenu(Timer **t);
static void setup(void);
static void handleopts(int argc, char *argv[], Timer *t);
static void cleanup(Timer *t);
static void help(void);

static const char title[] = "== POMODORO TIMER ==";
static const char *opts[] = {
	"Start Session.", "Set Timer.", "About pomodoro.", "Exit."
};
static const int numopts = sizeof(opts) / sizeof(opts[0]);

static void
initpairs(void)
{
	start_color();
	init_pair(1, COLOR_GREEN, COLOR_BLACK);
	init_pair(2, COLOR_BLUE, COLOR_BLACK);
	init_pair(3, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(4, COLOR_YELLOW, COLOR_BLACK);
	init_pair(5, COLOR_CYAN, COLOR_BLACK);
}

static Timer
*createtimer(void)
{
	Timer *t = malloc(sizeof(Timer));
	if (!t)
		return NULL;
	t->focusmin = 25;
	t->focussec = 0;
	t->pausemin = 5;
	t->pausesec = 0;
	t->sessions = 4;
	t->state = 1;

	return t;
}

static void
clearscreen(void)
{
	clear();
	attron(COLOR_PAIR(4));
	mvprintw(BEGIN_ROW - 2, CENTER_COL(title), "%s", title);
	attroff(COLOR_PAIR(4));
}

static void
drawmenu(int choice)
{
	clearscreen();
	for (int i = 0; i < numopts; i++) {
		if (i == choice) {
			attron(COLOR_PAIR(5));
			mvprintw(BEGIN_ROW + i, CENTER_COL(opts[i]) - 2, "> %s", opts[i]);
			attroff(COLOR_PAIR(5));
		} else {
			mvprintw(BEGIN_ROW + i, CENTER_COL(opts[i]) - 2, "  %s", opts[i]);
		}
	}
}

static void
drawbar(int totalsec, int remainsec)
{
	int barw = 30;
	int filled = (remainsec * barw) / totalsec;
	int centerbar = (COLS - barw) / 2;

	mvprintw(BEGIN_ROW + 5, centerbar, "[");

	for (int i = 0; i < barw; i++) {
		if (i < filled) {
			attron(COLOR_PAIR(4));
			mvprintw(BEGIN_ROW + 5, centerbar + 1 + i, "=");
			attroff(COLOR_PAIR(4));
		}
			
		else
			mvprintw(BEGIN_ROW + 5, centerbar + 1 + i, " ");
		mvprintw(BEGIN_ROW + 5, centerbar + 1 + barw, "]");
	}
}

static int
selchoice(int numopts)
{
	int ch, choice = 0;

	while (1) {
		drawmenu(choice);
		ch = getch();

		switch (ch) {
			case KEY_UP:
				choice = (choice - 1 + numopts) % numopts;
				break;
			case KEY_DOWN:
				choice = (choice+1) % numopts;
				break;
			case '\n': 
				return choice;
			case 'q':
				return -1;
			
		}
	}
}

static void
drawtimer(Timer *t, int sesscount, int min, int sec)
{
	clearscreen();

	int remainsec = min * 60 + sec;
	int totalsec = (t->state ? t->focusmin * 60 + t->focussec : t->pausemin * 60 + t->pausesec);

	drawbar(totalsec, remainsec);

	if (t->state) {
		attron(COLOR_PAIR(1));
		mvprintw(BEGIN_ROW, CENTER_COL("FOCUS"), "FOCUS");
		attroff(COLOR_PAIR(1));
	}
	else {
		attron(COLOR_PAIR(2));
		mvprintw(BEGIN_ROW, CENTER_COL("BREAK"), "BREAK");
		attroff(COLOR_PAIR(2));
	}

	snprintf(t->timebuf, sizeof(t->timebuf), "Time: %02d:%02d", min, sec);
	mvprintw(BEGIN_ROW + 1, CENTER_COL(t->timebuf), "%s", t->timebuf);

	snprintf(t->sessionbuf, sizeof(t->sessionbuf), "%d/%d", sesscount, t->sessions);
	attron(COLOR_PAIR(3));
	mvprintw(BEGIN_ROW + 3, CENTER_COL(t->sessionbuf), "%s", t->sessionbuf);
	attroff(COLOR_PAIR(3));
	refresh();
}

static void
updtimer(Timer *t, int *min, int *sec, int *status, int sesscount)
{
	while (1) {
		drawtimer(t, sesscount, *min, *sec);
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
startpomo(Timer *t)
{
	int sesscount = 1;

	for (int i = 0; i < t->sessions; i++) {
		int focusmin = t->focusmin;
		int focussec = t->focussec;
		updtimer(t, &focusmin, &focussec, &t->state, sesscount);
		int pausemin = t->pausemin;
		int pausesec = t->pausesec;
		updtimer(t, &pausemin, &pausesec, &t->state, sesscount);
		sesscount++;
	}
}

static void
settime(int *focus, int *pause, int *sessions)
{
	clearscreen();

	WINDOW *win;
	int height = 10, width = 40;
	int starty = (LINES - height) / 2, startx = (COLS - width) / 2;
	char input[3];
	int wrows, wcols;

	win = newwin(height, width, starty + 2, startx);
	getmaxyx(win, wrows, wcols);
	box(win, 0, 0);
	keypad(win, TRUE);
	echo();
	curs_set(TRUE);

	mvwprintw(win, 1, (wcols - strlen("Set the timer: (minutes):")) / 2, "Set the timer (minutes):");
	wattron(win, A_BOLD);
	wattron(win, COLOR_PAIR(1));
	mvwprintw(win, 3, 2, "Focus: ");
	wattroff(win, A_BOLD);
	wattroff(win, COLOR_PAIR(1));
	wgetnstr(win, input, 2);
	*focus = atoi(input);

	wattron(win, A_BOLD);
	wattron(win, COLOR_PAIR(2));
	mvwprintw(win, 4, 2, "Break: ");
	wattroff(win, A_BOLD);
	wattroff(win, COLOR_PAIR(2));
	wgetnstr(win, input, 2);
	*pause = atoi(input);

	wattron(win, A_BOLD);
	wattron(win, COLOR_PAIR(3));
	mvwprintw(win, 5, 2, "Sessions: ");
	wattroff(win, A_BOLD);
	wattroff(win, COLOR_PAIR(3));
	wgetnstr(win, input, 2);
	*sessions = atoi(input);

	noecho();
	delwin(win);
	curs_set(FALSE);
}

static void
showinfo(void)
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
	int numlines = sizeof(info) / sizeof(info[0]);

	for (int i = 0; i < numlines; i++) {
		mvprintw(((LINES - numlines) / 2) + i, CENTER_COL(info[i]), "%s", info[i]);
	}
	refresh();
	getch();
}

static void
runmenu(Timer **t)
{
	setup();
	int running = 1;

	while (running) {
		int choice = selchoice(numopts);
		switch (choice) {
			case 0:
				if (!*t)
					*t = createtimer();
				startpomo(*t);
				break;
			case 1:
				if (!*t)
					*t = createtimer();
				settime(&(*t)->focusmin, &(*t)->pausemin, &(*t)->sessions);
				break;
			case 2:
				showinfo();
				break;
			case 3:
			case -1:
				running = 0;
				break;
		}
	}
}

static void
setup(void)
{
	initscr();
    noecho();
    cbreak();
	curs_set(FALSE);
    keypad(stdscr, TRUE);
	initpairs();
}

static void
handleopts(int argc, char *argv[], Timer *t)
{
	int opt;

	while ((opt = getopt(argc, argv, "f:p:s:h")) != -1) {
		switch (opt) {
			case 'f':
				t->focusmin = atoi(optarg);
				break;
			case 'p':
				t->pausemin = atoi(optarg);
				break;
			case 's':
				t->sessions = atoi(optarg);
				break;
			case 'h':
				help();
				exit(EXIT_SUCCESS);
			default:
				fprintf(stderr, "Usage: %s [-f focus] [-p pause] [-s sessions] [-h help]\n", argv[0]);
				exit(EXIT_FAILURE);
		}
	}
}

static void
cleanup(Timer *t)
{
	if (t) {
		free(t);
		t = NULL;
	}
	endwin();
}

static void
help(void)
{
    printf("Pomodoro Timer - Usage:\n");
    printf("  -f <minutes>   Set focus time (default: 25)\n");
    printf("  -p <minutes>   Set break time (default: 5)\n");
    printf("  -s <sessions>  Set number of sessions (default: 4)\n");
    printf("  -h             Show this help message\n");
}

int
main(int argc, char *argv[])
{
	Timer *t = NULL;
	if (argc == 1)
		runmenu(&t); /* run a interactive menu */
	else {
		t = createtimer();
		handleopts(argc, argv, t);
		setup();
		startpomo(t); /*start the timer immediately */
	}
	cleanup(t);
    return EXIT_SUCCESS;
}
