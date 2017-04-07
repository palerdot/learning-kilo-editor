/** includes **/
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>


/** data **/
// struct for storing current terminal's original attributes
struct termios orig_terminal_attrs;

/** error handling **/
void die(const char *s) {
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


/** init **/
int main () {

    enableRawMode();

    char c; // gets the current pressed char from the terminal
    // following line scans for the input character
    // it does so till it scans for Ctrl + D exit character
    // both 'read' and 'STDIN_FILENO' is defined in unistd.h
    // also exit when we type the letter 'q'
    while (1) {
        c = '\0';
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");
        if (iscntrl(c)) {
            // printing non printable characters
            //carriage return: \r is used to return to the start of the line
            //new line: \n is used to return to a new line
            printf("%d \r\n", c);
        } else {
            // printable characters
            printf("%d ('%c') \r\n", c, c);
        }
        // break the input queue if we encounter 'q'
        if (c == 'q') {
            break;
        }
    }

    return 0;
}
