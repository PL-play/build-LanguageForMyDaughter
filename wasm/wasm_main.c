//
// Created by ran on 24-10-3.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vm/vm.h"
#include <emscripten.h>
// WebAssembly 入口函数，处理参数或文件内容
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



// 通过 JavaScript 传递参数的接口
EMSCRIPTEN_KEEPALIVE
void handle_file_content(const char* content) {
    process_file_content(content);
}

