#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <algorithm>
#include <cctype>

#define CTRL_KEY(k) ((k) & 0x1f) // Macro to handle control key combinations

class Editor
{
private:
    std::string buffer, filename;
    int screenrows, screencols;
    int cursor_x, cursor_y;
    int numrows, rowoff, coloff;
    std::vector<std::string> rows = {""}; // Initialize with an empty row

public:
    Editor();
    ~Editor();
    std::string path;
    void RefreshScreen();
    bool ProcessKeypress();
    void Draw();
    void Open(const std::string &path_to_file);
    void saveToFileText(const std::string &filename);

private:
    char ReadKey();
    void DrawRows();
    void DrawStatusBar();
    int getWindowSize();
    int getCursorPosition();
    void editRow(char c);
    bool escCase(char c);
    void enterCase();
    void tabCase();
    void backCase();
    void delCase();
    void str_trim(std::string &str);
    void end_line();
    void begin_line();
};

Editor::Editor() : cursor_x{0}, cursor_y{0}, numrows{1}, rowoff{0}, coloff{0}
{
    getWindowSize(); // Initialize the screen size
}

Editor::~Editor()
{
    // Clear the screen and reset cursor position on exit

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
    int rowlen = row ? row->size() : 0;
    if (cursor_x > rowlen)
    {
        cursor_x = rowlen; // Adjust cursor position if it's beyond the row length
    }
}

char Editor::ReadKey()
{
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
    {
        if (nread == -1 && errno != EAGAIN)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }
    }
    return c; // Return the read character
}

bool Editor::ProcessKeypress()
{
    char c = ReadKey();
    switch (c)
    {
    case 27: // Escape key
        return escCase(c);
        break;
    case 127: // Backspace
        backCase();
        break;
    case '\r': // Enter
        enterCase();
        break;
    case '\t': // Tab
        tabCase();
        break;
    default:
        editRow(c); // Add character to the current row
    }
    return false;
}

void Editor::DrawRows()
{
    getWindowSize(); // Get the current window size
    for (int y{0}; y < screenrows; y++)
    {
        int filerow = y + rowoff;
        if (y == screenrows - 1)
        {
            DrawStatusBar(); // Draw the status bar on the last row
        }
        else if (filerow < rows.size())
        {
            int len = screencols + coloff;
            int size = rows[filerow].size();
            len = std::min(len, size);
            buffer.append(rows[filerow].substr(coloff, len)); // Append the current row to the buffer
        }

        buffer.append("\x1b[K"); // Clear the rest of the line

        if (y < screenrows - 1)
        {
            buffer.append("\r\n"); // Move to the next line
        }
    }
}

void Editor::Draw()
{
    buffer.append("\x1b[?25l"); // Hide the cursor
    RefreshScreen();            // Refresh the screen
    DrawRows();                 // Draw the rows
    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (cursor_y) + 1, (cursor_x) + 1); // Move the cursor to the correct position
    buffer.append(buf);
    buffer.append("\x1b[?25h");                          // Show the cursor
    write(STDOUT_FILENO, buffer.c_str(), buffer.size()); // Write the buffer to the screen
    buffer.clear();                                      // Clear the buffer
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
    buf[i] = false;
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

void Editor::Open(const std::string &path_to_file)
{
    numrows = 0;
    rows.pop_back();
    std::ifstream file(path_to_file);
    if (!file.is_open())
    {
        std::cerr << "Could not open the file " << filename << std::endl;
        return;
    }
    // set filename
    std::filesystem::path fs_path(path_to_file);
    this->filename = fs_path.filename().string();

    std::string line;
    while (std::getline(file, line))
    {
        // Remove trailing newline and carriage return characters
        while (!line.empty() && (line.back() == '\n' || line.back() == '\r'))
        {
            line.pop_back();
        }

        std::string row = line;

        rows.push_back(row);
        numrows++;
    }

    file.close();
}

void Editor::DrawStatusBar()
{
    std::string str = "";
    buffer.append("\x1b[1;7m");
    if (!filename.empty())
    {
        str += "File name: " + filename;
    }
    else
    {
        str += "[NO NAME] ";
    }
    str += " " + std::to_string(cursor_y + rowoff + 1) + " : " + std::to_string(cursor_x + coloff + 1) + "   EXIT:[ESC]  ";
    buffer.append(str);
    int len = str.length();
    while (len < screencols)
    {
        buffer.append(" "); // Fill the rest of the line with spaces
        len++;
    }
    buffer.append("\x1b[m"); // Reset colors
}

