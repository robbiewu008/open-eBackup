add_definitions(-D_SUSE10_64 -DBUILD_LCOV)
add_library(testStubobj
    OBJECT
    stub.cpp
    stub.h
)

target_compile_options(testStubobj INTERFACE -fPIC -Wall -MMD)

target_link_libraries(testStubobj
    PRIVATE
    safe_cmplib
)
