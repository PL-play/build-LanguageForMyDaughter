//
// Created by ran on 24-10-3.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "vm/vm.h"
#include "common/common.h"
#ifdef WASM_LOG
#include <emscripten/emscripten.h>
#endif

void log_immediate(const char* message) {
#ifdef WASM_LOG
    EM_ASM_({console.warn(UTF8ToString($0));}, message);
#else
    printf("%s\n",message);
#endif
}

void log_setup(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), format, args);
    char final_message[550];
    snprintf(final_message, sizeof(final_message), "[status][info]- %s", buffer);
#ifdef WASM_LOG
    EM_ASM_({console.warn(UTF8ToString($0));}, final_message);
#else
    printf("%s\n",final_message);
#endif
}

// WebAssembly entry
void process_file_content(const char* content) {
    if (content == NULL) {
        log_immediate("[status][error]-Error: content is NULL");
        return;
    }
    char preface[100];
    sprintf(preface, "[status][error]-+++++稚%d.%d.%d 献给我的女儿朵朵+++++", ZHI_VERSION_MAJOR, ZHI_VERSION_MINOR, ZHI_VERSION_PATCH);
    log_immediate(preface);
    log_setup("SET UP MAX_CLASS_NESTING:[%d]", MAX_CLASS_NESTING);
    log_setup("SET UP ARRAY_INIT_SIZE:[%d]", ARRAY_INIT_SIZE);
    log_setup("SET UP MAX_CALL_STACK:[%d]", MAX_CALL_STACK);
    log_setup("SET UP INITIAL_GC_SIZE:[%d]", INITIAL_GC_SIZE);
    log_setup("SET UP GC_GROW_FACTOR:[%d]", GC_GROW_FACTOR);
    log_setup("SET UP GC_MAX_SIZE:[%d]", GC_MAX_SIZE);
    log_setup("SET UP GC_CALL_TRIGGER:[%d]", GC_CALL_TRIGGER);
    log_setup("SET UP INIT_METHOD_NAME:[%s]", INIT_METHOD_NAME);


    log_immediate("[status]-Processing content");
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
//        log_immediate("[status][error]-Runtime error.");
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

