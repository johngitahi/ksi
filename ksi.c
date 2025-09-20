#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

// data structures to represent a single row of text in the editor
typedef struct erow {
	int size;
	char *chars;
} erow;

// a global struct to hold the editor's state
struct editorConfig {
	int screenrows;
	int screencols;
	int numrows;
	erow *row;
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
	E.numrows = 0;
	E.row = NULL;
}


// function to handle keypresses
void editorProcessKeypress() {
	char c;
	// read only a single character
	read(STDIN_FILENO, &c, 1);
	if (c == 'q') {
		exit (0);
	}
}

// function to dra all the rows of the editor
void editorDrawRows() {
	int y;
	for (y = 0; y < E.screenrows; y++) {
		if (y < E.numrows) {
			printf("%s", E.row[y].chars);
		}
		// clear the line to the right of the text
		printf("\x1b[K");

		// the first row will be printed at the top. the rest will be below it.
		printf("\r\n");
	}
}

// function to refresh the screen
void editorRefreshScreen() {
	printf("\x1b[2J");
	printf("\x1b[H");
	
	editorDrawRows();
	
	printf("\x1b[H");
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
