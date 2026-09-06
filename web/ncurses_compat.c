#include "ncurses.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <emscripten.h>

static WINDOW *stdscr;
static int do_echo;
enum { WGETSTR_BUF = 128 };
enum { RENDER_CELLS = LINES * COLS };
enum { RENDER_BYTES = RENDER_CELLS + (LINES - 1) + 1 };
static char render_out[RENDER_BYTES];
static char render_screen[RENDER_CELLS];

EM_JS(int, web_midway_read_key, (), {
	if (!Module.midwayKeyQueue || Module.midwayKeyQueue.length === 0) {
		return -1;
	}
	return Module.midwayKeyQueue.shift();
});

EM_JS(void, web_midway_render, (const char *text), {
	const target = document.getElementById('midway-screen');
	if (!target) return;
	target.textContent = UTF8ToString(text);
});

static void clamp_cursor(WINDOW *win)
{
	if (win->cur_y < 0) win->cur_y = 0;
	if (win->cur_y >= win->rows) win->cur_y = win->rows - 1;
	if (win->cur_x < 0) win->cur_x = 0;
	if (win->cur_x >= win->cols) win->cur_x = win->cols - 1;
}

static WINDOW *alloc_window(int rows, int cols, int y, int x)
{
	WINDOW *win = calloc(1, sizeof(*win));
	if (!win) return NULL;
	win->rows = rows;
	win->cols = cols;
	win->begin_y = y;
	win->begin_x = x;
	win->cells = malloc((size_t)rows * (size_t)cols);
	if (!win->cells) {
		free(win);
		return NULL;
	}
	memset(win->cells, ' ', (size_t)rows * (size_t)cols);
	return win;
}

static void blit_window(const WINDOW *win, char *screen)
{
	for (int y = 0; y < win->rows; y++) {
		for (int x = 0; x < win->cols; x++) {
			const int sy = win->begin_y + y;
			const int sx = win->begin_x + x;
			if (sy < 0 || sy >= LINES || sx < 0 || sx >= COLS) continue;
			screen[sy * COLS + sx] = win->cells[y * win->cols + x];
		}
	}
}

static int blocking_key(void)
{
	for (;;) {
		const int c = web_midway_read_key();
		if (c >= 0) return c;
		emscripten_sleep(16);
	}
}

static int append_formatted(WINDOW *win, const char *fmt, va_list ap)
{
	char buf[512];
	vsnprintf(buf, sizeof(buf), fmt, ap);
	return mvwaddstr(win, win->cur_y, win->cur_x, buf);
}

WINDOW *initscr(void)
{
	stdscr = alloc_window(LINES, COLS, 0, 0);
	return stdscr;
}

WINDOW *newwin(int nlines, int ncols, int begin_y, int begin_x)
{
	return alloc_window(nlines, ncols, begin_y, begin_x);
}

int endwin(void)
{
	return 0;
}

int noecho(void)
{
	do_echo = 0;
	return 0;
}

int echo(void)
{
	do_echo = 1;
	return 0;
}

int clear(void)
{
	if (!stdscr) return ERR;
	memset(stdscr->cells, ' ', (size_t)stdscr->rows * (size_t)stdscr->cols);
	stdscr->cur_y = 0;
	stdscr->cur_x = 0;
	return 0;
}

int refresh(void)
{
	if (!stdscr) return ERR;
	memcpy(render_screen, stdscr->cells, sizeof(render_screen));
	int p = 0;
	for (int y = 0; y < LINES; y++) {
		for (int x = 0; x < COLS; x++) {
			render_out[p++] = render_screen[y * COLS + x];
		}
		if (y < LINES - 1) render_out[p++] = '\n';
	}
	render_out[p] = '\0';
	web_midway_render(render_out);
	return 0;
}

int werase(WINDOW *win)
{
	if (!win) return ERR;
	memset(win->cells, ' ', (size_t)win->rows * (size_t)win->cols);
	win->cur_y = 0;
	win->cur_x = 0;
	return 0;
}

int wmove(WINDOW *win, int y, int x)
{
	if (!win) return ERR;
	win->cur_y = y;
	win->cur_x = x;
	clamp_cursor(win);
	return 0;
}

