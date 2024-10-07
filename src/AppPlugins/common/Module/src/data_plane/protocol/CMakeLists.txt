set(PROTOCOL_TARGET_NAME data_plane_protocol)
set(PROTOCOL_TARGET_NAME ${PROTOCOL_TARGET_NAME} PARENT_SCOPE)

add_library(${PROTOCOL_TARGET_NAME}
            STATIC include/protocol/Utilities.h
                   src/Utilities.cpp
                   include/protocol/Protocol.h
                   include/protocol/MessageHeader.h

                   # Message buffer
                   include/protocol/Message.h
                   src/Message.cpp
                   
                   # Authorization message
                   include/protocol/AuthorizationMessage.h
                   src/AuthorizationMessage.cpp

                   # Error response
                   include/protocol/ErrorResponse.h
                   src/ErrorResponse.cpp

                   # fuse message
                   include/protocol/fuse/FuseMessage.h
                   src/fuse/FuseMessage.cpp

                   # fuse requests and responses
                   include/protocol/fuse/Flush.h
                   include/protocol/fuse/Lookup.h
                   include/protocol/fuse/GetAttributes.h
                   include/protocol/fuse/GetExtendedAttributes.h
                   include/protocol/fuse/ReadDirectory.h
                   include/protocol/fuse/ReadLink.h
                   include/protocol/fuse/Create.h
                   include/protocol/fuse/SetAttribute.h
                   include/protocol/fuse/Release.h
                   include/protocol/fuse/Open.h
                   include/protocol/fuse/WriteBuffer.h
                   include/protocol/fuse/Read.h
                   include/protocol/fuse/Unlink.h
                   include/protocol/fuse/RemoveDirectory.h
                   include/protocol/fuse/MakeDirectory.h
                   include/protocol/fuse/Link.h
                   include/protocol/fuse/SymLink.h
                   include/protocol/fuse/Rename.h
                   include/protocol/fuse/FAllocate.h
                   include/protocol/fuse/ListExtendedAttributes.h)
target_include_directories(${PROTOCOL_TARGET_NAME}
                           PUBLIC include
                           ${OPEN_SRC}/boost_rel/include
                           ${OPEN_SRC}/libfuse_rel/include
                           ${MODULE_PATH}/src)

