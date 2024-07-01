#include <iostream>
#include "my_function.h"

int main()
{
    struct termios orig_termios;
    enableRawMode(&orig_termios);
      while (true) {
        char c = '\0';
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");
        if (iscntrl(c)) {
            std::cout << static_cast<int>(c) << "\r\n";
        } else {
            std::cout << static_cast<int>(c) << " ('" << c << "')\r\n";
        }
        if(static_cast<int>(c) == 27) break;
    };
    disableRawMode(&orig_termios);
    return EXIT_SUCCESS;
}