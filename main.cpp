#include <iostream>
#include "heads.h"

#define CTRL_KEY(k) ((k) & 0x1f)

int main(int argc, char *argv[])
{
    editorConfig EditorConfig;
    Editor editor;
    if (argc >= 2)
    {
        editor.path = argv[1];
        editor.Open(argv[1]);
    }
    while (true)
    {
        editor.Draw();
        if (editor.ProcessKeypress())
            break;
    };
    std::cout<<"\x1b[2J\x1b[?25h\x1b[0;0H";
    EditorConfig.~editorConfig();
    std::string save_path;
    std::cout << "Save path: ";
    std::getline(std::cin, save_path);
    std::cout << "File will be saved in: " << save_path << std::endl;
    editor.saveToFileText(save_path);
    std::cin.get();
    return EXIT_SUCCESS;
}