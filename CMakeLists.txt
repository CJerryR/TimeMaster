cmake_minimum_required(VERSION 3.16)

project(TimePlan
    VERSION 1.0.0
    DESCRIPTION "时智 - 智能日程管理"
    LANGUAGES CXX)

# ---- C++ 标准 ----
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Qt 自动化工具 ----
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Apple 平台：构建 macOS bundle (.app)
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "macOS minimum target")
endif()

# ---- 查找 Qt6 ----
find_package(Qt6 6.2 REQUIRED COMPONENTS
    Core
    Widgets
    Network
    Sql
)

qt_standard_project_setup()

# ---- 源码 ----
set(PROJECT_SOURCES
    src/main.cpp

    # core
    src/core/Types.h
    src/core/Types.cpp
    src/core/Database.h
    src/core/Database.cpp
    src/core/DeepSeekClient.h
    src/core/DeepSeekClient.cpp

    # ui
    src/ui/Theme.h
    src/ui/Theme.cpp
    src/ui/EventDialog.h
    src/ui/EventDialog.cpp
    src/ui/MonthView.h
    src/ui/MonthView.cpp
    src/ui/TimeGridView.h
    src/ui/TimeGridView.cpp
    src/ui/CalendarPage.h
    src/ui/CalendarPage.cpp
    src/ui/AnalyticsPage.h
    src/ui/AnalyticsPage.cpp
    src/ui/CategoryPieChart.h
    src/ui/CategoryPieChart.cpp
    src/ui/widgets/StatsCardsWidget.h
    src/ui/widgets/StatsCardsWidget.cpp
    src/ui/widgets/HorizontalBarChart.h
    src/ui/widgets/HorizontalBarChart.cpp
    src/ui/widgets/DailyTrendChart.h
    src/ui/widgets/DailyTrendChart.cpp
    src/ui/widgets/SourceDistributionWidget.h
    src/ui/widgets/SourceDistributionWidget.cpp
    src/ui/widgets/InsightsWidget.h
    src/ui/widgets/InsightsWidget.cpp
    src/ui/widgets/RhythmCardWidget.h
    src/ui/widgets/RhythmCardWidget.cpp
    src/ui/Sidebar.h
    src/ui/Sidebar.cpp
    src/ui/ChatPage.h
    src/ui/ChatPage.cpp
    src/ui/SettingsDialog.h
    src/ui/SettingsDialog.cpp
    src/ui/MainWindow.h
    src/ui/MainWindow.cpp
)

# ---- 可执行文件 ----
qt_add_executable(TimePlan
    MACOSX_BUNDLE
    WIN32
    ${PROJECT_SOURCES}
)

target_link_libraries(TimePlan PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::Sql
)

target_include_directories(TimePlan PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ---- macOS Bundle 元数据 ----
if(APPLE)
    set_target_properties(TimePlan PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "时智"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.timeplan.app"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
        MACOSX_BUNDLE_COPYRIGHT "Copyright © 2026"
    )
endif()

# ---- 安装 ----
include(GNUInstallDirs)
install(TARGETS TimePlan
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# ---- macOS 部署：自动 deployqt 打包依赖 ----
if(APPLE)
    qt_generate_deploy_app_script(
        TARGET TimePlan
        OUTPUT_SCRIPT deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(SCRIPT ${deploy_script})
endif()
