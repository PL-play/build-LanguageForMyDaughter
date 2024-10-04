//
// Created by ran on 2023/12/18.
//
#include <stdio.h>
#include <stdlib.h>
#include "vm/vm.h"
#include <unistd.h>
#include <limits.h>

static void repl(VM *vm) {
    char line[10240];
    for (;;) {
        printf(">  ");
        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }
        interpret(vm, NULL, line);
        printf("\n");
        fflush(stdout);
    }
}

static char *read_file(const char *fp) {
    FILE *file = fopen(fp, "rb");

    if (file == NULL) {
        fprintf(stderr, "Can't open file \"%s\".\n", fp);
        exit(74);
    }

    fseek(file, 0L, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);

    char *buffer = malloc(file_size + 1);

    size_t read_len = fread(buffer, sizeof(char), file_size, file);
    if (read_len < file_size) {
        fprintf(stderr, "Can't read file \"%s\".\n", fp);
        exit(74);
    }

    buffer[read_len] = '\0';

    fclose(file);
    return buffer;
}

static void run_file(VM *vm, const char *fp) {
    char *source = read_file(fp);
    // 获取文件的真实全路径
    char real_path[PATH_MAX];
    realpath(fp, real_path);
    InterpretResult result = interpret(vm, real_path, source);
    free(source);
    if (result == INTERPRET_COMPILE_ERROR) exit(65);
    if (result == INTERPRET_RUNTIME_ERROR) exit(70);
}

int main(int argc, char *argv[]) {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        // printf("Current working directory: %s\n", cwd);
    } else {
        perror("getcwd() error");
    }
    fflush(stdout);
    VM vm;
    init_VM(&vm,NULL,NULL,true);
    if (argc == 1) {
        repl(&vm);
    } else if (argc == 2) {
        run_file(&vm, argv[1]);
    } else {
        fprintf(stderr, "Usage: ZHI [path]\n");
        exit(64);
    }
    free_VM(&vm);
    return 0;
}
