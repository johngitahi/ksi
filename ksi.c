#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>

#define QUIT_KEY 'q'

// data structures to represent a single row of text in the editor
typedef struct erow {
	int size;
	char *chars;
} erow;

// a global struct to hold the editor's state
struct editorConfig {
	int cx, cy; // Cursor's column and row
	int screenrows;
	int screencols;
	int numrows;
	erow *row;
	char *filename;
} E;

// struct to hold the original terminal attributes
static struct termios original_termios;

// function to restore the terminal attributes to original state
void restore_terminal() {
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
}

// function to set the terminal to raw mode
void set_raw_mode() {
	struct termios raw_termios;

	tcgetattr(STDIN_FILENO, &original_termios);
	atexit(restore_terminal);
	raw_termios = original_termios;
	raw_termios.c_lflag &= ~(ICANON | ECHO | ISIG);
	raw_termios.c_cc[VMIN] = 1;
	raw_termios.c_cc[VTIME] = 0;
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw_termios);
}

int getWindowSize(int *rows, int *cols) {
	struct winsize ws;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
		return -1;
	} else {
		*cols = ws.ws_col;
		*rows = ws.ws_row;
		return (0);
	}
}

void editorOpen(char *filename) {
	FILE *fp = fopen(filename, "r");
	if (!fp) {
		perror("Unable to open file");
		exit(1);
	}

	char *line = NULL;
	size_t linecap = 0;
	size_t linelen;

	while ((linelen = getline(&line, &linecap, fp)) != -1) {
		// we reallocate the array to hold a new row
		E.row = realloc(E.row, sizeof(erow) * (E.numrows + 1));

		// Remove the newline character if it exists
		if (linelen > 0 && line[linelen - 1] == '\n') {
			linelen--;
		}

		E.row[E.numrows].size = linelen;
		E.row[E.numrows].chars = malloc(linelen + 1);
		memcpy(E.row[E.numrows].chars, line, linelen);
		E.row[E.numrows].chars[linelen] = '\0';
		E.numrows++;
	}
	// free the line buffer allocated by getline
	free(line);
	fclose(fp);
}


// initialize the editor
void initEditor() {
	E.cx = 0;
	E.cy = 0;
	E.numrows = 0;
	E.row = NULL;
	E.filename = NULL;

	// Get the window size at startup
	if (getWindowSize(&E.screenrows, &E.screencols) == -1) {
		perror("getWindowSize");
		exit(1);
	}
}


// function to handle keypresses
void editorProcessKeypress() {
	char c;
	// read only a single character
	read(STDIN_FILENO, &c, 1);
	if (c == '\x1b') {
		char seq[3];
		if (read(STDIN_FILENO, &seq[0], 1) != 1) return;
		if (read(STDIN_FILENO, &seq[1], 1) != 1) return;

		if (seq[0] == '[') {
			switch (seq[1]) {
			case 'A':
				if (E.cy != 0) E.cy--;
				break;
			case 'B':
				if (E.cy < E.numrows - 1) E.cy++;
				break;
			case 'C':
				if (E.cx < E.screencols - 1) E.cx++;
				break;
			case 'D':
				if (E.cx != 0) E.cx--;
				break;
			case 'H':
				E.cx = 0;
				break;
			case 'F':
				E.cx = E.screencols - 1;
				break;
			}
		}
	} else {
		if (c == QUIT_KEY) {
			exit(0);
		}
	}
}

// function to dra all the rows of the editor
void editorDrawRows() {
	int y;
	for (y = 0; y < E.screenrows - 1; y++) {
		if (y < E.numrows) {
			printf("%s", E.row[y].chars);
		} else {
			printf("~");
		}
		// clear the line to the right of the text
		printf("\x1b[K");

		// the first row will be printed at the top. the rest will be below it.
		printf("\r\n");
	}
}

void editorDrawStatusBar() {
	// invert the colors with escape sequence
	printf("\x1b[7m");
	char status[80];
	char rstatus[80];

	int len = snprintf(status, sizeof(status),
					   "File: %s | Lines: %d",
					   E.filename ? E.filename : "[No Name]", E.numrows
		);
	int rlen = snprintf(rstatus, sizeof(rstatus),
						"%d/%d", E.cy + 1, E.numrows);

	if (len > E.screencols) len = E.screencols;
	printf("%s", status);

	while (len < E.screencols) {
		if (E.screencols - len == rlen) {
			printf("%s", rstatus);
			break;
		} else {
			printf(" ");
			len++;
		}
	}
	printf("\x1b[m");
	printf("\r\n");
}

// function to refresh the screen
void editorRefreshScreen() {
	printf("\x1b[?251"); // hides cursor
	printf("\x1b[H");
	printf("\x1b[2J");
	
	editorDrawRows();
	editorDrawStatusBar();

	// position cursor at top-left corner again after drawing
	printf("\x1b[%d;%dH", E.cy + 1, E.cx + 1);

	printf("\x1b[?25h"); // shows cursor
	fflush(stdout); // manually flush stdout
}

int main(int argc, char *argv[]) {
	initEditor();

	if (argc >= 2) {
		editorOpen(argv[1]);
	}
	
	set_raw_mode();

	while (true) {
		editorRefreshScreen();
		editorProcessKeypress();
	}
    
	return (0);	
}
