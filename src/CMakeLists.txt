add_subdirectory(ui)

# Mock executable to verify UI compilation
add_executable(Deadliner
    main.cpp
)

target_link_libraries(Deadliner PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets ui_lib)