void Editor::editRow(char c)
{
    int filerow = cursor_y + rowoff;
    if (filerow >= rows.size())
    {
        return; // Safe guard in case filerow is out of bounds
    }

    std::string *row = &rows[filerow];

    if (cursor_x >= screencols - 1)
    {
        // If cursor is at the end of screen, move to next row
        numrows++;
        rows.push_back(std::string());
        cursor_y++;
        cursor_x = 0;
        coloff = 0;
        row = &rows[filerow + 1];
    }
    else if (row->size() >= screencols - 1)
    {
        if (filerow == numrows - 1)
        {
            std::string newRow;
            newRow = row->back();
            rows.insert(rows.begin() + filerow + 1, newRow);
            numrows++;
        }
        else
        {
            std::string *second_row = &rows[filerow + 1];
            second_row = row->back() + second_row; // Move the last character to the new row
        }
        // Split the row if it exceeds screen columns
        row->pop_back();
        if (cursor_x > row->size())
        {
            cursor_y++;
            cursor_x = 1; // After moving the last char, the new row starts at position 1
        }
    }

    row->insert(cursor_x + coloff, 1, c); // Insert the character at the current position
    cursor_x++;
}

bool Editor::escCase(char c)
{
    char seq[3];
    auto *row = (cursor_y >= numrows) ? NULL : &rows[cursor_y + rowoff];
    if (read(STDIN_FILENO, &seq[0], 1) == 0)
        return true;
    if (read(STDIN_FILENO, &seq[1], 1) == 0)
        return true;

    if (seq[0] == '[')
    {
        if (seq[1] >= '0' && seq[1] <= '9')
        {
            if (read(STDIN_FILENO, &seq[2], 1) != 1)
                return true;
            if (seq[2] == '~')
            {
                getWindowSize();
                switch (seq[1])
                {
                case '3': // delete
                    delCase();
                    return false;
                case '1':
                case '7':
                    begin_line();
                    return false;
                case '4':
                case '8':
                    end_line();
                    return false;
                case '5': // page up
                    if (cursor_y >= 1)
                    {
                        cursor_y = 0;
                    }
                    else
                    {
                        rowoff -= (screencols - 2);
                        if (rowoff < 0)
                        {
                            rowoff = 0;
                        };
                    }
                    return false;
                case '6': // page down
                    if (cursor_y < screenrows - 2)
                    {
                        cursor_y = screenrows - 2;
                    }
                    else
                    {
                        rowoff += cursor_y;
                        if (cursor_y + rowoff > numrows)
                        {
                            rowoff = numrows - cursor_y - 1;
                            cursor_y = numrows - rowoff - 1;
                        }
                    }

                    return false;
                }
            }
        }
        else
        {
            switch (seq[1])
            {
            case 'A': // Up arrow
                if (cursor_y > 0)
                {
                    cursor_y--;
                }
                else if (rowoff > 0)
                {
                    rowoff--;
                }

                return false;
            case 'B': // Down arrow
                if (cursor_y < screenrows - 2)
                {
                    cursor_y++;
                }
                else if (rowoff + cursor_y + 1 < numrows)
                {
                    rowoff++;
                }
                return false;
            case 'C': // Right arrow
                if (row && cursor_x < screencols - 1 && cursor_x + coloff < row->size() - 1)
                {
                    cursor_x++;
                }
                else if (cursor_x + coloff + 1 < row->size() - 1)
                {
                    coloff += cursor_x;
                    cursor_x = 0;
                }
                else if (cursor_x + coloff >= row->size() - 1 && row)
                {
                    cursor_y++;
                    begin_line();
                }

                return false;
            case 'D': // Left arrow
                if (cursor_x != 0)
                {
                    cursor_x--;
                }
                else if (coloff > 0)
                {
                    cursor_x = screencols - 1;
                    coloff -= screencols;
                    if (coloff < 0)
                    {
                        coloff = 0;
                    }
                }
                else if (cursor_y > 0)
                {
                    cursor_y--;
                    end_line();
                }
                return false;
            case 'H':
                begin_line();
                return false;
            case 'F':
                end_line();
                return false;
            }
        }
    }
    else if (seq[0] == 'O')
    {
        switch (seq[1])
        {
        case 'H':
            begin_line();
            return false;
        case 'F':
            end_line();
            return false;
        }
    }
    return false;
}

