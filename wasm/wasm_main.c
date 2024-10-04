//
// Created by ran on 24-10-3.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vm/vm.h"
#include "common/common.h"
#ifdef WASM_LOG
#include <emscripten/emscripten.h>
#endif

void log_immediate(const char* message) {
#ifdef WASM_LOG
    EM_ASM_({console.warn(UTF8ToString($0));}, message);
#endif
#ifndef WASM_LOG
    printf("%s\n",message);
#endif
}

// WebAssembly entry
void process_file_content(const char* content) {
    if (content == NULL) {
        printf("Error: content is NULL\n");
        return;
    }
    log_immediate("[status]-Receive content.");
    log_immediate("[status]-Initialize ZHI VM...");
    VM vm;
    init_VM(&vm,NULL,NULL,true);
#ifdef WASM_LOG
    log_immediate("[status]-ZHI VM Initialized.");
    log_immediate("[status]-interpret ...");
#endif
    InterpretResult result = interpret(&vm, NULL, content);
    printf("\n");
    // TODO In an Emscripten-compiled WebAssembly environment, output buffering behavior may differ from traditional
    //  native environments. In particular, fflush(stdout); may not work as expected under Emscripten, causing output
    //  to not be flushed immediately to the browser's console.
//    fflush(stdout);
#ifdef WASM_LOG
    if(result == PARSE_ERROR) {
        log_immediate("[status][error]-Parse error.");
    }
    else if (result == INTERPRET_COMPILE_ERROR) {
        log_immediate("[status][error]-Compile error.");
    }
    else if(result == INTERPRET_RUNTIME_ERROR){
        log_immediate("[status][error]-Runtime error.");
    }
#endif
    free_VM(&vm);
#ifdef WASM_LOG
    log_immediate("[status]-Process ended.\n");
#endif
}



// js interface
EMSCRIPTEN_KEEPALIVE
void handle_file_content(const char* content) {
    process_file_content(content);
}

