/** includes **/
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>

/* defines */
// ANDs a character with bitwise 00011111
// converts the first 3 bits to zero which is similar to what CTRL KEY does
#define CTRL_KEY(k) ((k) & 0x1f)

/** data **/
// struct for storing current terminal's original attributes
struct termios orig_terminal_attrs;

/** error handling **/
void die(const char *s) {
    // when there is an error, clear the screen and restore the cursor positions
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    perror(s);
    exit(1);
}
/** terminal operations **/
// disable raw mode
void disableRawMode () {
    // restore the terminal with its original terminal attributes
    // tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_terminal_attrs);
    if( tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_terminal_attrs) == -1 ) die("tcsetattr");
}

// enable raw mode
void enableRawMode () {
    // make a copy of the current terminal attributes
    // tcgetattr(STDIN_FILENO, &orig_terminal_attrs);
    if (tcgetattr(STDIN_FILENO, &orig_terminal_attrs) == -1 ) die("tcgetattr");
    // at the exit of the program disable raw mode
    atexit(disableRawMode);
    // copy the original terminal attributes to our raw
    struct termios raw = orig_terminal_attrs;

    // tcgetattr(STDIN_FILENO, &raw);
    if (tcgetattr(STDIN_FILENO, &raw) == -1 ) die("tcgetattr");

    // -----------------------------------
    // input flags:
    // we are doing bitwise AND operator on the ECHO which is a BITWISE value
    // disabling software control flow operations
    // ICRNL => carriage return and new line flag
    // IXON => ctrl-s and ctrl-q
    raw.c_iflag &= ~(BRKINT | INPCK | ISTRIP | ICRNL | IXON);
    // -----------------------------------
    // disabling OPOST flag for carraige return new line conversion by terminal when outputting
    // -----------------------------------
    // output flags:
    raw.c_oflag &= ~(OPOST);
    // -----------------------------------
    // ICANON => also disable the canonical mode with ICANON
    // ISIG => also disabling SIGINT & SIGTSTP for ctrl-c and ctrl-z
    // IEXTEN => for disabling ctrl-v
    // -----------------------------------
    // control flags:
    raw.c_cflag |= (CS8);
    // -----------------------------------
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);

    // input read timeout values
    raw.c_cc[VMIN] = 0;
    // time in milliseconds
    raw.c_cc[VTIME] = 1;
    // tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    if( tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1 ) die("tcsetattr");
}

/** input processing methods **/
char editorReadKey() {

    int input_read,
        valid_input_read;

    char c;

    do {
        input_read = read(STDIN_FILENO, &c, 1);
        valid_input_read = (input_read != 1);
        // handling error when reading input
        if (input_read == -1 && errno != EAGAIN) die("read");
    } while (valid_input_read);

    return c;
}

/** output **/

void editorDrawRows() {
    int y;
    for (y = 0; y < 24; y++) {
        write(STDOUT_FILENO, "~\r\n", 3);
    }
}


void editorRefreshScreen() {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    editorDrawRows();
    write(STDOUT_FILENO, "\x1b[H", 3);
}
// process individual key presses
void processKeyPress() {

    char c = editorReadKey();

    switch (c) {
        case CTRL_KEY('q'):
            exit(0);
            break;

        default:
            break;
    }
}

/** init **/
int main () {

    enableRawMode();

    while (1) {
        editorRefreshScreen();
        processKeyPress();
    }

    return 0;
}
