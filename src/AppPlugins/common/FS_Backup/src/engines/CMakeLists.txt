add_subdirectory(hostcommon)
add_subdirectory(posix)
add_subdirectory(archive_client)
if (NAS)
    add_subdirectory(libaio)
    add_subdirectory(libnfs)
    add_subdirectory(libsmb)

    add_subdirectory(nfs_anti_ransomware)
endif()

if (OBS)
    add_subdirectory(object)
endif()