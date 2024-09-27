#!/bin/bash

DEST_DIR="${AGENT_ROOT}/obj/libsecurec"
LIB_SECUREC_A=${DEST_DIR}/lib/libsecurec.a

if [ "$1" = "clean" ]
then
    cd ${DEST_DIR}/src/
    make -f Makefile clean
    rm -f ${AGENT_ROOT}/platform/kmc/lib/libsecurec.a
    exit 0
fi

if [ -f "${LIB_SECUREC_A}" ]
then
    echo "${LIB_SECUREC_A} has exist."
    cp -f "${LIB_SECUREC_A}" ${AGENT_ROOT}/platform/kmc/lib
    exit 0
fi

rm -rf ${DEST_DIR}

if [ "`uname`" = "AIX" ] ; then
    cp -rf "${AGENT_ROOT}/build/securec/Makefile_AIX" "${AGENT_ROOT}/platform/securec/src/Makefile"
    cp -rf "${AGENT_ROOT}/platform/securec/" ${DEST_DIR}
elif [ "`uname`" = "SunOS" ] ; then
    cp -rf "${AGENT_ROOT}/build/securec/Makefile_SunOS" "${AGENT_ROOT}/platform/securec/src/Makefile"
    cp -rf "${AGENT_ROOT}/platform/securec/" ${DEST_DIR}
elif [ "`uname`" = "HP-UX" ] ; then
    cp -rf "${AGENT_ROOT}/build/securec/Makefile_HP" "${AGENT_ROOT}/platform/securec/src/Makefile"
    cp -rf "${AGENT_ROOT}/platform/securec/" ${DEST_DIR}
else
    cp -rf "${AGENT_ROOT}/platform/securec/" ${DEST_DIR}
    sed -i 's/$(PROJECT): note_msg $(OBJECTS)/$(PROJECT): note_msg lib $(OBJECTS)/1' "${DEST_DIR}/src/Makefile"
fi

cd ${DEST_DIR}/src
# delete all .o files
rm -rf ${DEST_DIR}/src/*.o

gmake -f "${DEST_DIR}/src/Makefile" "lib"
if [ $? -ne 0 ];then
    echo "Compile SecureC static lib failed"
    exit 1
fi
 
cp -f "${LIB_SECUREC_A}" ${AGENT_ROOT}/platform/kmc/lib

[ -f "${LIB_SECUREC_A}" ] && echo "Compile SecureC static lib success"