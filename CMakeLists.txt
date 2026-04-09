cmake_minimum_required(VERSION 3.24)

project(Deadliner LANGUAGES CXX)
set(PROJECT_VERSION "0.0.0.1.2a")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

include(GNUInstallDirs)
include(CTest)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets Sql Test LinguistTools)
if(UNIX AND NOT APPLE)
    find_package(Qt6 6.5 REQUIRED COMPONENTS DBus)
endif()

set(DEADLINER_TS_FILES
    resources/i18n/deadliner_en.ts
    resources/i18n/deadliner_ru.ts
)

set(DEADLINER_RESOURCES
    resources/icons.qrc
)

add_library(deadliner_core
    src/domain/models.h
    src/domain/logic.h
    src/domain/logic.cpp
    src/infrastructure/logging/logging.h
    src/infrastructure/logging/logging.cpp
    src/infrastructure/settings/settings_store.h
    src/infrastructure/settings/settings_store.cpp
    src/infrastructure/storage/repository.h
    src/infrastructure/storage/repository.cpp
    src/infrastructure/platform/autostart_manager.h
    src/infrastructure/platform/autostart_manager.cpp
    src/application/services.h
    src/application/services.cpp
    src/ui/reminder_dialog.h
    src/ui/reminder_dialog.cpp
    src/ui/break_window.h
    src/ui/break_window.cpp
    src/ui/common/navigation.h
    src/ui/common/display_strings.h
    src/ui/common/display_strings.cpp
    src/ui/event_dialog.h
    src/ui/event_dialog.cpp
    src/ui/events/events_page.h
    src/ui/events/events_page.cpp
    src/ui/i18n/language_manager.h
    src/ui/i18n/language_manager.cpp
    src/ui/profile_dialog.h
    src/ui/profile_dialog.cpp
    src/ui/main_window.h
    src/ui/main_window.cpp
    src/ui/overdue/overdue_page.h
    src/ui/overdue/overdue_page.cpp
    src/ui/onboarding/onboarding_dialog.h
    src/ui/onboarding/onboarding_dialog.cpp
    src/ui/profiles/profiles_page.h
    src/ui/profiles/profiles_page.cpp
    src/ui/settings/settings_page.h
    src/ui/settings/settings_page.cpp
    src/ui/settings/theme_manager.h
    src/ui/settings/theme_manager.cpp
    src/ui/statistics/statistics_page.h
    src/ui/statistics/statistics_page.cpp
    src/ui/today/today_page.h
    src/ui/today/today_page.cpp
    src/ui/tray_controller.h
    src/ui/tray_controller.cpp
    src/ui/widgets/empty_state_widget.h
    src/ui/widgets/empty_state_widget.cpp
    src/app/app_controller.h
    src/app/app_controller.cpp
    ${DEADLINER_RESOURCES}
)

target_include_directories(deadliner_core PUBLIC src)

target_link_libraries(deadliner_core
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Sql
)

if(UNIX AND NOT APPLE)
    target_link_libraries(deadliner_core PUBLIC Qt6::DBus)
endif()

qt_add_translations(deadliner_core
    TS_FILES ${DEADLINER_TS_FILES}
)

qt_add_executable(deadliner
    src/main.cpp
)

target_link_libraries(deadliner PRIVATE deadliner_core)

install(TARGETS deadliner
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    BUNDLE DESTINATION .
)

install(FILES packaging/linux/deadliner.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications OPTIONAL)
install(FILES packaging/linux/deadliner.png DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps OPTIONAL)

set(CPACK_PACKAGE_NAME "Deadliner")
set(CPACK_PACKAGE_VENDOR "Deadliner")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_FILE_NAME "deadliner-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}")
set(CPACK_GENERATOR "ZIP;TGZ")
include(CPack)

if(BUILD_TESTING)
    add_executable(deadliner_domain_tests
        tests/test_domain.cpp
    )

    target_link_libraries(deadliner_domain_tests
        PRIVATE
            deadliner_core
            Qt6::Test
    )

    add_executable(deadliner_repository_tests
        tests/test_repository.cpp
    )

    add_executable(deadliner_scheduler_tests
        tests/test_scheduler.cpp
    )

    target_link_libraries(deadliner_repository_tests
        PRIVATE
            deadliner_core
            Qt6::Test
    )

    target_link_libraries(deadliner_scheduler_tests
        PRIVATE
            deadliner_core
            Qt6::Test
    )

    add_test(NAME deadliner_domain_tests COMMAND deadliner_domain_tests)
    add_test(NAME deadliner_repository_tests COMMAND deadliner_repository_tests)
    add_test(NAME deadliner_scheduler_tests COMMAND deadliner_scheduler_tests)
endif()
