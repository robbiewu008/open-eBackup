#!/bin/sh
set +x

PID=$1
AGENT_ROOT_PATH=$2
AGENT_THIRDPARTY_BINPATH="${AGENT_ROOT_PATH}/bin"
AGENT_THIRDPARTY_TMPPATH="${AGENT_ROOT_PATH}/tmp"
AGENT_THIRDPARTY_LOGPATH="${AGENT_ROOT_PATH}/log"
LOG_FILE_NAME="${AGENT_THIRDPARTY_LOGPATH}/thirdparty.log"
AGENT_THIRDPARTY_RSTFILE="$AGENT_THIRDPARTY_TMPPATH/RST${PID}.txt"
AGENT_THIRDPARTY_INPUT_PARA_TEMP_FILE="${AGENT_THIRDPARTY_TMPPATH}/input_tmp${PID}"
. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"

#for GetValue
ArgFile="${AGENT_THIRDPARTY_TMPPATH}/ARG${PID}"

if [ -p "${AGENT_THIRDPARTY_INPUT_PARA_TEMP_FILE}" ]
then
    INPUT_PARAMETER_LIST=`cat ${AGENT_THIRDPARTY_INPUT_PARA_TEMP_FILE}`
fi

function Exit()
{
    if [ "$1" = "1" ]
    then
        if [ "$2" = "-log" ]
        then
            Log "$3"
        fi
        if [ "$4" = "-ret" ]
        then
            echo "$5" > ${AGENT_THIRDPARTY_RSTFILE}
        fi
        exit 1
    fi
    if [ "$1" = "0" ]
    then
        if [ "$2" = "-log" ]
        then
            Log "$3"
        fi
        if [ "$4" = "-ret" ]
        then
            if [ "$5" != "" ]
            then
                echo "$5" > ${AGENT_THIRDPARTY_RSTFILE}
            fi
        fi
        exit 0
    fi
}
