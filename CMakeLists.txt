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
target_link_libraries(${PROJECT_NAME} Qt::Widgets Qt::Core winmm)

option(BUILD_TESTS "Build the test executables" ON)
if(BUILD_TESTS)
    enable_testing()
    find_package(Qt6 COMPONENTS Test QUIET)
    if(NOT Qt6Test_FOUND)
        find_package(Qt5 5.15 REQUIRED COMPONENTS Test)
    endif()

    set(TEST_BASE_SOURCES Task.cpp)

    # 测试 1: Task 序列化
    add_executable(test_task test/test_task.cpp ${TEST_BASE_SOURCES})
    target_link_libraries(test_task Qt::Core Qt::Test)
    target_include_directories(test_task PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    add_test(NAME test_task COMMAND test_task)

    # 测试 2: AccountManager 注册/登录
    add_executable(test_account test/test_account.cpp Task.cpp AccountManager.cpp)
    target_link_libraries(test_account Qt::Core Qt::Test)
    target_include_directories(test_account PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    add_test(NAME test_account COMMAND test_account)

    # 测试 3: TaskManager CRUD 和冲突检测
    add_executable(test_taskmgr test/test_taskmgr.cpp Task.cpp TaskManager.cpp AccountManager.cpp)
    target_link_libraries(test_taskmgr Qt::Core Qt::Test)
    target_include_directories(test_taskmgr PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    add_test(NAME test_taskmgr COMMAND test_taskmgr)
endif()
