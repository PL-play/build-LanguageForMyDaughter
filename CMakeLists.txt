cmake_minimum_required(VERSION 3.16)
project(ZHI C)

find_package(ZCollection REQUIRED)
find_package(ZJSON REQUIRED)
set(CMAKE_INSTALL_PREFIX install)

if (APPLE)
    set(CMAKE_INSTALL_RPATH @loader_path/../lib)
elseif (UNIX)
    set(CMAKE_INSTALL_RPATH \$ORIGIN/../lib)
endif ()
set(CMAKE_C_STANDARD 11)

include_directories("src")

add_library(ZHI STATIC
        src/vm/vm.c src/vm/gc.c
        src/compiler/compiler.c src/compiler/parser.c src/compiler/scanner.c
        src/common/alloc-testing.c src/common/framework.c
        src/chunk/chunk.c src/chunk/debug.c src/chunk/object.c src/chunk/value.c
)
target_link_libraries(ZHI PUBLIC
        m
        ZCollection::ZCollection
        ZJSON::ZJSON
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
