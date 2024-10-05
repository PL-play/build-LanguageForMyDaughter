//
// Created by ran on 2023/12/18.
//

#ifndef ZHI_COMMON_H_
#define ZHI_COMMON_H_
#include "stddef.h"
#include <stdint.h>
#include "hashtable/hash_table_m.h"
#include "list/array_list_m.h"
#include "list/linked_list_m.h"

#define ZHI_VERSION_MAJOR 1
#define ZHI_VERSION_MINOR 0
#define ZHI_VERSION_PATCH 0

#define GROW_ARRAY(type, cap, count, array, init_size, e) do{ \
                                          if ((type)->cap == 0) {\
                                            (type)->cap = init_size;\
                                            (type)->array = malloc(sizeof(e) * (type)->cap);\
                                          } else if ((type)->cap <= (type)->count + 1) {\
                                            (type)->cap += (type)->cap >> 1;\
                                            e *r = realloc((type)->array, sizeof(e) * (type)->cap);\
                                            if (r == NULL) {\
                                              exit(1);\
                                            }\
                                            (type)->array = r;\
                                          }\
                                      }while(0)

#define MAX_INTERPOLATION_NESTING 8

// this keyword literal
#define THIS_LITERAL "this"
#define INIT_METHOD_NAME "init"
#define SUPER_LITERAL "hero"

#define MAX_CLASS_NESTING 8

#define ARRAY_INIT_SIZE 8

// max call stack
#define MAX_CALL_STACK (UINT8_MAX * 64)

// TODO initial size and grow factor is allowed to be configured from compiler
#ifdef INIT_GC
#define INITIAL_GC_SIZE (INIT_GC)
#endif
#ifndef INIT_GC
#define INITIAL_GC_SIZE (1024)
#endif
#define GC_GROW_FACTOR (2)
#ifdef MAX_GC
#define GC_MAX_SIZE (MAX_GC)
#endif
#ifndef MAX_GC
#define GC_MAX_SIZE (1024*1024*50)
#endif

// When user call "__GC()" function, vm treat it as a "suggestion" of garbage zcollection but could be ignored according
// to the GC policy. This controls how many "suggestions" will definitely trigger a GC.
#ifdef GC_TRIGGER
#define GC_CALL_TRIGGER (GC_TRIGGER)
#endif
#ifndef GC_TRIGGER
#define GC_CALL_TRIGGER (4)
#endif

#endif //ZHI_COMMON_H_
