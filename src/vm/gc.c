//
// Created by ran on 2024-05-28.
//
#include "gc.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef ALLOC_TESTING
#include "common/alloc-testing.h"
#endif
#ifdef WASM_LOG
#include <emscripten/emscripten.h>
#endif
#include "chunk/debug.h"
#include "hashtable/hash-pointer.h"
#include "hashtable/compare-pointer.h"

DEFINE_LINKED_LIST(Obj*, NULL, GrayObj)
DEFINE_HASHTABLE(int, 0, MarkedObj)

static void trace_reference(GC *gc);

static void blacken_obj(GC *gc, Obj *obj);

static void mark_value_array(GC *gc, ValueArrayList *);

static void unmark_objects(GC *gc);

static void mark_methods(GC *gc, ValueHashtable *table);

void init_GC(GC *gc) {
    gc->gray_obj = GrayObjnew_linked_list();
    gc->mk_obj = MarkedObjnew_hash_table((MarkedObjHashtableHashFunc) hash_pointer,
                                         (MarkedObjHashtableEqualsFunc) pointer_compare);
    MarkedObjregister_hashtable_free_functions(gc->mk_obj, NULL, NULL);
}

void free_GC(GC *gc) {
    GrayObjfree_linked_list(gc->gray_obj, NULL);
    MarkedObjfree_hash_table(gc->mk_obj);
    free(gc);
}

void mark_value(GC *gc, Value value) {
    if (IS_OBJ(value)) {
        mark_object(gc, AS_OBJ(value));
    }
}

void mark_object(GC *gc, Obj *obj) {
    if (obj == NULL) return;
    if (obj->is_marked) return;
    obj->is_marked = true;
    // add obj to gray list
    GrayObjappend_list(gc->gray_obj, obj);
    // add to mk hashmap
    MarkedObjput_hash_table(gc->mk_obj, obj, 1);
#ifdef DEBUG_GC_LOG
  printf("mark object:  \n");
  printf("   ");
  print_object(obj);
  printf("\n");
#endif
}

// TODO Some roots created during compile time can be cached so no need to mark them every time. However in REPL
//  environment, input source is keep compiling, so I think a flag is needed to tell if marking process can reuse
//  cached objects.
void mark_roots(GC *gc, VM *vm) {
    // vm's stack
#ifdef DEBUG_GC_LOG
  printf("++ mark roots: \n");
  printf("  ++mark VM's stack: \n");
#endif
    for (size_t i = 0; i < vm->stack->size; i++) {
        Value value = Valueget_data_arraylist(vm->stack, i);
        mark_value(gc, value);
    }
#ifdef DEBUG_GC_LOG
  printf("  --mark VM's stack: \n");

  printf("  ++mark global names: \n");
#endif
    // global names, which are ObjString*
    String_HashtableEntrySet *entry = String_hashtable_entry_set(vm->compile_context.global_names);
    for (size_t i = 0; i < entry->size; i++) {
        String_KVEntry *e = entry->entry_set[i];
        mark_object(gc, (Obj *) e->key);
    }
    String_free_hashtable_entry_set(entry);
#ifdef DEBUG_GC_LOG
  printf("  --mark global names: \n");

  printf("  ++mark global values: \n");
#endif
    // global variables
    for (size_t i = 0; i < vm->compile_context.global_values->size; i++) {
        Value value = Valueget_data_arraylist(vm->compile_context.global_values, i);
        mark_value(gc, value);
    }
#ifdef DEBUG_GC_LOG
  printf("  --mark global values: \n");
#endif

#ifdef DEBUG_GC_LOG
  printf("  ++mark class names: \n");
#endif
    for (size_t i = 0; i < vm->compile_context.class_objs->size; i++) {
        Value value = Valueget_data_arraylist(vm->compile_context.class_objs, i);
        mark_value(gc, value);
    }
#ifdef DEBUG_GC_LOG
  printf("  --mark class names: \n");
#endif

#ifdef DEBUG_GC_LOG
  printf("  ++mark functions or closures in call frame: \n");
#endif
    // functions or closures in call frame
    for (size_t i = 0; i < vm->frame_count; ++i) {
        mark_object(gc, vm->frames[i].function);
    }
#ifdef DEBUG_GC_LOG
  printf("  --mark functions or closures in call frame: ");
  printf("  ++mark open upvalue: ");
#endif
    // open upvalue
    for (ObjUpvalue *upvalue = vm->open_upvalues; upvalue != NULL; upvalue = upvalue->next) {
        mark_object(gc, (Obj *) upvalue);
    }
#ifdef DEBUG_GC_LOG
  printf("  --mark open upvalue: \n");

  printf("  ++mark objects created in compiler: \n");
#endif
    // objects created in compiler
    int size = list_size(vm->compile_context.objs);
    if (size > 0) {
        LinkedListNode *node = head_of_list(vm->compile_context.objs);
        do {
            Obj *function = data_of_node_linked_list(node);
            mark_object(gc, function);
            node = next_node_linked_list(node);
            size--;
        } while (size > 0);
    }
#ifdef DEBUG_GC_LOG
  printf("  --mark objects created in compiler: \n");
#endif
}

