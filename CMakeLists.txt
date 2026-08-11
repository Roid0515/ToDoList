cmake_minimum_required(VERSION 3.20)
project(PersonalTodo VERSION 2.0.0 LANGUAGES C RC)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(TodoList WIN32
    src/main.c
    src/app.c
    src/ui_main.c
    src/ui_controls.c
    src/ui_layout.c
    src/ui_tasks.c
    src/ui_settings.c
    src/database.c
    src/task.c
    src/paths.c
    src/settings.c
    src/utf.c
    src/log.c
    third_party/sqlite/sqlite3.c
    resources/app.rc
)

target_include_directories(TodoList PRIVATE src third_party/sqlite resources)
target_compile_definitions(TodoList PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX
    SQLITE_THREADSAFE=0 SQLITE_OMIT_LOAD_EXTENSION)
target_link_libraries(TodoList PRIVATE user32 gdi32 comctl32 shell32 ole32 uuid)

if(MSVC)
    target_compile_options(TodoList PRIVATE /W4 /utf-8 /permissive- /sdl)
    set_source_files_properties(third_party/sqlite/sqlite3.c PROPERTIES COMPILE_OPTIONS "/W0")
    set_property(TARGET TodoList PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
