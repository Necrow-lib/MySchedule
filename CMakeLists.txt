cmake_minimum_required(VERSION 3.16)
project(myschedule VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Widgets Core QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 5.15 REQUIRED COMPONENTS Widgets Core)
endif()

set(SOURCES
    main.cpp
    mainwindow.cpp
    AccountManager.cpp
    TaskManager.cpp
    Task.cpp
    ReminderThread.cpp
)

set(HEADERS
    mainwindow.h
    AccountManager.h
    TaskManager.h
    Task.h
    ReminderThread.h
)

set(FORMS
    mainwindow.ui
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${FORMS})
target_link_libraries(${PROJECT_NAME} Qt::Widgets Qt::Core)