/**
 *  While there are objects in the gray set:
           - Remove an object from the gray set.
           - Move it to the black set.
           - For each reference the object holds:
               - If the referenced object is white, move it to the gray set.
 * @param gc
 */
static void trace_reference(GC *gc) {
#ifdef DEBUG_GC_LOG
  printf("++trace reference\n");
#endif
    while (gc->gray_obj->size > 0) {
        Obj *obj = GrayObjpeek_list_last(gc->gray_obj);
        GrayObjremove_list_last(gc->gray_obj);
        blacken_obj(gc, obj);
    }
#ifdef DEBUG_GC_LOG
  printf("\n--trace reference\n");
#endif
}

/**
 * Traverse a single object's reference.
 * A black object is any object whose is_marked field is set and that is no longer in the gray list.
 * @param obj
 */
static void blacken_obj(GC *gc, Obj *obj) {
#ifdef DEBUG_GC_LOG
  print_object(obj);
#endif
    switch (obj->type) {
        case OBJ_NATIVE: {
#ifdef DEBUG_GC_LOG
      printf("mark native obj's name\n");
#endif
            mark_object(gc, (Obj *) ((ObjNative *) obj)->name);
            break;
        }
        case OBJ_STRING: break;
        case OBJ_FUNCTION: {
            ObjFunction *function = (ObjFunction *) obj;
            if (function->name != NULL) {
#ifdef DEBUG_GC_LOG
        printf("mark function obj's name\n");
#endif
                mark_object(gc, (Obj *) function->name);
            }
#ifdef DEBUG_GC_LOG
      printf("mark function constants array\n");
#endif
            mark_value_array(gc, function->chunk->constants);
            break;
        }
        case OBJ_UPVALUE: {
            ObjUpvalue *upvalue = (ObjUpvalue *) obj;
            if (upvalue->is_closed) {
#ifdef DEBUG_GC_LOG
        printf("mark closed upvalue\n");
#endif
                mark_value(gc, upvalue->closed);
            }
            break;
        }
        case OBJ_CLOSURE: {
            ObjClosure *closure = (ObjClosure *) obj;
#ifdef DEBUG_GC_LOG
      printf("mark closure's function\n");
#endif
            mark_object(gc, (Obj *) closure->function);
            for (int i = 0; i < closure->upvalue_count; ++i) {
#ifdef DEBUG_GC_LOG
        printf("mark closure's upvalues\n");
#endif
                mark_object(gc, (Obj *) closure->upvalues[i]);
            }
            break;
        }
        case OBJ_CLASS: {
            ObjClass *klass = (ObjClass *) obj;
            if (klass->name != NULL) {
#ifdef DEBUG_GC_LOG
        printf("mark class\n");
#endif
                mark_object(gc, (Obj *) klass->name);
            }
            // mark methods
            mark_methods(gc, klass->methods);
            mark_methods(gc, klass->static_methods);
            break;
        }
        case OBJ_INSTANCE: {
#ifdef DEBUG_GC_LOG
      printf("mark instance\n");
#endif
            // mark class
            ObjInstance *instance = (ObjInstance *) obj;
            mark_object(gc, (Obj *) instance->klass);

            // mark fields
            ValueHashtableEntrySet *entry = Valuehashtable_entry_set(instance->fields);
            for (size_t i = 0; i < entry->size; i++) {
                ValueKVEntry *e = entry->entry_set[i];
                mark_object(gc, (Obj *) e->key);
                mark_value(gc, e->value);
            }
            Valuefree_hashtable_entry_set(entry);
            break;
        }
        case OBJ_INSTANCE_METHOD: {
#ifdef DEBUG_GC_LOG
      printf("mark instance method\n");
#endif
            InstanceMethod *method = (InstanceMethod *) obj;
            mark_object(gc, method->method);
            mark_value(gc, method->receiver);
            break;
        }
        case OBJ_MODULE: {
#ifdef DEBUG_GC_LOG
      printf("mark module\n");
#endif
            ObjModule *module = (ObjModule *) obj;
            mark_object(gc, (Obj *) module->lib);
            String_HashtableEntrySet *entry = String_hashtable_entry_set(module->imports);
            for (size_t i = 0; i < entry->size; i++) {
                String_KVEntry *e = entry->entry_set[i];
                mark_object(gc, (Obj *) e->key);
            }
            String_free_hashtable_entry_set(entry);
            break;
        }

        case OBJ_ARRAY: {
#ifdef DEBUG_GC_LOG
      printf("mark array\n");
#endif
            ObjArray *array = (ObjArray *) obj;
            for (size_t i = 0; i < array->array->size; ++i) {
                mark_value(gc, Valueget_data_arraylist(array->array, i));
            }
            break;
        }
    }
}

