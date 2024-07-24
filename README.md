# Txt_Editor_Cpp

Txt_Editor_Cpp is a simple text editor that operates within the terminal. The repository includes a Dockerfile to create a development environment, and the project can be built using CMake.

## Features

- **Opening existing text files**: Load and display text files for editing.
- **Editing text in the terminal**: Perform basic text editing functions such as inserting, deleting, and navigating text.
- **Saving changes to a file**: Save your edits back to the file.

## Requirements

- **Docker** (optional, for creating the development environment)
- **CMake** (for building the project)

## Creating the Development Environment

To create the development environment using Docker, use the provided Dockerfile. The Dockerfile sets up all necessary dependencies and tools required to build and run the project.

1. Build the Docker image:
    ```sh
    docker build -t txt_editor_cpp .
    ```

2. Run a Docker container from the image:
    ```sh
    docker run -it --rm --name txt_editor_cpp_container txt_editor_cpp
    ```

## Building the Project

To build the project using CMake:

1. Clone the repository:
    ```sh
    git clone https://github.com/FranciszekCieslik/Txt_Editor_Cpp.git
    cd txt_editor_cpp
    ```

2. Create a build directory and navigate into it:
    ```sh
    mkdir build
    cd build
    ```

3. Run CMake and build the project:
    ```sh
    cmake ..
    make
    ```

## Running the Editor

To open an existing text file, use the following command:

```sh
./txt_editor_app [file path]

# Example
To open the file example.txt, run:
```sh
./txt_editor_app example.txt
```

## Editor Controls
- Arrow Keys: Move the cursor up, down, left, and right.
- Enter: Insert a new line.
- Backspace: Delete the character before the cursor.
- Tab: Insert four spaces.
- Escape: Exit the editor.
## Code Structure

The Editor class handles the main functionality of the text editor, including:

- Opening files: Reading files into the editor buffer.
- Editing text: Inserting and deleting characters.
- Rendering: Displaying the text buffer to the terminal.
- Handling input: Processing key presses for editing and navigation.
- Raw Mode Configuration
  
**The editorConfig** class manages terminal settings to enable raw mode, allowing the editor to read input character-by-character without waiting for a newline.
