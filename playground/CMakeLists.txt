add_executable(skip_list_study legacy/skip_list_study.cpp)

add_executable(polar_string_study polar_string_study.cpp log.cpp stat.cpp)

find_package(OpenMP)
if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif ()

add_executable(test_engine_nonoff test_engine.cpp ../engine_race/engine_race.cc ../engine_race/log.cc
        ../engine_race/stat.cc)
#link_directories(${CMAKE_SOURCE_DIR}/lib)
message(${CMAKE_SOURCE_DIR})


add_executable(test_sparsepp legacy/test_sparsepp.cpp stat.cpp log.cpp)

add_executable(test_aio_linux test_aio_linux.cpp)
target_link_libraries(test_aio_linux PRIVATE rt)

add_executable(test_aio_linux_wrapper legacy/test_aio_posix_wrapper.cpp)
target_link_libraries(test_aio_linux_wrapper PRIVATE rt)

add_executable(test_aio_linux_simple test_aio_linux_simple.cpp log.cpp)
target_link_libraries(test_aio_linux_simple PRIVATE rt)

#target_link_libraries(test_engine_nonoff PRIVATE ${CMAKE_SOURCE_DIR}/engine_race/lib/libengine.a)