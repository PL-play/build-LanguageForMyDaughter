//
// Created by ran on 24-10-3.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vm/vm.h"
#include <emscripten.h>
// WebAssembly entry
void process_file_content(const char* content) {
    if (content == NULL) {
        printf("Error: content is NULL\n");
        return;
    }
#ifdef WASM_LOG
        printf("[status]-Receive content.\n");
        printf("[status]-Initialize ZHI VM...\n");
#endif
    VM vm;
    init_VM(&vm,NULL,NULL,true);
#ifdef WASM_LOG
    printf("[status]-ZHI VM Initialized.\n");
    printf("[status]-interpret ...\n");
#endif
    InterpretResult result = interpret(&vm, NULL, content);
    printf("\n");
    // TODO In an Emscripten-compiled WebAssembly environment, output buffering behavior may differ from traditional
    //  native environments. In particular, fflush(stdout); may not work as expected under Emscripten, causing output
    //  to not be flushed immediately to the browser's console.
//    fflush(stdout);
#ifdef WASM_LOG
    if(result == PARSE_ERROR) {
        printf("[status][error]-Parse error.\n");
    }
    else if (result == INTERPRET_COMPILE_ERROR) {
        printf("[status][error]-Compile error.\n");
    }
    else if(result == INTERPRET_RUNTIME_ERROR){
        printf("[status][error]-Runtime error.\n");
    }
#endif
    free_VM(&vm);
#ifdef WASM_LOG
    printf("[status]-Process ended.\n");
#endif
}



// js interface
EMSCRIPTEN_KEEPALIVE
void handle_file_content(const char* content) {
    process_file_content(content);
}

