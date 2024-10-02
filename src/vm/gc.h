//
// Created by ran on 2024-05-28.
//

#ifndef ZHI_VM_GC_H_
#define ZHI_VM_GC_H_

#include "vm.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
    Tricolor Abstraction in Garbage Collection (GC)

    The tricolor abstraction is a method used in garbage collection algorithms, particularly in tracing garbage
    collectors, to keep track of which objects have been visited during the collection process. This method helps
    in efficiently identifying live objects and collecting garbage. The objects are divided into three sets,
    represented by colors:

      - White: Objects that have not been visited yet.
      - Gray: Objects that have been visited, but their children (i.e., objects they reference)
              have not been fully processed.
      - Black: Objects that have been fully processed (i.e., they and all objects they reference
              have been visited).

    Algorithm Overview
    The algorithm generally works as follows:

    1. Initialization:
      - Start with all objects in the white set.
      - Move the root objects (starting points of the program) to the gray set.

    2. Tracing:
      - While there are objects in the gray set:
           - Remove an object from the gray set.
           - Move it to the black set.
           - For each reference the object holds:
               - If the referenced object is white, move it to the gray set.

    3. Collection:
       - After all gray objects are processed (i.e., the gray set is empty), all remaining white objects
         are considered unreachable and can be collected as garbage.
 */

DECLARE_LINKED_LIST(Obj*, GrayObj)

DECLARE_HASHTABLE(Obj*, int, MarkedObj)

// Garbage Collector
typedef struct GC {
 GrayObjLinkedList *gray_obj;
 MarkedObjHashtable *mk_obj;
} GC;

/**
 * init gray obj list
 *
 * @param gc
 */
void init_GC(GC *gc);

void free_GC(GC *gc);

void mark_value(GC *gc, Value value);

void mark_object(GC *gc, Obj *obj);

/**
 * Some sources of root:
 *  1. Values in VM's stack
 *  2. Global variables
 *  3. Function or Closure objects in VM's CallFrame
 *  4. Open UpValues of VM
 *  5. Objects created in compiler
 *
 * Among these roots there are objects that created in compile time and remain in runtime, so we can
 * cache them and not marking roots of them.
 *
 * The changing roots are
 *   1. values in VM's stack;
 *   2. function or closure objects in VM's call frame;
 *   3. upvalues of VM.
 *
 * @param stack
 */
void mark_roots(GC *gc, VM *vm);

/**
 * Sweep white objects after marking
 *
 * @param gc
 * @param vm
 */
void sweep(GC *gc, VM *vm);

/**
 * sweep String objects in String interning hashtable.
 *
 * @param gc
 * @param vm
 */
void sweep_string(GC *gc, VM *vm);

/**
 * GC
 *
 * @param gc
 * @param vm
 */
void collect_garbage(GC *gc, VM *vm);
#ifdef __cplusplus
}
#endif
#endif //ZHI_VM_GC_H_
