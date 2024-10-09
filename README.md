# build-LanguageForMyDaughter

# ZHI Language

This project is the development of a custom programming language designed with simplicity and creativity in mind, created as a fun, learning-oriented language for my daughter. It is a dynamic, interpreted language featuring bytecode, a virtual machine, garbage collection (GC), and debugging tools such as AST (Abstract Syntax Tree) output and bytecode decompilation, allowing playful coding concepts while remaining powerful enough to demonstrate important programming fundamentals.

## Project Structure

- **src**: Contains all source code for the core language, including:

- **test**: Includes unit tests for the various parts of the language core.

- **testwasm**: Tests specific to the WebAssembly (WASM) version of the language.

- **wasm**: Contains code to compile the project to WebAssembly, allowing it to run in the browser. This directory includes:
    - `wasm_main.c`: The main entry point for the WebAssembly version.

- **wasmzhistd**: A directory containing `.duo` files to be included in the WASM module and embedded in the final WebAssembly output.

- **website**: Frontend for the language's web playground.

- **zhi**: Contains the entry point for the command-line executable version of the language:
    - `main.c`: The main entry point for the command-line tool.

## Building

This project uses CMake for building and supports multiple build configurations:

1. **Native Build** (Linux/Mac):
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```
   This will create an executable named ZHI in the bin directory.
2. (Optional) WebAssembly Build (for running in the browser): Make sure you have Emscripten installed, then run:
   ```
   mkdir build-emcc
   cd build-emcc
   emcmake cmake ..
   emmake make
   ```
   This will create a .js file and corresponding .wasm module.

## Usage
1. Command-Line Execution: The ZHI executable can be run from the command line, where you can pass .duo scripts (the language's file extension) for interpretation.

2. WebAssembly: The WebAssembly version can be used in the browser, with the language running through a web interface where users can write and execute .duo code interactively

## Features
- Dynamic Typing: The language supports dynamic typing for easy experimentation.
- Closures and Functions: Supports powerful closures and first-class functions.
- Object-Oriented: Includes basic support for object-oriented programming with classes and inheritance.
- Playground Integration: A web playground using WebAssembly is available for users to experiment with the language interactively.

## Future Plans

This language is still evolving, with plans to add more playful and educational features to enhance the learning experience for new programmers, especially young learners. Ongoing efforts include support for exception handling and coroutines to enable more advanced control flow and asynchronous programming capabilities.

## License
This project is licensed under the MIT License.