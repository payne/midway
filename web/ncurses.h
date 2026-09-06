#ifndef MIDWAY_WEB_NCURSES_H
#define MIDWAY_WEB_NCURSES_H

#include <stdarg.h>

#define COLS 80
#define LINES 24
#define ERR (-1)
#define TRUE 1
#define FALSE 0

typedef struct WINDOW {
	int rows;
	int cols;
	int begin_y;
	int begin_x;
	int cur_y;
	int cur_x;
	char *cells;
} WINDOW;

WINDOW *initscr(void);
WINDOW *newwin(int nlines, int ncols, int begin_y, int begin_x);
int endwin(void);
int noecho(void);
int echo(void);
int clear(void);
int refresh(void);
int werase(WINDOW *win);
int wmove(WINDOW *win, int y, int x);
int wclrtoeol(WINDOW *win);
int wrefresh(WINDOW *win);
int waddch(WINDOW *win, const char ch);
char winch(WINDOW *win);
int wprintw(WINDOW *win, const char *fmt, ...);
int mvwaddstr(WINDOW *win, int y, int x, const char *str);
int mvwaddch(WINDOW *win, int y, int x, const char ch);
int mvwprintw(WINDOW *win, int y, int x, const char *fmt, ...);
int wgetstr(WINDOW *win, char *str);
int wgetch(WINDOW *win);
int leaveok(WINDOW *win, int bf);
int crmode(void);
int nocrmode(void);
int mvcur(int oldrow, int oldcol, int newrow, int newcol);

int move(int y, int x);
int addstr(const char *str);
int printw(const char *fmt, ...);
int mvaddch(int y, int x, const char ch);
int mvaddstr(int y, int x, const char *str);
int mvprintw(int y, int x, const char *fmt, ...);

#endif