static void mark_methods(GC *gc, ValueHashtable *table) {
    ValueHashtableEntrySet *entry = Valuehashtable_entry_set(table);
    for (size_t i = 0; i < entry->size; i++) {
        ValueKVEntry *e = entry->entry_set[i];
        mark_object(gc, (Obj *) e->key);
        mark_value(gc, e->value);
    }
    Valuefree_hashtable_entry_set(entry);
}

static void mark_value_array(GC *gc, ValueArrayList *array_list) {
    for (size_t i = 0; i < array_list->size; ++i) {
        mark_value(gc, Valueget_data_arraylist(array_list, i));
    }
}

void sweep(GC *gc, VM *vm) {
    RTObjHashtableIterator *it = RTObjhashtable_iterator(vm->objects);
    while (RTObjhashtable_iter_has_next(it)) {
        RTObjKVEntry *entry = RTObjhashtable_next_entry(it);
        Obj *obj = RTObjtable_entry_key(entry);
        size_t size = RTObjtable_entry_value(entry);
        if (!obj->is_marked) {
            RTObjremove_hash_table(vm->objects, obj);
#ifdef DEBUG_GC_LOG
      printf("+++++sweep+++++\n");
      print_object(obj);
      printf("\n");
#endif
            free_object(obj);
            deallocate_rt_obj(vm->gc_info, size);
#ifdef DEBUG_GC_LOG
      printf("-----sweep-----\n");
#endif
        }
    }
    RTObjfree_hashtable_iter(it);
}

void sweep_string(GC *gc, VM *vm) {
    String_HashtableIterator *it = String_hashtable_iterator(vm->compile_context.string_intern);
    while (String_hashtable_iter_has_next(it)) {
        ObjString *str = String_table_entry_key(String_hashtable_next_entry(it));
        if (!str->obj.is_marked) {
            String_remove_hash_table(vm->compile_context.string_intern, str);
        }
    }
    String_free_hashtable_iter(it);
}

void collect_garbage(GC *gc, VM *vm) {
#ifdef DEBUG_GC_LOG
  clock_t start, end;
  start = clock();
#endif
#ifdef WASM_LOG
    clock_t start, end;
    start = clock();
    EM_ASM_({
       console.warn(UTF8ToString($0));
   }, "[status]-ZHI GC collecting garbage...");
#endif
    mark_roots(gc, vm);
#ifdef WASM_LOG
    EM_ASM_({
       console.warn(UTF8ToString($0));
   }, "[status]-ZHI GC tracing root...");
#endif
    trace_reference(gc);
#ifdef WASM_LOG
    EM_ASM_({
       console.warn(UTF8ToString($0));
   }, "[status]-ZHI GC sweeping...");
#endif
    sweep_string(gc, vm);
    sweep(gc, vm);
    update_next_gc_size(vm->gc_info);
    unmark_objects(gc);
#ifdef WASM_LOG
    EM_ASM_({
      console.warn(UTF8ToString($0));
  }, "[status]-ZHI GC end.");
    end = clock();
    double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    char ss[100];
    sprintf(ss,"[status]-ZHI GC time: %f ms\n", cpu_time_used * 1000);
    EM_ASM_({
     console.warn(UTF8ToString($0));
 }, ss);
#endif
#ifdef DEBUG_GC_LOG
  end = clock();
  double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  printf("\n\n----------------------------------------\n");
  printf("!!!!!!!!!!GC time: %f ms!!!!!!!!!!!\n", cpu_time_used * 1000);
  printf("----------------------------------------\n\n");
#endif
}

static void unmark_objects(GC *gc) {
    MarkedObjHashtableIterator *it = MarkedObjhashtable_iterator(gc->mk_obj);
    while (MarkedObjhashtable_iter_has_next(it)) {
        Obj *obj = MarkedObjtable_entry_key(MarkedObjhashtable_next_entry(it));
        obj->is_marked = false;
        MarkedObjremove_hash_table(gc->mk_obj, obj);
    }
    MarkedObjfree_hashtable_iter(it);
}
