add_executable(skip_list_study skip_list_study.cpp)

add_executable(polar_string_study polar_string_study.cpp log.cpp stat.cpp)


add_executable(test_engine_nonoff test_engine.cpp)
#link_directories(${CMAKE_SOURCE_DIR}/lib)
message(${CMAKE_SOURCE_DIR})
target_link_libraries(test_engine_nonoff PRIVATE ${CMAKE_SOURCE_DIR}/lib/libengine.a)