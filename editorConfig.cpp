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

void editorConfig::enableRawMode()
{
    // Get the current terminal attributes and save them to orig_termios
    tcgetattr(STDIN_FILENO, orig_termios);

    // Create a copy of the original terminal attributes for modification
    raw = new termios(*orig_termios);

    // Modify input flags:
    // BRKINT: Disable break condition signal.
    // ICRNL: Disable mapping of CR (carriage return) to NL (newline).
    // INPCK: Disable parity checking.
    // ISTRIP: Disable stripping of the eighth bit.
    // IXON: Disable XON/XOFF flow control.
    raw->c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    // Modify output flags:
    // OPOST: Disable implementation-defined output processing.
    raw->c_oflag &= ~(OPOST);

    // Modify control flags:
    // CS8: Set character size to 8 bits per byte.
    raw->c_cflag |= (CS8);

    // Modify local flags:
    // ECHO: Disable echoing of typed characters.
    // ICANON: Disable canonical mode (line buffering).
    // IEXTEN: Disable implementation-defined input processing.
    // ISIG: Disable generation of signal characters (e.g., Ctrl-C).
    raw->c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    // Modify special control characters:
    // VMIN: Minimum number of bytes of input needed before read() returns.
    raw->c_cc[VMIN] = 0;
    // VTIME: Maximum time to wait before read() returns (in tenths of a second).
    raw->c_cc[VTIME] = 1;

    // Set the modified attributes immediately
    tcsetattr(STDIN_FILENO, TCSAFLUSH, raw);
}

void editorConfig::disableRawMode()
{
    // Set the origin attributes immediately
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
