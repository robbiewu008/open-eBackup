
aux_source_directory(. COM_SRCS)
aux_source_directory(uuid UUID_SRCS)
aux_source_directory(utils UTILS_SRCS)
aux_source_directory(checkpoint CHECKPOINT_SRCS)
aux_source_directory(sha256 SHA256_SRCS)
aux_source_directory(httpclient HTTP_SRCS)
aux_source_directory(model MODEL_SRCS)
aux_source_directory(client CLIENT_SRCS)
aux_source_directory(token_mgr TOKEN_MGR_SRCS)
aux_source_directory(cert_mgr CERT_MGR_SRCS)
aux_source_directory(execute_python EXECUTE_PYTHON_SRCS)
aux_source_directory(login_host LOGIN_HOST_SRC)
aux_source_directory(openstorage_api_client OPENSTORAGE_API_CLIENT_SRCS)
file(GLOB_RECURSE OPENSTORAGE_API_CLIENT_SRCS openstorage_api_client/*.cpp openstorage_api_client/*.h)


add_library(com_obj
    OBJECT
    ${COM_SRCS}
    ${UUID_SRCS}
    ${UTILS_SRCS}
    ${CHECKPOINT_SRCS}
    ${SHA256_SRCS}
    ${HTTP_SRCS}
    ${MODEL_SRCS}
    ${CLIENT_SRCS}
    ${TOKEN_MGR_SRCS}
    ${CERT_MGR_SRCS}
    ${EXECUTE_PYTHON_SRCS}
    ${LOGIN_HOST_SRC}
    ${OPENSTORAGE_API_CLIENT_SRCS}
)


target_link_libraries(com_obj
    PRIVATE
    vir_safe_cmplib
)