##################################################################################
# SRC CMAKELISTS
##################################################################################

# --[ Proto library

file(GLOB proto_files proto/*.proto)
peloton_protobuf_generate_cpp_py(${proto_gen_folder} proto_srcs proto_hdrs proto_python ${proto_files})

# include python files either to force generation
add_library(peloton-proto STATIC ${proto_hdrs} ${proto_srcs} ${proto_python})
set(Peloton_LINKER_LIBS peloton-proto ${Peloton_LINKER_LIBS}) # note, crucial to prepend!
peloton_default_properties(peloton-proto)

##################################################################################

# --[ Parser library

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

set(SQLParser_BISON "${PROJECT_SOURCE_DIR}/src/parser/sql_parser.y")
set(SQLScanner_FLEX "${PROJECT_SOURCE_DIR}/src/parser/sql_scanner.l")
set(SQLParser_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/sql_parser.cpp")
set(SQLScanner_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/sql_scanner.cpp")

# Skip rebuilding parser if it is already built
if(EXISTS ${SQLParser_SOURCE})
	message("-- Found peloton parser")
else()
  	message("-- Building peloton parser")
	BISON_TARGET(SQLParser ${SQLParser_BISON} ${SQLParser_SOURCE})
	FLEX_TARGET(SQLScanner ${SQLScanner_FLEX} ${SQLScanner_SOURCE})
	ADD_FLEX_BISON_DEPENDENCY(SQLScanner SQLParser)
endif()

# Check if the parser is built
add_custom_target(peloton-parser-check
	COMMAND sed -i -e 's/Parser/Parser/g' ${PROJECT_SOURCE_DIR}/src/CMakeLists.txt
)

set(PARSER_FLAGS "-w -Wno-error -fpermissive -Wno-sign-compare")
set_source_files_properties(${SQLParser_SOURCE} PROPERTIES COMPILE_FLAGS ${PARSER_FLAGS})
set_source_files_properties(${SQLScanner_SOURCE} PROPERTIES COMPILE_FLAGS ${PARSER_FLAGS})

add_library(peloton-parser STATIC ${SQLParser_SOURCE} ${SQLScanner_SOURCE})
add_dependencies(peloton-parser peloton-parser-check)

set(Peloton_LINKER_LIBS peloton-parser ${Peloton_LINKER_LIBS}) # note, crucial to prepend!
peloton_default_properties(peloton-parser)

##################################################################################

# --[ Peloton library

# creates 'srcs' lists
peloton_pickup_peloton_sources(${PROJECT_SOURCE_DIR})

add_library(peloton SHARED ${srcs})

target_link_libraries(peloton ${Peloton_LINKER_LIBS} peloton-proto peloton-parser)

peloton_default_properties(peloton)
set_target_properties(peloton PROPERTIES
    VERSION   ${PELOTON_TARGET_VERSION}
    SOVERSION ${PELOTON_TARGET_SOVERSION}
)

##################################################################################

# --[ Peloton

set(peloton_main ${PROJECT_SOURCE_DIR}/src/main/peloton/peloton.cpp)
add_executable(peloton-bin ${peloton_main})
target_link_libraries(peloton-bin peloton)
set_target_properties(peloton-bin PROPERTIES OUTPUT_NAME peloton)

##################################################################################

# --[ Benchmarks

# --[ tpcc
file(GLOB_RECURSE tpcc_srcs ${PROJECT_SOURCE_DIR}/src/main/tpcc/*.cpp)
add_executable(tpcc EXCLUDE_FROM_ALL ${tpcc_srcs})
target_link_libraries(tpcc peloton)

# --[ ycsb
file(GLOB_RECURSE ycsb_srcs ${PROJECT_SOURCE_DIR}/src/main/ycsb/*.cpp)
add_executable(ycsb EXCLUDE_FROM_ALL ${ycsb_srcs})
target_link_libraries(ycsb peloton)

# --[ sdbench
file(GLOB_RECURSE sdbench_srcs ${PROJECT_SOURCE_DIR}/src/main/sdbench/*.cpp)
add_executable(sdbench EXCLUDE_FROM_ALL ${sdbench_srcs})
target_link_libraries(sdbench peloton)


# --[ logger
file(GLOB_RECURSE logger_srcs ${PROJECT_SOURCE_DIR}/src/main/logger/*.cpp)
list(APPEND logger_srcs ${ycsb_srcs})
list(REMOVE_ITEM  logger_srcs ${PROJECT_SOURCE_DIR}/src/main/ycsb/ycsb.cpp)
list(APPEND logger_srcs ${tpcc_srcs})
list(REMOVE_ITEM  logger_srcs ${PROJECT_SOURCE_DIR}/src/main/tpcc/tpcc.cpp)
add_executable(logger EXCLUDE_FROM_ALL ${logger_srcs})
target_link_libraries(logger peloton)


# --[ link to jemalloc
set(EXE_LINK_LIBRARIES ${JEMALLOC_LIBRARIES})
set(EXE_LINK_FLAGS "-Wl,--no-as-needed")
set(EXE_LIST peloton-bin ycsb tpcc sdbench logger)
foreach(exe_name ${EXE_LIST})
    target_link_libraries(${exe_name} ${EXE_LINK_LIBRARIES})
    set_target_properties(${exe_name} PROPERTIES LINK_FLAGS ${EXE_LINK_FLAGS})
endforeach()

# --[ benchmark

add_custom_target(benchmark)
add_dependencies(benchmark tpcc ycsb sdbench logger)


