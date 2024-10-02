cmake_minimum_required(VERSION 3.16)
project(ZHI C)

#find_package(ZCollection REQUIRED)
#find_package(ZJSON REQUIRED)
set(CMAKE_INSTALL_PREFIX install)

if (APPLE)
    set(CMAKE_INSTALL_RPATH @loader_path/../lib)
elseif (UNIX)
    set(CMAKE_INSTALL_RPATH \$ORIGIN/../lib)
endif ()
set(CMAKE_C_STANDARD 11)

include_directories("src")
include_directories("src/zcollection/include")
include_directories("src/zjson/include")
# 检测是否使用 Emscripten
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")  # 生成 HTML 文件
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s WASM=1 -s ALLOW_MEMORY_GROWTH=1")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_ZLIB=1 -s USE_LIBPNG=1")
endif ()

add_library(ZHI STATIC
        src/vm/vm.c src/vm/gc.c
        src/compiler/compiler.c src/compiler/parser.c src/compiler/scanner.c
        src/common/alloc-testing.c src/common/framework.c
        src/chunk/chunk.c src/chunk/debug.c src/chunk/object.c src/chunk/value.c
        # 如果设置find_package(ZCollection REQUIRED)和find_package(ZJSON REQUIRED)，或手动连接/usr/local/lib/libZCollection.a和/usr/local/lib/libZJSON.a，则不需要
        # 手动设置zcollection和zjson
        src/zcollection/src/list/array_list.c src/zcollection/src/list/linked_list.c src/zcollection/src/tree/binary_search_tree.c
        src/zcollection/src/heap/binary_heap.c
        src/zcollection/src/tree/avl_tree.c src/zcollection/src/tree/rb_tree.c src/zcollection/src/stack/stack_al.c src/zcollection/src/tree/trie.c
        src/zcollection/src/hashtable/hash_table.c src/zcollection/src/hashtable/hash-int.c
        src/zcollection/src/hashtable/hash-pointer.c src/zcollection/src/hashtable/hash-string.c src/zcollection/src/hashtable/compare-int.c
        src/zcollection/src/hashtable/compare-pointer.c
        src/zcollection/src/hashtable/compare-string.c src/zcollection/src/hashtable/hash_set.c src/zcollection/src/tree/b_tree.c src/zcollection/src/queue/dqueue_ll.c
        src/zcollection/src/stack/stack_ll.c
        src/zcollection/src/tree/bp_tree_indisk.c src/zcollection/src/tree/b_tree2.c src/zcollection/src/unionfind/union_find.c src/zcollection/src/queue/priority_queue.c
        src/zcollection/src/hashtable/hash-double.c
        src/zcollection/src/hashtable/sha256.c

        src/zjson/src/zjson.c
)

target_link_libraries(ZHI PUBLIC
        m
        #        ZCollection::ZCollection
        #        ZJSON::ZJSON
        #        /usr/local/lib/libZCollection.a #手动连接库
        #        /usr/local/lib/libZJSON.a
)

# tests
file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/test/*.c")
foreach (file ${files})
    get_filename_component(name ${file} NAME_WE)
    add_executable(
            ${name} ${file}
    )
    # Option to enable ALLOC_TESTING
    option(ENABLE_ALLOC_TESTING "Enable allocation testing" OFF)
    # Define ALLOC_TESTING if the option is enabled
    if (ENABLE_ALLOC_TESTING)
        add_definitions(-DALLOC_TESTING=ON)
    endif ()

    option(ENABLE_DEBUG_TRACE "Enable debugging trace execution" OFF)

    if (ENABLE_DEBUG_TRACE)
        # Define the DEBUG_TRACE_EXECUTION macro
        add_definitions(-DDEBUG_TRACE_EXECUTION=ON)
    endif ()
    if (ENABLE_GC_DEBUG)
        add_definitions(-DDEBUG_GC_LOG=${ENABLE_GC_DEBUG})
    endif ()
    if (INIT_GC)
        add_definitions(-DINIT_GC=${INIT_GC})
    endif ()
    if (GC_TRIGGER)
        add_definitions(-DGC_TRIGGER=${GC_TRIGGER})
    endif ()

    target_link_libraries(${name} ZHI)
    install(TARGETS ${name} RUNTIME DESTINATION bin)
endforeach ()

# 添加 main.c 作为可执行程序
add_executable(ZHI_executable src/main.c)

# 将 ZHI 库与可执行程序链接
target_link_libraries(ZHI_executable PRIVATE ZHI)

# 安装可执行程序
install(TARGETS ZHI_executable RUNTIME DESTINATION bin)
