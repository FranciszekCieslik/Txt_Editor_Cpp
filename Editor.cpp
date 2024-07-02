#include "editorConfig.cpp"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

class Editor
{
private:
    editorConfig* edCon;
    int screenrows;
    int screencols;
public:
    Editor();
    ~Editor();
    void RefreshScreen();
    bool ProcessKeypress();
    void Draw();
private:
    char ReadKey();
    void DrawRows();
    int getWindowSize();
    int getCursorPosition();
};

Editor::Editor()
{

}

Editor::~Editor()
{
    RefreshScreen();
}

void Editor::RefreshScreen() {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
}

char Editor::ReadKey()
{
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1);
    if (c == 27) { // If the key is 'Esc'
        char seq[3];
        if (read(STDIN_FILENO, &seq[0], 1) == 0) return 27;
        if (read(STDIN_FILENO, &seq[1], 1) == 0) return 27;

        if (seq[0] == '[') {
            switch (seq[1]) {
                case 'A': return 'k'; // Up arrow
                case 'B': return 'j'; // Down arrow
                case 'C': return 'l'; // Right arrow
                case 'D': return 'h'; // Left arrow
            }
        }
    }
    return c;
}


bool Editor::ProcessKeypress() {
  char c = ReadKey();
  switch (c) {
    case 27:
      RefreshScreen();
      return true;
      break;
  }
  return false;
}

void Editor::DrawRows() {
  getWindowSize();
  for (int y{0}; y < screenrows; y++) {
    write(STDOUT_FILENO, "~", 1);
    if (y < screenrows - 1) {
      write(STDOUT_FILENO, "\r\n", 2);
    }
  }
}

void Editor::Draw()
{
    RefreshScreen();
    DrawRows();
    write(STDOUT_FILENO, "\x1b[H", 3);
}

int Editor::getCursorPosition() {
  char buf[32];
  unsigned int i = 0;
  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;
  while (i < sizeof(buf) - 1) {
    if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
    if (buf[i] == 'R') break;
    i++;
  }
  buf[i] = '\0';
  if (buf[0] != '\x1b' || buf[1] != '[') return -1;
  if (sscanf(&buf[2], "%d;%d", &screenrows, &screencols) != 2) return -1;
  return 0;
}

int Editor::getWindowSize() {
  struct winsize ws;
  if (1 || ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
    return getCursorPosition();
  } else {
    screencols = ws.ws_col;
    screenrows = ws.ws_row;
    return 0;
  }
}