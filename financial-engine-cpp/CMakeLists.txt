cmake_minimum_required(VERSION 3.30)
project(stock_analysis_engine)

set(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(stock_analysis_engine main.cpp
        include/stock_data.h)
