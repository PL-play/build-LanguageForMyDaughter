cmake_minimum_required(VERSION 3.16)
project(ZHILanguage C)

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

add_library(zhi_core STATIC
        src/vm/vm.c src/vm/gc.c
        src/compiler/compiler.c src/compiler/parser.c src/compiler/scanner.c
        src/common/alloc-testing.c src/common/framework.c
        src/chunk/chunk.c src/chunk/debug.c src/chunk/object.c src/chunk/value.c
        # 如果设置find_package(ZCollection REQUIRED)和find_package(ZJSON REQUIRED)，
        # 或手动连接/usr/local/lib/libZCollection.a和/usr/local/lib/libZJSON.a，则不需要
        # 手动设置zcollection和zjson
        src/zcollection/src/list/array_list.c src/zcollection/src/list/linked_list.c
        src/zcollection/src/tree/binary_search_tree.c
        src/zcollection/src/heap/binary_heap.c
        src/zcollection/src/tree/avl_tree.c src/zcollection/src/tree/rb_tree.c
        src/zcollection/src/stack/stack_al.c src/zcollection/src/tree/trie.c
        src/zcollection/src/hashtable/hash_table.c src/zcollection/src/hashtable/hash-int.c
        src/zcollection/src/hashtable/hash-pointer.c src/zcollection/src/hashtable/hash-string.c
        src/zcollection/src/hashtable/compare-int.c
        src/zcollection/src/hashtable/compare-pointer.c
        src/zcollection/src/hashtable/compare-string.c src/zcollection/src/hashtable/hash_set.c
        src/zcollection/src/tree/b_tree.c src/zcollection/src/queue/dqueue_ll.c
        src/zcollection/src/stack/stack_ll.c
        src/zcollection/src/tree/bp_tree_indisk.c src/zcollection/src/tree/b_tree2.c
        src/zcollection/src/unionfind/union_find.c src/zcollection/src/queue/priority_queue.c
        src/zcollection/src/hashtable/hash-double.c
        src/zcollection/src/hashtable/sha256.c

        src/zjson/src/zjson.c
)

target_link_libraries(zhi_core PUBLIC
        m
        #        ZCollection::ZCollection # find_package(ZCollection REQUIRED)
        #        ZJSON::ZJSON  #find_package(ZJSON REQUIRED)
        #        /usr/local/lib/libZCollection.a #手动连接库
        #        /usr/local/lib/libZJSON.a
)
# 测试用例，非emcmake时测试时开启
if(NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
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

        target_link_libraries(${name} zhi_core)
    endforeach ()

    # test collection
    file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/test/test_collection/*.c")
    foreach (file ${files})
        get_filename_component(name ${file} NAME_WE)
        add_executable(
                ${name} ${file}
        )
        target_link_libraries(${name} zhi_core)
    endforeach ()
    # test json
    file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/test/test_json/*.c")
    foreach (file ${files})
        get_filename_component(name ${file} NAME_WE)
        add_executable(
                ${name} ${file}
        )
        target_link_libraries(${name} zhi_core)
    endforeach ()
endif ()

if(NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
    # 添加 main.c 作为可执行程序
    add_executable(ZHI src/main.c)

    # 将 ZHI 库与可执行程序链接
    target_link_libraries(ZHI PRIVATE zhi_core)

    # 安装可执行程序
    install(TARGETS ZHI RUNTIME DESTINATION bin)
endif ()

# 检测是否使用 Emscripten
# 单独为 WebAssembly 设置目标
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
    # 为 Wasm 构建使用不同的入口文件
    add_executable(ZHI_wasm wasm/wasm_main.c)

    # 链接 ZHI 库
    target_link_libraries(ZHI_wasm PRIVATE zhi_core)

    # 设置输出文件的后缀名为 .js
    set_target_properties(ZHI_wasm PROPERTIES SUFFIX ".js")

    # 设置编译选项
    target_compile_options(ZHI_wasm PRIVATE
            -O2
            -sALLOW_MEMORY_GROWTH=1
            -sSAFE_HEAP=1
            -sASSERTIONS=2
            -gsource-map #在编译时启用 Source Maps，可以在浏览器中查看 C 代码的源文件和行号
    )
    # 设置链接选项，确保参数正确传递
    target_link_options(ZHI_wasm PRIVATE
            -O2
            -sALLOW_MEMORY_GROWTH=1
            -sSAFE_HEAP=1
            -sASSERTIONS=2
            "-sSTACK_SIZE=5242880"  # 设置栈大小为 5MB
            "-sEXPORTED_FUNCTIONS=[_malloc,_free,_handle_file_content]"
            "-sEXPORTED_RUNTIME_METHODS=[ccall,cwrap]"
            --no-entry
            # --source-map-base http://localhost:8000/ZHI_wasm.wasm #启用 Emscripten 的日志功能，获取更多调试信息
    )

    # 安装生成的文件
    install(TARGETS ZHI_wasm RUNTIME DESTINATION bin)
endif()



