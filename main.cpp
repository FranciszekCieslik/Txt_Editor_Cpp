#include <iostream>
#include "my_function.h"

#define CTRL_KEY(k) ((k) & 0x1f)

int main()
{
    struct termios orig_termios;
    enableRawMode(&orig_termios);
    while (true) 
    {
       editorRefreshScreen();
       if(editorProcessKeypress()) break;
    };
    editorRefreshScreen();
    disableRawMode(&orig_termios);
    return EXIT_SUCCESS;
}