int wclrtoeol(WINDOW *win)
{
	if (!win) return ERR;
	for (int x = win->cur_x; x < win->cols; x++) {
		win->cells[win->cur_y * win->cols + x] = ' ';
	}
	return 0;
}

int wrefresh(WINDOW *win)
{
	if (!win || !stdscr) return ERR;
	if (win != stdscr) {
		blit_window(win, stdscr->cells);
	}
	return refresh();
}

int waddch(WINDOW *win, int ch)
{
	if (!win) return ERR;
	clamp_cursor(win);
	win->cells[win->cur_y * win->cols + win->cur_x] = (char)ch;
	if (win->cur_x < win->cols - 1) win->cur_x++;
	return 0;
}

int winch(WINDOW *win)
{
	if (!win) return ' ';
	clamp_cursor(win);
	return (unsigned char)win->cells[win->cur_y * win->cols + win->cur_x];
}

int wprintw(WINDOW *win, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int rc = append_formatted(win, fmt, ap);
	va_end(ap);
	return rc;
}

int mvwaddstr(WINDOW *win, int y, int x, const char *str)
{
	if (!win || !str) return ERR;
	wmove(win, y, x);
	for (const unsigned char *p = (const unsigned char *)str; *p; p++) {
		if (win->cur_x >= win->cols) break;
		win->cells[win->cur_y * win->cols + win->cur_x] = (char)*p;
		win->cur_x++;
	}
	return 0;
}

int mvwaddch(WINDOW *win, int y, int x, int ch)
{
	if (!win) return ERR;
	wmove(win, y, x);
	return waddch(win, ch);
}

int mvwprintw(WINDOW *win, int y, int x, const char *fmt, ...)
{
	if (!win) return ERR;
	wmove(win, y, x);
	va_list ap;
	va_start(ap, fmt);
	int rc = append_formatted(win, fmt, ap);
	va_end(ap);
	return rc;
}

int wgetstr(WINDOW *win, char *str)
{
	/* Midway passes 128-byte command buffers to wreadstr/wgetstr. */
	if (!str) return ERR;
	int idx = 0;
	for (;;) {
		const int c = blocking_key();
		if (c == '\r' || c == '\n') {
			break;
		}
		if ((c == 8 || c == 127) && idx > 0) {
			idx--;
			if (do_echo && win) {
				if (win->cur_x > 0) {
					win->cur_x--;
					waddch(win, ' ');
					win->cur_x--;
				}
				wrefresh(win);
			}
			continue;
		}
		if (c < 32 || c > 126 || idx >= WGETSTR_BUF - 1) continue;
		str[idx++] = (char)c;
		if (do_echo && win) {
			waddch(win, (char)c);
			wrefresh(win);
		}
	}
	str[idx] = '\0';
	return 0;
}

int wgetch(WINDOW *win)
{
	(void)win;
	return blocking_key();
}

int leaveok(WINDOW *win, int bf)
{
	(void)win;
	(void)bf;
	return 0;
}

int crmode(void)
{
	return 0;
}

int nocrmode(void)
{
	return 0;
}

int mvcur(int oldrow, int oldcol, int newrow, int newcol)
{
	(void)oldrow;
	(void)oldcol;
	(void)newrow;
	(void)newcol;
	return 0;
}

int move(int y, int x)
{
	if (!stdscr) return ERR;
	return wmove(stdscr, y, x);
}

int addstr(const char *str)
{
	if (!stdscr) return ERR;
	return mvwaddstr(stdscr, stdscr->cur_y, stdscr->cur_x, str);
}

int printw(const char *fmt, ...)
{
	if (!stdscr) return ERR;
	va_list ap;
	va_start(ap, fmt);
	int rc = append_formatted(stdscr, fmt, ap);
	va_end(ap);
	return rc;
}

int mvaddch(int y, int x, int ch)
{
	if (!stdscr) return ERR;
	wmove(stdscr, y, x);
	return waddch(stdscr, ch);
}

int mvaddstr(int y, int x, const char *str)
{
	if (!stdscr) return ERR;
	return mvwaddstr(stdscr, y, x, str);
}

int mvprintw(int y, int x, const char *fmt, ...)
{
	if (!stdscr) return ERR;
	wmove(stdscr, y, x);
	va_list ap;
	va_start(ap, fmt);
	int rc = append_formatted(stdscr, fmt, ap);
	va_end(ap);
	return rc;
}
