cmake_minimum_required(VERSION 3.5)

project(leveldb-ui LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core Widgets Gui REQUIRED)
find_package(leveldb)

set(src
  main.cpp
  mainwindow.cpp
  mainwindow.h
)

set(uis
  mainwindow.ui
)

QT5_WRAP_UI(src ${uis})
add_executable(leveldb-ui ${src})
target_link_libraries(leveldb-ui Qt5::Core Qt5::Widgets Qt5::Gui leveldb)
