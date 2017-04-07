#include <ctype.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>

// struct for storing current terminal's original attributes
struct termios orig_terminal_attrs;

// disable raw mode
void disableRawMode () {
    // restore the terminal with its original terminal attributes
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_terminal_attrs);
}

// enable raw mode
void enableRawMode () {
    // make a copy of the current terminal attributes
    tcgetattr(STDIN_FILENO, &orig_terminal_attrs);
    // at the exit of the program disable raw mode
    atexit(disableRawMode);
    // copy the original terminal attributes to our raw
    struct termios raw = orig_terminal_attrs;

    tcgetattr(STDIN_FILENO, &raw);

    // we are doing bitwise AND operator on the ECHO which is a BITWISE value
    // also disable the canonical mode with ICANON
    raw.c_lflag &= !(ECHO | ICANON);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}


int main () {

    enableRawMode();

    char c; // gets the current pressed char from the terminal
    // following line scans for the input character
    // it does so till it scans for Ctrl + D exit character
    // both 'read' and 'STDIN_FILENO' is defined in unistd.h
    // also exit when we type the letter 'q'
    while( read(STDIN_FILENO, &c, 1) == 1 && c != 'q' ) {
        if (iscntrl(c)) {
            // printing non printable characters
            printf("%d \n", c);
        } else {
            // printable characters
            printf("%d ('%c') \n", c, c);
        }
    }

    return 0;
}
