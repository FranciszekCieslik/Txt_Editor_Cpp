#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

class editorConfig
{
private:
  struct termios *orig_termios;
  struct termios *raw;
public:
    editorConfig(/* args */);
    ~editorConfig();
private:
    void enableRawMode();
    void disableRawMode();
};


void editorConfig::enableRawMode() {
    tcgetattr(STDIN_FILENO, orig_termios);
    raw = new  termios (*orig_termios);
    raw->c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw->c_oflag &= ~(OPOST);
    raw->c_cflag |= (CS8);
    raw->c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw->c_cc[VMIN] = 0;
    raw->c_cc[VTIME] = 1;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, raw);
}

void editorConfig::disableRawMode() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, orig_termios);
}

editorConfig::editorConfig(/* args */)
{
    orig_termios = new struct termios; 
    enableRawMode();
}

editorConfig::~editorConfig()
{
    disableRawMode();
}