void Editor::enterCase()
{
    int filerow = cursor_y + rowoff;
    std::string *row = &rows[filerow];
    std::string newRow;
    if (cursor_x == 0)
    {
        rows.insert(rows.begin() + filerow, newRow);
    }
    else if (cursor_x >= row->size())
    {
        rows.insert(rows.begin() + filerow + 1, newRow);
    }
    else
    {
        std::string tailstr = row->substr(cursor_x, screencols),
                    tipstr = row->substr(0, cursor_x);
        *row = tipstr;
        if (filerow + 1 >= numrows)
        {
            rows.push_back(std::string());
            numrows++;
        }
        row = &rows[filerow + 1];
        if (row->size() + tailstr.length() <= screencols - 1)
        {
            *row = tailstr + *row;
            cursor_x = 0;
            coloff = 0;
            cursor_y++;
            return;
        }
        else
        {
            newRow = tailstr;
            rows.insert(rows.begin() + filerow + 1, newRow);
        }
    }
    cursor_x = 0;
    coloff = 0;
    numrows++;
    cursor_y++;
    return;
}

void Editor::tabCase()
{
    for (size_t i{0}; i < 4; i++)
        editRow(' ');
}

void Editor::backCase()
{
    int filerow = cursor_y + rowoff;
    if (filerow >= rows.size())
    {
        return; // Safe guard in case filerow is out of bounds
    }

    std::string *row = &rows[filerow];
    if (row->size() <= 1)
    {
        if (numrows > 1)
        {
            // Remove the character before the cursor position
            rows.erase(rows.begin() + filerow);
            numrows--;
        }
        else
        {
            *row = "";
        }
        if (filerow > 0)
        {
            cursor_x = rows[filerow - 1].size();
            cursor_y--;
            return;
        }
        cursor_x = 0;
        return;
    }
    else
    {
        if (cursor_x <= 0)
        {
            if (filerow <= 0)
            {
                return;
            }
            int filler = screencols - rows[filerow - 1].size();
            std::string tailstr = (filler < row->size()) ? row->substr(filler, screencols - filler) : "";
            std::string tipstr = (filler >= 0) ? row->substr(0, filler) : *row;
            cursor_x = rows[filerow - 1].size();
            cursor_y--;

            rows[filerow - 1] += tipstr;
            *row = tailstr;

            return;
        }
        int len = row->length();
        std::string tailstr = row->substr(cursor_x + coloff, len),
                    tipstr = row->substr(0, cursor_x + coloff);
        tipstr.pop_back();
        *row = tipstr + tailstr;
    }
    cursor_x--;
}

void Editor::delCase()
{
    int filerow = cursor_y + rowoff;
    if (filerow >= numrows)
        return; // Ensure filerow is within bounds

    std::string &row = rows[filerow];

    if (row.size() <= 1)
    {
        if (numrows > 1)
        {
            // Remove the character at the cursor position
            rows.erase(rows.begin() + filerow);
            numrows--;
        }
        else
        {
            row = "";
            cursor_x = 0;
            coloff = 0;
        }
        return;
    }

    if (cursor_x >= row.size())
    {
        if (filerow < numrows - 1)
        {
            cursor_x = 0;
            coloff = 0;
            cursor_y++;
            backCase();
        }
        return;
    }

    if (cursor_x < row.size() - 1)
    {
        std::string tipstr = row.substr(0, cursor_x + coloff);
        std::string tailstr = row.substr(cursor_x + 1 + coloff, row.size() - cursor_x - 1 - coloff);
        row = tipstr + tailstr;
        return;
    }

    // If cursor_x is at the last character, just remove that character
    row = row.substr(0, row.size() - 1);
}

void Editor::str_trim(std::string &str)
{
    str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch)
                           { return !std::isspace(ch); })
                  .base(),
              str.end());
}

void Editor::saveToFileText(const std::string &filename)
{
    std::ofstream outFile(filename);

    if (!outFile)
    {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
        return;
    }

    for (auto &str : rows)
    {
        str_trim(str);
        outFile << str << '\n';
    }

    outFile.close();
}

void Editor::end_line()
{
    auto *row = (cursor_y >= numrows) ? NULL : &rows[cursor_y + rowoff];

    if (row && row->size() <= screencols)
    {
        cursor_x = row->size();
    }
    else
    {
        cursor_x = screencols;
        coloff = row->size() - screencols;
    }
}

void Editor::begin_line()
{
    cursor_x = 0;
    coloff = 0;
}