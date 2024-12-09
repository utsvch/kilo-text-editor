#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>

struct termios orig_termios;

#define FLAG_INPUT 1
#define FLAG_OUTPUT 2
#define FLAG_CONTROL 4
#define FLAG_LOCAL 8

#define CTRL_KEY(k) ((k) & 0x1f)


void editorRefreshScreen() {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
}

void die(const char* s) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);

  perror(s);
  exit(1);
}


void turn_off_attr(struct termios *r, unsigned int feature_to_turn_off, tcflag_t flag_type) {
  if (flag_type == FLAG_LOCAL) {
    r->c_lflag &= ~(feature_to_turn_off);
  } 
  else if (flag_type == FLAG_INPUT) {
    r->c_iflag &= ~(feature_to_turn_off);
  }
  else if (flag_type == FLAG_OUTPUT) {
    r->c_oflag &= ~(feature_to_turn_off);
  }
  else if (flag_type == FLAG_CONTROL) {
    r->c_cflag |= (feature_to_turn_off);
  }
}


void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) die("tcsetattr");
}


void enableRawMode() {
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");
  atexit(disableRawMode);

  struct termios raw = orig_termios;


  // turn off canonical mode to read inputs byte-by-byte instead of line-by-line
  // turn_off_attr(&raw, ICANON, FLAG_LOCAL);

  // Turn off Ctrl-C and Ctrl-Z signals
  // turn_off_attr(&raw, ISIG, FLAG_LOCAL);

  // turn_off_attr(&raw, IEXTEN, FLAG_LOCAL);

  // Turn off Ctrl-S and Ctrl-Q data transmission signals
  // Club all the above together
  turn_off_attr(&raw, ( ECHO | ICANON | ISIG | IEXTEN ), FLAG_LOCAL);

  turn_off_attr(&raw, ( BRKINT | ICRNL | INPCK | ISTRIP | IXON ), FLAG_INPUT);

  // Turn off auto translation of carriage return to newline
  // turn_off_attr(&raw, ICRNL, FLAG_INPUT);

  // Turn off output processing flag that auto translates carriage returns to newline turn_off_attr(&raw, OPOST, FLAG_OUTPUT);

  turn_off_attr(&raw, CS8, FLAG_CONTROL);

  // control char settings
  // VMIN tracks min chars needed for read() can return 
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

char editorReadKey() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1) != 1)) {
    // error handling for reading input
    if (nread == -1 && errno != EAGAIN) die("read");
    printf("%c", c);
  }
  return c;
}


void editorProcessKeypress() {
  char c = editorReadKey();

  switch(c) {
    case CTRL_KEY('q'):
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);

      exit(0);
      break;
  }
}


int main() {
  enableRawMode();

  while (1) {
    editorProcessKeypress();
    editorRefreshScreen();
  }
  return 0;
}
