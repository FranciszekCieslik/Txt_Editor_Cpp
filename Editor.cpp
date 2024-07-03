#include "editorConfig.cpp"
#include "AppendBuffer.cpp"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <cerrno>

class Editor
{
private:
    AppendBuffer * buffer;
    int screenrows, screencols;
    int cursor_x, cursor_y;
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
    void MoveCursor(char key); 
};

Editor::Editor():cursor_x{0},cursor_y{0}
{
    buffer = new AppendBuffer;
    getWindowSize();
}

Editor::~Editor()
{
    buffer->append("\x1b[2J", 4);
    buffer->append("\x1b[?25h", 6);
    write(STDIN_FILENO, buffer->b, buffer->len);
}

void Editor::RefreshScreen() {
//   buffer->append("\x1b[2J", 4);
  buffer->append("\x1b[H", 3);
}

char Editor::ReadKey() {
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) {
            perror("read");
            exit(EXIT_FAILURE);
        }
    }
    
    if (c == 27) { // If the key is 'Esc'
        char seq[3];
        if (read(STDIN_FILENO, &seq[0], 1) == 0) return 27;
        if (read(STDIN_FILENO, &seq[1], 1) == 0) return 27;

        if (seq[0] == '[') {
            switch (seq[1]) {
                case 'A': // Up arrow
                    if (cursor_y != 0) cursor_y--;
                    return '\0';
                case 'B': // Down arrow
                    if (cursor_y != screenrows - 1) cursor_y++;
                    return '\0';
                case 'C': // Right arrow
                    if (cursor_x != screencols - 1) cursor_x++;
                    return '\0';
                case 'D': // Left arrow
                    if (cursor_x != 0) cursor_x--;
                    return '\0';
            }
        }
        return c; // If other escape sequences, ignore
    }

    return c; // Return the character read if not an escape sequence
}

bool Editor::ProcessKeypress() {
  char c = ReadKey();
  switch (c) {
    case 27:
      return true;
      break;
  }
  return false;
}

void Editor::DrawRows() {
  getWindowSize();
  for (int y{0}; y < screenrows; y++) {
    buffer->append("~",1);
    buffer->append("\x1b[K",3);
    if (y < screenrows - 1) {
      buffer->append("\r\n", 2);
    }
  }
}

void Editor::Draw()
{
    buffer->append("\x1b[?25l", 6);
    RefreshScreen();
    DrawRows();
    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", cursor_y + 1, cursor_x + 1);
    buffer->append(buf, strlen(buf));
    buffer->append("\x1b[?25h", 6);
    write(STDOUT_FILENO, buffer->b, buffer->len);
    buffer->free();
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