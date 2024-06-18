cmake_minimum_required(VERSION 3.16)
project(CarputerSerialProtocol CXX)

# NanoProtobuf generation
get_filename_component(nanopbDir
    ${CMAKE_CURRENT_LIST_DIR}/../nanopb
    REALPATH
)

list(APPEND CMAKE_MODULE_PATH ${nanopbDir}/extra)
message(STATUS "CMAKE_MODULE_PATH = ${CMAKE_MODULE_PATH}")
find_package(Nanopb REQUIRED)
set_target_properties(nanopb PROPERTIES LINKER_LANGUAGE CXX)


add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../etl ${CMAKE_CURRENT_BINARY_DIR}/etl)
set (protosDir ${CMAKE_CURRENT_LIST_DIR})
set (protoSrcs ${protosDir}/serial-protocol.proto ${protosDir}/timestamp.proto)
set (protoOutputDir ${CMAKE_CURRENT_BINARY_DIR})

nanopb_generate_cpp(TARGET CarputerSerialProtobufs RELPATH ${CMAKE_CURRENT_SOURCE_DIR}
    ${protoSrcs}
)
set_target_properties(CarputerSerialProtobufs PROPERTIES LINKER_LANGUAGE CXX)

# Utils/logging
get_filename_component(utilsDir
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils
    REALPATH
)

add_library(CarputerSerialProtocol
    SerialAgent.cpp
    SerialAgent.hpp
    ${CarputerSerialProtobufs_SRCS}
    ${CarputerSerialProtobufs_HDRS}
)
#get_filename_component(etl_profile_header ${CMAKE_CURRENT_SOURCE_DIR}/../etl_profile.h REALPATH)
#target_compile_definitions(CarputerSerialProtocol
#    PRIVATE
#    $<$<COMPILE_LANGUAGE:CXX>:-include ${etl_profile_header}>
#)

set_target_properties(CarputerSerialProtocol PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(CarputerSerialProtocol
    PRIVATE
    CarputerSerialProtobufs
    etl::etl
)
