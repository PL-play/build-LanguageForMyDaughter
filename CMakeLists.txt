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
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
    add_definitions(-DWASM_LOG=ON)
    add_definitions(-DINIT_GC=1024*5)
endif ()
# 递归获取 src 目录下的所有 .c文件
file(GLOB_RECURSE SRC_FILES src/*.c)
set(SRC_FILES ${SRC_FILES})
# 为了在浏览器中模拟模块导入
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
    # 定义输入目录和输出的 C 源文件路径
    set(INPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/wasmzhistd)
    set(OUTPUT_FILE ${CMAKE_BINARY_DIR}/wasmstd.c)

    # 获取指定目录下的所有文件（递归）
    file(GLOB_RECURSE ALL_FILES "${INPUT_DIRECTORY}/*")

    # 开始生成 C 文件
    file(WRITE ${OUTPUT_FILE} "#include <stddef.h>\n#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n\n")
    file(APPEND ${OUTPUT_FILE} "struct WASM_ZHI_FileData {\n    const char* name;\n    const char* base64_content;\n    size_t size;\n};\n\n")

    file(APPEND ${OUTPUT_FILE} "struct WASM_ZHI_FileData wasm_std_file_data[] = {\n")

    # 遍历所有文件
    foreach (FILE_PATH ${ALL_FILES})
        # 获取相对路径
        file(RELATIVE_PATH REL_PATH ${INPUT_DIRECTORY} ${FILE_PATH})

        # 使用 Python 脚本对文件进行 Base64 编码，并将结果写入 C 文件
        execute_process(
                COMMAND python3 -c "
import base64
with open('${FILE_PATH}', 'rb') as f:
    content = f.read()
    encoded = base64.b64encode(content).decode('utf-8')
    print(f'    {{\"${REL_PATH}\", \"' + encoded + '\", ' + str(len(content)) + ' },')
"
                OUTPUT_VARIABLE BASE64_CONTENT
        )

        # 将 Base64 编码的内容添加到 C 文件
        file(APPEND ${OUTPUT_FILE} "${BASE64_CONTENT}")
    endforeach ()

    # 数组结束
    file(APPEND ${OUTPUT_FILE} "    {NULL, NULL, 0} // 数组结束\n};\n")

    # 将生成的文件添加到项目的源文件列表中
    set(SRC_FILES ${SRC_FILES} ${OUTPUT_FILE})
endif ()
# 添加静态或共享库
add_library(zhi_core ${SRC_FILES})


target_link_libraries(zhi_core PUBLIC
        m
        #        ZCollection::ZCollection # find_package(ZCollection REQUIRED)
        #        ZJSON::ZJSON  #find_package(ZJSON REQUIRED)
        #        /usr/local/lib/libZCollection.a #手动连接库
        #        /usr/local/lib/libZJSON.a
)


# 测试用例，非emcmake时测试时开启
if (NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
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

if (NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
    # 添加 main.c 作为可执行程序
    add_executable(ZHI zhi/main.c)

    # 将 ZHI 库与可执行程序链接
    target_link_libraries(ZHI PRIVATE zhi_core)

    # 安装可执行程序
    install(TARGETS ZHI RUNTIME DESTINATION bin)
endif ()

# 检测是否使用 Emscripten
# 单独为 WebAssembly 设置目标
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
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
endif ()



