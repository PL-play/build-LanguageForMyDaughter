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
//    printf("Received file content: %s\n", content);
//    printf("Initialize ZHI VM...\n");
    VM vm;
    init_VM(&vm,NULL,NULL,true);
//    printf("Initialized.\n");
//    printf("interpret ...\n");
    InterpretResult result = interpret(&vm, NULL, content);
    // TODO In an Emscripten-compiled WebAssembly environment, output buffering behavior may differ from traditional
    //  native environments. In particular, fflush(stdout); may not work as expected under Emscripten, causing output
    //  to not be flushed immediately to the browser's console.
    printf("\n");
    fflush(stdout);
//    printf("interpret result:%s.\n",result==0?"OK":"ERROR");
    if (result == INTERPRET_COMPILE_ERROR) {
        printf("compile error\n");
    }
    if(result == INTERPRET_RUNTIME_ERROR){
        printf("runtime error\n");
    }
//    printf("Free ZHI VM...\n");
    free_VM(&vm);
//    printf("process_file_content ended.\n");
}



// js interface
EMSCRIPTEN_KEEPALIVE
void handle_file_content(const char* content) {
    process_file_content(content);
}

