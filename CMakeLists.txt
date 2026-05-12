cmake_minimum_required(VERSION 3.16)
project(TimeMaster VERSION 4.3.1 LANGUAGES CXX)

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
    src/core/I18n.h
    src/core/I18n.cpp

    # Utils
    src/utils/MarkdownToHtml.h
    src/utils/MarkdownToHtml.cpp

    # V4.3 — global runtime preferences (week-start, AI permissions)
    src/core/Preferences.h
    src/core/Preferences.cpp

    # UI infra (字体 + 自绘图标)
    src/ui/FontLoader.h
    src/ui/FontLoader.cpp
    src/ui/IconRenderer.h
    src/ui/IconRenderer.cpp

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
    src/ui/OnboardingDialog.h
    src/ui/OnboardingDialog.cpp
    src/ui/AiHistoryDialog.h
    src/ui/AiHistoryDialog.cpp
    src/ui/AiResultsDialog.h
    src/ui/AiResultsDialog.cpp
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
    src/ui/widgets/ComparisonWidget.h
    src/ui/widgets/ComparisonWidget.cpp
    src/ui/widgets/MotivationWidget.h
    src/ui/widgets/MotivationWidget.cpp
    src/ui/widgets/EmptyState.h
    src/ui/widgets/EmptyState.cpp
    src/ui/widgets/ShadowEffect.h

    # V4.3 — FlowLayout for wrap-on-overflow chip rows (category, color)
    src/ui/widgets/FlowLayout.h
    src/ui/widgets/FlowLayout.cpp
)

# ---- 可选资源（字体 + 图标 SVG） ----
# 若 assets/app.qrc 存在则一起编译进来（用户放置的 ttf 字体也会被打包）
set(APP_RESOURCES "")
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets/app.qrc")
    list(APPEND APP_RESOURCES "assets/app.qrc")
endif()

# Windows 资源（exe 图标 + 元信息）
set(WIN_RES "")
if(WIN32 AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets/app.rc")
    list(APPEND WIN_RES "assets/app.rc")
endif()

qt_add_executable(TimeMaster WIN32 MACOSX_BUNDLE
    ${APP_SOURCES}
    ${APP_RESOURCES}
    ${WIN_RES}
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

# macOS bundle metadata
if(APPLE)
    set_target_properties(TimeMaster PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "TimeMaster"
        MACOSX_BUNDLE_GUI_IDENTIFIER "local.timemaster.app"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    )
endif()

message(STATUS "TimeMaster ${PROJECT_VERSION} configured (Qt ${Qt6_VERSION})")
