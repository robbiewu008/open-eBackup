set(REPO_HANDLER_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../)
message(STATUS "REPO_HANDLER_INC_DIR: " ${REPO_HANDLER_INC_DIR})

add_library(repo_factory_obj
    OBJECT
    RepositoryFactory.cpp
)

target_include_directories(repo_factory_obj
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${REPO_HANDLER_INC_DIR}
)

target_link_libraries(repo_factory_obj
        PRIVATE
        vir_safe_cmplib
        )
