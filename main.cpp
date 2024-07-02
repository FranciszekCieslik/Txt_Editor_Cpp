#include <iostream>
#include "Editor.cpp"

#define CTRL_KEY(k) ((k) & 0x1f)

int main()
{
    editorConfig EditorConfig;
    Editor editor;
    while (true) 
    {
       editor.Draw();
       if(editor.ProcessKeypress()) break;
    };
    return EXIT_SUCCESS;
}