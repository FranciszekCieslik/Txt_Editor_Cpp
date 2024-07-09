#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>

#define CTRL_KEY(k) ((k) & 0x1f)

struct Row
{
    int size;
    std::string chars;
};

class Editor
{
private:
    std::string buffer;
    int screenrows, screencols;
    int cursor_x, cursor_y;
    int numrows, rowoff, coloff;
    std::vector<Row> rows;

public:
    Editor();
    ~Editor();
    void RefreshScreen();
    bool ProcessKeypress();
    void Draw();
    void Open(const std::string &filename);

private:
    char ReadKey();
    void DrawRows();
    int getWindowSize();
    int getCursorPosition();
    void scroll();
};

Editor::Editor() : cursor_x{0}, cursor_y{0}, numrows{0}, rowoff{0}, coloff{0}
{
    getWindowSize();
}

Editor::~Editor()
{
    buffer.append("\x1b[2J");
    buffer.append("\x1b[?25h");
    buffer.append("\x1b[0;0H");
    write(STDIN_FILENO, buffer.c_str(), buffer.size());
}

void Editor::RefreshScreen()
{
    buffer.append("\x1b[H");
    auto *row = (cursor_y >= numrows) ? NULL : &rows[cursor_y];
    row = (cursor_y >= numrows) ? NULL : &rows[cursor_y];
    int rowlen = row ? row->size : 0;
    if (cursor_x > rowlen)
    {
        cursor_x = rowlen;
    }
}

char Editor::ReadKey()
{
    int nread;
    char c;
    auto *row = (cursor_y >= numrows) ? NULL : &rows[cursor_y];
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
    {
        if (nread == -1 && errno != EAGAIN)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }
    }

    if (c == 27)
    { // If the key is 'Esc'
        char seq[3];
        if (read(STDIN_FILENO, &seq[0], 1) == 0)
            return 27;
        if (read(STDIN_FILENO, &seq[1], 1) == 0)
            return 27;

        if (seq[0] == '[')
        {
            if (seq[1] >= '0' && seq[1] <= '9')
            {
                if (read(STDIN_FILENO, &seq[2], 1) != 1)
                    return '\x1b';
                if (seq[2] == '~')
                {
                    getWindowSize();
                    switch (seq[1])
                    {
                    case '3': // delete
                        cursor_x++;
                        return '\b';
                    case '1':
                    case '7':
                        cursor_x = 0;
                        return '\0';
                    case '4':
                    case '8':
                        cursor_x = screencols - 1;
                        return '\0';
                    case '5':
                        cursor_y = 0;
                        return '\0';
                    case '6':
                        cursor_y = screenrows - 1;
                        return '\0';
                    }
                }
            }
            else
            {
                switch (seq[1])
                {
                case 'A': // Up arrow
                    if (cursor_y != 0)
                        cursor_y--;
                    return '\0';
                case 'B': // Down arrow
                    if (cursor_y < numrows)
                        cursor_y++;
                    return '\0';
                case 'C': // Right arrow
                    if (row && cursor_x < row->size)
                    {
                        cursor_x++;
                    }else if (row && cursor_x == row->size)
                    {
                        cursor_y++;
                        cursor_x = 0;
                    }
                    
                    return '\0';
                case 'D': // Left arrow
                    if (cursor_x != 0){
                        cursor_x--;
                    }else if(cursor_y > 0){
                        cursor_y--;
                        cursor_x = rows[cursor_y].size;
                    }
                    return '\0';
                case 'H':
                    cursor_x = 0;
                    return '\0';
                case 'F':
                    cursor_x = screencols - 1;
                    return '\0';
                }
            }
        }
        else if (seq[0] == 'O')
        {
            switch (seq[1])
            {
            case 'H':
                cursor_x = 0;
                return '\0';
            case 'F':
                cursor_x = screencols - 1;
                return '\0';
            }
        }
    }
    return c; // Return the character read if not an escape sequence
}

bool Editor::ProcessKeypress()
{
    char c = ReadKey();
    switch (c)
    {
    case 27:
        return true;
        break;
    }
    return false;
}

void Editor::DrawRows()
{
    getWindowSize();
    for (int y{0}; y < screenrows; y++)
    {
        int filerow = y + rowoff;
        if (filerow >= numrows)
        {
            buffer.append("~");
        }
        else
        {
            int len = rows[filerow].size - coloff;
            if (len < 0)
                len = 0;
            if (len > screencols)
                len = screencols + 1;
            buffer.append(rows[filerow].chars.substr(coloff, len));
        }
        buffer.append("\x1b[K");
        if (y < screenrows - 1)
        {
            buffer.append("\r\n");
        }
    }
}

void Editor::Draw()
{
    scroll();
    buffer.append("\x1b[?25l");
    RefreshScreen();
    DrawRows();
    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (cursor_y - rowoff) + 1, (cursor_x - coloff) + 1);
    buffer.append(buf);
    buffer.append("\x1b[?25h");
    write(STDOUT_FILENO, buffer.c_str(), buffer.size());
    buffer.clear();
}

int Editor::getCursorPosition()
{
    char buf[32];
    unsigned int i = 0;
    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
        return -1;
    while (i < sizeof(buf) - 1)
    {
        if (read(STDIN_FILENO, &buf[i], 1) != 1)
            break;
        if (buf[i] == 'R')
            break;
        i++;
    }
    buf[i] = '\0';
    if (buf[0] != '\x1b' || buf[1] != '[')
        return -1;
    if (sscanf(&buf[2], "%d;%d", &screenrows, &screencols) != 2)
        return -1;
    return 0;
}

int Editor::getWindowSize()
{
    struct winsize ws;
    if (1 || ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
    {
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
            return -1;
        return getCursorPosition();
    }
    else
    {
        screencols = ws.ws_col;
        screenrows = ws.ws_row;
        return 0;
    }
}

void Editor::Open(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Could not open the file " << filename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        // Remove trailing newline and carriage return characters
        while (!line.empty() && (line.back() == '\n' || line.back() == '\r'))
        {
            line.pop_back();
        }

        Row row;
        row.size = line.size();
        row.chars = line;

        rows.push_back(row);
        numrows++;
    }

    file.close();
}

void Editor::scroll()
{
    if (cursor_y < rowoff)
    {
        rowoff = cursor_y;
    }
    if (cursor_y >= rowoff + screenrows)
    {
        rowoff = cursor_y - screenrows + 1;
    }
    if (cursor_x < coloff)
    {
        coloff = cursor_x;
    }
    if (cursor_x >= coloff + screencols)
    {
        coloff = cursor_x - screencols + 1;
    }
}
