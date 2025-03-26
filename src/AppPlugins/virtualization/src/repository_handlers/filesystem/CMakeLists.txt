set(REPO_HANDLER_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../)
message(STATUS "REPO_HANDLER_INC_DIR: " ${REPO_HANDLER_INC_DIR})

add_library(fs_handler_obj
    OBJECT
    FileSystemHandler.cpp
)

target_include_directories(fs_handler_obj
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${REPO_HANDLER_INC_DIR}
)

target_link_libraries(fs_handler_obj
        PRIVATE
        vir_safe_cmplib
        )