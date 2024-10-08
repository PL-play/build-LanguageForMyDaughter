//
// Created by ran on 2024/7/2.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common/framework.h"

// Define a macro to check for the Windows environment
#if defined(_WIN32) || defined(_WIN64)
#include <limits.h>
#define GET_ABSOLUTE_PATH(relative_path, absolute_path) \
        _fullpath(absolute_path, relative_path, _MAX_PATH)
#else
#include <limits.h>
#define GET_ABSOLUTE_PATH(relative_path, absolute_path) \
        realpath(relative_path, absolute_path)
#endif


// Helper macros to count the number of arguments
#define COUNT_ARGS(...) COUNT_ARGS_IMPL(__VA_ARGS__, 5, 4, 3, 2, 1, 0)
#define COUNT_ARGS_IMPL(_1, _2, _3, _4, _5, count, ...) count

static void count_arg() {
  printf("%d\n", COUNT_ARGS(a, b, c));       // Output: 3
  printf("%d\n", COUNT_ARGS(1, 2));          // Output: 2
  printf("%d\n", COUNT_ARGS(42));            // Output: 1
  printf("%d\n", COUNT_ARGS());              // Output: 0
}

static void file_path() {
  char *relative_path = "../lib.duo";
  char absolute_path[PATH_MAX];

  // Get the absolute path
  if (GET_ABSOLUTE_PATH(relative_path, absolute_path) == NULL) {
    perror("GET_ABSOLUTE_PATH");
  }

  // Print the absolute path
  printf("Absolute path: %s\n", absolute_path);
}

static void get_parent_directory(const char *path, char *parent_dir) {
  // Find the last occurrence of '/' or '\'
  const char *last_slash = strrchr(path, '/');
  const char *last_backslash = strrchr(path, '\\');

  // Determine the position of the last path separator
  const char *last_separator = last_slash > last_backslash ? last_slash : last_backslash;

  if (last_separator != NULL) {
    // Calculate the length of the parent directory path
    size_t parent_length = last_separator - path;

    // Copy the parent directory path to the output buffer
    strncpy(parent_dir, path, parent_length);
    parent_dir[parent_length] = '\0'; // Null-terminate the string
  } else {
    // If no separator is found, the path is the root or invalid
    strcpy(parent_dir, ""); // Return an empty string
  }
}

void resolve_absolute_path(const char *current_dir, const char *relative_path, char *absolute_path) {
  char stack[256][256]; // Stack to hold directory names
  int top = -1; // Initialize stack pointer

  // Split the current directory into components and push onto the stack
  char temp_dir[1024];
  strcpy(temp_dir, current_dir);

  char *token = strtok(temp_dir, "/\\");
  while (token != NULL) {
    strcpy(stack[++top], token);
    token = strtok(NULL, "/\\");
  }

  // Process the relative path
  const char *ptr = relative_path;
  while (*ptr) {
    if (*ptr == '.' && (*(ptr + 1) == '/' || *(ptr + 1) == '\\' || *(ptr + 1) == '\0')) {
      // Current directory (.)
      ptr++;
      if (*ptr == '/' || *ptr == '\\') {
        ptr++;
      }
    } else if (*ptr == '.' && *(ptr + 1) == '.' && (*(ptr + 2) == '/' || *(ptr + 2) == '\\' || *(ptr + 2) == '\0')) {
      // Parent directory (..)
      if (top >= 0) {
        top--;
      }
      ptr += 2;
      if (*ptr == '/' || *ptr == '\\') {
        ptr++;
      }
    } else {
      // Directory name
      const char *start = ptr;
      while (*ptr && *ptr != '/' && *ptr != '\\') {
        ptr++;
      }
      int length = ptr - start;
      strncpy(stack[++top], start, length);
      stack[top][length] = '\0';
      if (*ptr == '/' || *ptr == '\\') {
        ptr++;
      }
    }
  }

  // Construct the absolute path from the stack
  absolute_path[0] = '\0';
  if (current_dir[0] == '/' || current_dir[1] == ':') {
    if (current_dir[1] == ':') {
      // Windows absolute path
      strncat(absolute_path, current_dir, 2);
      strcat(absolute_path, "\\");
    } else {
      strcat(absolute_path, "/");
    }
  }

  for (int i = 0; i <= top; i++) {
    strcat(absolute_path, stack[i]);
    if (i < top) {
      strcat(absolute_path, "/");
    }
  }
}

static void test_get_parent() {
  char path1[] = "/f/g/a/g";
  char path2[] = "F:\\a\\b\\d";
  char parent_dir[256]; // Ensure this buffer is large enough for your paths

  get_parent_directory(path1, parent_dir);
  printf("Parent directory of '%s': %s\n", path1, parent_dir);

  get_parent_directory(path2, parent_dir);
  printf("Parent directory of '%s': %s\n", path2, parent_dir);

}

static void test_resolve_path() {
  char current_dir1[] = "/a/f/g";
  char relative_path1[] = "./c/a/f";
  char relative_path2[] = "c/sf/";
  char relative_path3[] = "../ce/a/f";

  char current_dir2[] = "C:\\a\\f\\g";
  char relative_path4[] = ".\\c";
  char relative_path5[] = "c";
  char relative_path6[] = "..\\c";

  char absolute_path[1024];

  // Unix-style paths
  resolve_absolute_path(current_dir1, relative_path1, absolute_path);
  printf("Absolute path: %s\n", absolute_path);

  resolve_absolute_path(current_dir1, relative_path2, absolute_path);
  printf("Absolute path: %s\n", absolute_path);

  resolve_absolute_path(current_dir1, relative_path3, absolute_path);
  printf("Absolute path: %s\n", absolute_path);

  // Windows-style paths
  resolve_absolute_path(current_dir2, relative_path4, absolute_path);
  printf("Absolute path: %s\n", absolute_path);

  resolve_absolute_path(current_dir2, relative_path5, absolute_path);
  printf("Absolute path: %s\n", absolute_path);

  resolve_absolute_path(current_dir2, relative_path6, absolute_path);
  printf("Absolute path: %s\n", absolute_path);

}

