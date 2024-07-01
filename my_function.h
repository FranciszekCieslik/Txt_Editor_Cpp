#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

void die(const char *s) {
  perror(s);
  exit(1);
}

void disableRawMode(struct termios* orig_termios) {
  if(tcsetattr(STDIN_FILENO, TCSAFLUSH, orig_termios) == -1)
     die("tcsetattr");
}

void enableRawMode(struct termios * orig_termios) {
    if(tcgetattr(STDIN_FILENO, orig_termios) == -1) die("tcgetattr");
    struct termios raw = *orig_termios;
     raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
     raw.c_oflag &= ~(OPOST);
     raw.c_cflag |= (CS8);
     raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
     raw.c_cc[VMIN] = 0;
     raw.c_cc[VTIME] = 1;
    if( tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}
