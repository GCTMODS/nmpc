CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7 FATAL_ERROR)
PROJECT(cnmpc)

INCLUDE_DIRECTORIES(../include)

ADD_LIBRARY(cnmpc SHARED cnmpc.cpp)

ADD_DEPENDENCIES(cnmpc qpDUNES nmpclib)

ExternalProject_Get_Property(qpDUNES binary_dir)
SET(qpDUNES_dir ${binary_dir})

TARGET_LINK_LIBRARIES(cnmpc
	${qpDUNES_dir}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}qpDUNES${CMAKE_STATIC_LIBRARY_SUFFIX}
	nmpclib)
