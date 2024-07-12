#include <iostream>
#include "heads.h"

#define CTRL_KEY(k) ((k) & 0x1f)

int main(int argc, char *argv[]) 
{
    editorConfig EditorConfig;
    Editor editor;
    if (argc >= 2)
        editor.Open(argv[1]);
    while (true) 
    {
       editor.Draw();
       if(editor.ProcessKeypress()) break;
    };
    return EXIT_SUCCESS;
}