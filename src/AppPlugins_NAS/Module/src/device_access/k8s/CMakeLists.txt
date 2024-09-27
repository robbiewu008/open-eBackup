
add_library(k8s_obj
    OBJECT
	K8sutil.cpp
	MSRestClient.cpp
	MSRestApi.cpp
)

target_include_directories(k8s_obj
	PUBLIC
	${MODULE_SRC_PATH}
	${PLATFORM}/SecureCLib_rel/include
	${OPEN_SRC}/boost_rel/include
	${OPEN_SRC}/jsoncpp_rel/include
	${OPEN_SRC}/curl_rel/include
)
target_link_libraries(k8s_obj
    PRIVATE
	securec
	log_util

	boost_chrono
	boost_atomic
	boost_thread
	boost_filesystem
	boost_system
	boost_thread
	boost_log
	boost_date_time
    curl_http_util
)
