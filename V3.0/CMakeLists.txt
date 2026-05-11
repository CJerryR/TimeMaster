cmake_minimum_required(VERSION 3.16)
project(TimeMaster VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Default build type: Release if not specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# UTF-8 source on MSVC (Chinese strings)
if(MSVC)
    add_compile_options(/utf-8 /Zc:__cplusplus /permissive-)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Sql)

qt_standard_project_setup()

set(APP_SOURCES
    src/main.cpp

    # Core
    src/core/Types.h
    src/core/Types.cpp
    src/core/Database.h
    src/core/Database.cpp
    src/core/DeepSeekClient.h
    src/core/DeepSeekClient.cpp

    # UI (top-level)
    src/ui/Theme.h
    src/ui/Theme.cpp
    src/ui/MainWindow.h
    src/ui/MainWindow.cpp
    src/ui/Sidebar.h
    src/ui/Sidebar.cpp
    src/ui/CalendarPage.h
    src/ui/CalendarPage.cpp
    src/ui/AnalyticsPage.h
    src/ui/AnalyticsPage.cpp
    src/ui/ChatPage.h
    src/ui/ChatPage.cpp
    src/ui/MonthView.h
    src/ui/MonthView.cpp
    src/ui/TimeGridView.h
    src/ui/TimeGridView.cpp
    src/ui/EventDialog.h
    src/ui/EventDialog.cpp
    src/ui/SettingsDialog.h
    src/ui/SettingsDialog.cpp
    src/ui/AiHistoryDialog.h
    src/ui/AiHistoryDialog.cpp
    src/ui/CategoryPieChart.h
    src/ui/CategoryPieChart.cpp

    # Widgets
    src/ui/widgets/StatsCardsWidget.h
    src/ui/widgets/StatsCardsWidget.cpp
    src/ui/widgets/HorizontalBarChart.h
    src/ui/widgets/HorizontalBarChart.cpp
    src/ui/widgets/DailyTrendChart.h
    src/ui/widgets/DailyTrendChart.cpp
    src/ui/widgets/RhythmCardWidget.h
    src/ui/widgets/RhythmCardWidget.cpp
    src/ui/widgets/SourceDistributionWidget.h
    src/ui/widgets/SourceDistributionWidget.cpp
    src/ui/widgets/InsightsWidget.h
    src/ui/widgets/InsightsWidget.cpp
)

qt_add_executable(TimeMaster WIN32 MACOSX_BUNDLE
    ${APP_SOURCES}
)

target_link_libraries(TimeMaster
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Network
        Qt6::Sql
)

target_include_directories(TimeMaster PRIVATE src)

# Windows specifics
if(WIN32)
    set_target_properties(TimeMaster PROPERTIES
        WIN32_EXECUTABLE TRUE
        OUTPUT_NAME "TimeMaster"
    )
endif()

# macOS bundle metadata (kept for cross-platform)
if(APPLE)
    set_target_properties(TimeMaster PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "TimeMaster"
        MACOSX_BUNDLE_GUI_IDENTIFIER "local.timemaster.app"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    )
endif()

# Convenience: print summary
message(STATUS "TimeMaster ${PROJECT_VERSION} configured (Qt ${Qt6_VERSION})")
