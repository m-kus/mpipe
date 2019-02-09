cmake_minimum_required(VERSION 2.8)

project(mpipe)

set(MPIPE_DIR "${CMAKE_SOURCE_DIR}/mpipe")
set(SPDLOG_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/dependencies/spdlog/include")
include_directories(${MPIPE_DIR} ${CMAKE_SOURCE_DIR}/dependencies ${SPDLOG_INCLUDE_DIR})

if(MSVC)
  set(CMAKE_CXX_FLAGS "-Gm /EHa")
  set(variables 
    CMAKE_CXX_FLAGS_DEBUG
    CMAKE_CXX_FLAGS_RELEASE
    CMAKE_CXX_FLAGS_RELWITHDEBINFO
    CMAKE_CXX_FLAGS_MINSIZEREL
  )
  foreach(variable ${variables})
    if(${variable} MATCHES "/MD")
      string(REGEX REPLACE "/MD" "/MT" ${variable} "${${variable}}")
    endif()
  endforeach()
else()
  set(CMAKE_CXX_FLAGS "-g -Wall -std=c++11 ${CMAKE_CXX_FLAGS}")
endif()

# mpipe

file(GLOB_RECURSE INC_UTIL "${MPIPE_DIR}/util/*.hpp")
file(GLOB_RECURSE INC_CORE "${MPIPE_DIR}/core/*.hpp")
file(GLOB_RECURSE INC_MCAST "${MPIPE_DIR}/mcast/*.hpp")
file(GLOB_RECURSE INC_MODULES "${MPIPE_DIR}/modules/*.hpp")
file(GLOB_RECURSE INC_EXTENSIONS "${MPIPE_DIR}/extensions/*.hpp")

source_group("util" FILES ${INC_UTIL})
source_group("core" FILES ${INC_CORE})
source_group("mcast" FILES ${INC_MCAST})
source_group("modules" FILES ${INC_MODULES})
source_group("extensions" FILES ${INC_EXTENSIONS})

add_library(mpipe "${MPIPE_DIR}/core/mpipe.cpp" 
  ${INC_UTIL} ${INC_CORE} ${INC_MCAST} ${INC_MODULES} ${INC_EXTENSIONS}
)

# mpipe_test

set(MPIPE_TEST_DIR "${CMAKE_SOURCE_DIR}/mpipe_test")

add_subdirectory(${CMAKE_SOURCE_DIR}/dependencies/googletest)

file(GLOB_RECURSE SRC_TESTS "${MPIPE_TEST_DIR}/*")

add_executable(mpipe_test ${SRC_TESTS})
target_include_directories(mpipe_test PUBLIC ${MPIPE_TEST_DIR} "${gtest_SOURCE_DIR}/include")
target_link_libraries(mpipe_test gtest gtest_main)

add_test(NAME "test" COMMAND mpipe_test)

if(MSVC)
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT mpipe_test)
endif()

# mpipe_binding

if(WIN32)
  set(MPIPE_MODULE_NAME mpipe.pyd)
else()
  set(MPIPE_MODULE_NAME mpipe.so)
endif() 

set(MPIPE_BINDING_DIR "${CMAKE_SOURCE_DIR}/mpipe_binding")

set(PYBIND11_CPP_STANDARD -std=c++11)
set(PYBIND11_PYTHON_VERSION 3.5)
add_subdirectory(${CMAKE_SOURCE_DIR}/dependencies/pybind11)

file(GLOB_RECURSE INC_BINDING "${MPIPE_BINDING_DIR}/*.hpp")
file(GLOB_RECURSE SRC_BINDING "${MPIPE_BINDING_DIR}/*.cpp")

pybind11_add_module(mpipe_binding ${SRC_BINDING} ${INC_BINDING})
target_include_directories(mpipe_binding PUBLIC ${MPIPE_BINDING_DIR})

add_custom_command(
  TARGET mpipe_binding 
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:mpipe_binding> "${CMAKE_SOURCE_DIR}/package/${MPIPE_MODULE_NAME}"
)

if(WIN32)
  add_custom_command(
    TARGET mpipe_binding 
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PDB_FILE:mpipe_binding> "${CMAKE_SOURCE_DIR}/package/mpipe.pdb"
  )
endif()

# mpipe_wheel

set(MPIPE_WHEEL_DIR ${CMAKE_SOURCE_DIR}/wheel)

add_custom_target(mpipe_wheel 
  ALL
  DEPENDS mpipe_binding
  SOURCES "${MPIPE_WHEEL_DIR}/setup.py"
)
add_custom_command(
  TARGET mpipe_wheel 
  PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:mpipe_binding> "${MPIPE_WHEEL_DIR}/${MPIPE_MODULE_NAME}"
)
add_custom_command(
  TARGET mpipe_wheel
  COMMAND ${PYTHON_EXECUTABLE}
  ARGS setup.py bdist_wheel
  WORKING_DIRECTORY ${MPIPE_WHEEL_DIR}
)