static void test_real_path(){
  // Current directory
  char *currentDir = "/mnt/f/work/c/clox/duoduolib";
  // Relative path from the current directory
  char *relativePath = "../lib.duo";

  // Buffer to hold the absolute path
  char absolutePath[PATH_MAX];

  // Construct the full path to resolve
  char fullPath[PATH_MAX];
  snprintf(fullPath, sizeof(fullPath), "%s/%s", currentDir, relativePath);

  // Resolve the full path to an absolute path
  char *resolvedPath = GET_ABSOLUTE_PATH(fullPath, absolutePath);

  if (resolvedPath) {
    printf("Resolved Absolute Path: %s\n", resolvedPath);
  } else {
    perror("Error resolving path");
  }

}



static void test_print() {
  printf("| |             | |               | |                              | |                     | |\n");
  printf("| |__     __ _  | |__     __ _    | |   ___   __   __   ___      __| |  _   _    ___     __| |  _   _    ___  \n");
  printf("| '_ \\   / _` | | '_ \\   / _` |   | |  / _ \\  \\ \\ / /  / _ \\    / _` | | | | |  / _ \\   / _` | | | | |  / _ \\ \n");
  printf("| |_) | | (_| | | |_) | | (_| |   | | | (_) |  \\ V /  |  __/   | (_| | | |_| | | (_) | | (_| | | |_| | | (_) |\n");
  printf("|_.__/   \\__,_| |_.__/   \\__,_|   |_|  \\___/    \\_/    \\___|    \\__,_|  \\__,_|  \\___/   \\__,_|  \\__,_|  \\___/ \n");
  printf("\n");
  printf("   / /_   ____ _   / /_   ____ _          / /  ____  _   __  ___          ____/ /  __  __  ____   ____/ /  __  __  ____ \n");
  printf("  / __ \\ / __ `/  / __ \\ / __ `/         / /  / __ \\| | / / / _ \\        / __  /  / / / / / __ \\ / __  /  / / / / / __ \\\n");
  printf(" / /_/ // /_/ /  / /_/ // /_/ /         / /  / /_/ /| |/ / /  __/       / /_/ /  / /_/ / / /_/ // /_/ /  / /_/ / / /_/ /\n");
  printf("/_.___/ \\__,_/  /_.___/ \\__,_/         /_/   \\____/ |___/  \\___/        \\__,_/   \\__,_/  \\____/ \\__,_/   \\__,_/  \\____/ \n");

  printf("\n");
    printf(" ▄▄▄▄▄▄▄▄▄▄   ▄▄▄▄▄▄▄▄▄▄▄  ▄▄▄▄▄▄▄▄▄▄   ▄▄▄▄▄▄▄▄▄▄▄     \n");
    printf("▐░░░░░░░░░░▌ ▐░░░░░░░░░░░▌▐░░░░░░░░░░▌ ▐░░░░░░░░░░░▌     \n");
    printf("▐░█▀▀▀▀▀▀▀█░▌▐░█▀▀▀▀▀▀▀█░▌▐░█▀▀▀▀▀▀▀█░▌▐░█▀▀▀▀▀▀▀█░▌     \n");
    printf("▐░▌       ▐░▌▐░▌       ▐░▌▐░▌       ▐░▌▐░▌       ▐░▌     \n");
    printf("▐░█▄▄▄▄▄▄▄█░▌▐░█▄▄▄▄▄▄▄█░▌▐░█▄▄▄▄▄▄▄█░▌▐░█▄▄▄▄▄▄▄█░▌    \n");
    printf("▐░░░░░░░░░░▌ ▐░░░░░░░░░░░▌▐░░░░░░░░░░▌ ▐░░░░░░░░░░░▌    \n");
    printf("▐░█▀▀▀▀▀▀▀█░▌▐░█▀▀▀▀▀▀▀█░▌▐░█▀▀▀▀▀▀▀█░▌▐░█▀▀▀▀▀▀▀█░▌     \n");
    printf("▐░▌       ▐░▌▐░▌       ▐░▌▐░▌       ▐░▌▐░▌       ▐░▌     \n");
    printf("▐░█▄▄▄▄▄▄▄█░▌▐░▌       ▐░▌▐░█▄▄▄▄▄▄▄█░▌▐░▌       ▐░▌     \n");
    printf("▐░░░░░░░░░░▌ ▐░▌       ▐░▌▐░░░░░░░░░░▌ ▐░▌       ▐░▌     \n");
    printf(" ▀▀▀▀▀▀▀▀▀▀   ▀         ▀  ▀▀▀▀▀▀▀▀▀▀   ▀         ▀      \n");


}


static UnitTestFunction tests[] = {
    // count_arg,
    // file_path,
    // test_get_parent,
    // test_resolve_path,
    // test_real_path,
  test_print,
    NULL
};

int main(int argc, char *argv[]) {
  setbuf(stdout, NULL);
  run_tests(tests);
  return 0;
}