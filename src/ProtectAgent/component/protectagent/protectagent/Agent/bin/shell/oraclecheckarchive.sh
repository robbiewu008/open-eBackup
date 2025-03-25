#!/bin/sh
# 
#  This file is a part of the open-eBackup project.
#  This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
#  If a copy of the MPL was not distributed with this file, You can obtain one at
#  http://mozilla.org/MPL/2.0/.
# 
#  Copyright (c) [2024] Huawei Technologies Co.,Ltd.
# 
#  THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
#  EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
#  MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
#
set +x

#@dest:  to check oracle archive dest using
#@date:  2015-01-24
#@authr: 
USAGE="Usage: ./oraclecheckarchive.sh AgentRoot PID"

AGENT_ROOT_PATH=$1
PID=$2
PARAM_NUM=$3
IN_ORACLE_HOME=""
. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"
. "${AGENT_ROOT_PATH}/sbin/oraclefunc.sh"

#********************************define these for the functions of agent_sbin_func.sh********************************
#for GetUserShellType
ORACLE_SHELLTYPE=
GRID_SHELLTYPE=
RDADMIN_SHELLTYPE=
#for Log
LOG_FILE_NAME="${LOG_PATH}/oraclecheckarchive.log"
#for GetOracleVersion
ORA_VERSION=
PREVERSION=
VERSION=
ORCLVESION="${STMP_PATH}/ORCV${PID}.txt"
SQLEXIT="${STMP_PATH}/EXITSQL${PID}.sql"
#for GetValue
#for GetArchiveLogMode
GET_ARCHIVE_LOG_MODE_SQL="${STMP_PATH}/get_archive_log_mode${PID}.sql"
#********************************define these for the functions of agent_sbin_func.sh********************************

#********************************define these for local script********************************
DBNAME=""
DBUSER=""
DBUSERPWD=""
DBINSTANCE=""
#Threshold of the database archive using, unit(M) 
DBARCHIVE_LIMEN=0

# Archive Log
ARCHIVEDESTSQL="${STMP_PATH}/ArchiveDestSQL${PID}.sql"
ARCHIVEDESTRST="${STMP_PATH}/ArchiveDestRST${PID}.txt"
ARCHIVE_DEST_LIST="${STMP_PATH}/ArchiveDestList${PID}.txt"
PARAM_CONTENT=`ReadInputParam`
test -z "$PARAM_CONTENT"              && ExitWithError "PARAM_FILE"
#********************************define these for local script********************************



checkArchiveModeAndArchiveUsing()
{
    # create check archive mode sql
    Log "Begin check archive dest directory."
    echo "set linesize 300;" > "${ARCHIVEDESTSQL}"
    echo "col DESTINATION for a255;" >> "${ARCHIVEDESTSQL}"
    echo "select DESTINATION from v\$archive_dest where STATUS='VALID';" >> "${ARCHIVEDESTSQL}"
    echo "exit" >> "${ARCHIVEDESTSQL}"
    Log "Exec SQL to get destination of archive dest(INACTIVE)."
    OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${ARCHIVEDESTSQL}" "${ARCHIVEDESTRST}" "${DBINSTANCE}" 30 0 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "$RET_CODE" -ne "0" ]
    then
        DeleteFile "${ARCHIVEDESTSQL}" "${ARCHIVEDESTRST}"
        Log "Get Archive log dest list failed."
        exit ${RET_CODE}
    fi
        
    DeleteFile "${ARCHIVE_DEST_LIST}"
    touch "${ARCHIVE_DEST_LIST}"
    cat "${ARCHIVEDESTRST}" | grep "^/" >> "${ARCHIVE_DEST_LIST}"
    cat "${ARCHIVEDESTRST}" | grep "^+" >> "${ARCHIVE_DEST_LIST}"
    cat "${ARCHIVEDESTRST}" | grep "USE_DB_RECOVERY_FILE_DEST" >> "${ARCHIVE_DEST_LIST}"
    DeleteFile "${ARCHIVEDESTSQL}" "${ARCHIVEDESTRST}"
    
    exec 4<&0
    exec <"${ARCHIVE_DEST_LIST}"
    while read line
    do
        STRARCHIVEDEST=$line
        if [ -z "${STRARCHIVEDEST}" ]
        then
            continue
        fi
        IS_RECOVERY_FILE_DEST=0
        FAST_USEDCAPACITY=0
        FAST_ALLCAPACITY=0
        
        Log "STRARCHIVEDEST=${STRARCHIVEDEST}"
        # default dest, need to search direcory
        if [ "${STRARCHIVEDEST}" = "USE_DB_RECOVERY_FILE_DEST" ]
        then
            IS_RECOVERY_FILE_DEST=1
            # get archive dest
            echo "set linesize 300;" > "${ARCHIVEDESTSQL}"
            echo "col NAME for a255;" >> "${ARCHIVEDESTSQL}"
            echo "select NAME from V\$RECOVERY_FILE_DEST;" >> "${ARCHIVEDESTSQL}"
            echo "exit" >> "${ARCHIVEDESTSQL}"

            Log "Exec SQL to get name of archive dest."
            OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${ARCHIVEDESTSQL}" "${ARCHIVEDESTRST}" "${DBINSTANCE}" 30 0 >> "${LOG_FILE_NAME}" 2>&1
            RET_CODE=$?
            if [ "$RET_CODE" -ne "0" ]
            then
                DeleteFile "${ARCHIVEDESTSQL}" "${ARCHIVEDESTRST}"
                DeleteFile "${ARCHIVE_DEST_LIST}"
                Log "Get Archive dest failed."
                exit ${RET_CODE}
            fi
    
            STRARCHIVEDEST=`sed -n '/----------/,/^ *$/p' "${ARCHIVEDESTRST}" | sed -e '/----------/d' -e '/^ *$/d' | ${MYAWK} '{print $1}'`
            Log "STRARCHIVEDEST=${STRARCHIVEDEST}."
            DeleteFile "${ARCHIVEDESTSQL}" "${ARCHIVEDESTRST}"
            
            # get archive using
            echo "set linesize 100;" > "${ARCHIVEDESTSQL}"
            echo "select SPACE_LIMIT/1024/1024,SPACE_USED/1024/1024 from V\$RECOVERY_FILE_DEST; " >> "${ARCHIVEDESTSQL}"
            echo "exit" >> "${ARCHIVEDESTSQL}"

            Log "Exec SQL to get using of archive dest."
            OracleExeSql "${DBUSER}" "${DBUSERPWD}" "${ARCHIVEDESTSQL}" "${ARCHIVEDESTRST}" "${DBINSTANCE}" 30 0 >> "${LOG_FILE_NAME}" 2>&1
            RET_CODE=$?
            if [ "$RET_CODE" -ne "0" ]
            then
                DeleteFile "${ARCHIVEDESTSQL}" "${ARCHIVEDESTRST}"
                DeleteFile "${ARCHIVE_DEST_LIST}"
                Log "Get Archive dest using failed."
                exit ${RET_CODE}
            fi
            
            #get result like below, and get the content between ----- and black
            #   Connected to:
            #   Oracle Database 11g Enterprise Edition Release 11.2.0.3.0 - 64bit Production
            #   With the Partitioning, OLAP, Data Mining and Real Application Testing options
            #
            #
            #   STATUS
            #   ------------
            #   OPEN
            #
            #   Disconnected from Oracle Database 11g Enterprise Edition Release 11.2.0.3.0 - 64bit Production
            TMPLINE=`sed -n '/----------/,/^ *$/p' "${ARCHIVEDESTRST}" | sed -e '/----------/d' -e '/^ *$/d' | ${MYAWK} '{print $1";"$2}'`
            for i in ${TMPLINE}
            do
                FAST_ALLCAPACITY=`echo $i | ${MYAWK} -F ";" '{print $1}'`
                FAST_USEDCAPACITY=`echo $i | ${MYAWK} -F ";" '{print $2}'`
                # deal number pot
                FAST_ALLCAPACITY=`echo $FAST_ALLCAPACITY | ${MYAWK} -F "." '{print $1}'`
                FAST_USEDCAPACITY=`echo $FAST_USEDCAPACITY | ${MYAWK} -F "." '{print $1}'`
                # first value
                break
            done
            
            DeleteFile "${ARCHIVEDESTSQL}" "${ARCHIVEDESTRST}"
            Log "FAST_ALLCAPACITY=$FAST_ALLCAPACITY;FAST_USEDCAPACITY=$FAST_USEDCAPACITY"
        else
            # check whether the archive dest is exists, if not, the suffix maybe is the perfix of the archive files.
            #.eg 
            # Archive destination            /lvoracle/app/oracle/11.2.0/db_1/dbs/arch/abc
            # the last abc mabe is the perfix of the archive files.
            LASTCHAR=`echo ${STRARCHIVEDEST} | ${MYAWK} '{print substr($NF,length($NF),1)}'`
            if [ "`RDsubstr $STRARCHIVEDEST 1 1`" = "/" ] && [ "${LASTCHAR}" != "/" ] && [ ! -d "${STRARCHIVEDEST}" ]
            then
                FILEPREFIX=`echo ${STRARCHIVEDEST} | ${MYAWK} -F "/" '{print $NF}'`
                LENPREFIX=`echo $FILEPREFIX | ${MYAWK} '{print length($1)}'`
                LENALLDEST=`echo $STRARCHIVEDEST | ${MYAWK} '{print length($1)}'`
                
                LENDEST=`expr $LENALLDEST - $LENPREFIX - 1`
                STRARCHIVEDEST=`RDsubstr $STRARCHIVEDEST 1 $LENDEST`
            fi
            
            if [ ! -d "${STRARCHIVEDEST}" ]
            then
                Log "Archive Dest ${STRARCHIVEDEST} is not exists"
            fi
        fi
        
        Log "STRARCHIVEDEST=${STRARCHIVEDEST}"
        if [ "`RDsubstr $STRARCHIVEDEST 1 12`" = "/dev/raw/raw" ]
        then
            Log "Archive dest ${STRARCHIVEDEST} is the raw, not support check archive dest size."
            continue
        fi
        
        FREECAPACITY=0
        ALLCAPACITY=0
        #file system
        if [ "`RDsubstr $STRARCHIVEDEST 1 1`" = "/" ]
        then
            if [ "${sysName}" = "HP-UX" ]
            then
                FREECAPACITY=`df -k $STRARCHIVEDEST | sed '1d' | $MYAWK '{print $1}' | sed -n '1p'`
                FREECAPACITY=`expr $FREECAPACITY / 1024`
                ALLCAPACITY=`df -k $STRARCHIVEDEST | sed '2,4d' | $MYAWK -F ":" '{print $2}' | $MYAWK '{print $1}'`
                ALLCAPACITY=`expr $ALLCAPACITY / 1024`
            elif [ "${sysName}" = "AIX" ]
            then
                FREECAPACITY=`df -k $STRARCHIVEDEST | sed '1d' | $MYAWK '{print $3}'`
                FREECAPACITY=`expr $FREECAPACITY / 1024`
                ALLCAPACITY=`df -k $STRARCHIVEDEST | sed '1d' | $MYAWK '{print $2}'`
                ALLCAPACITY=`expr $ALLCAPACITY / 1024`
            elif [ "${sysName}" = "SunOS" ]
            then
                FREECAPACITY=`df -k $STRARCHIVEDEST | sed '1d' | $MYAWK '{print $4}'`
                FREECAPACITY=`expr $FREECAPACITY / 1024`
                ALLCAPACITY=`df -k $STRARCHIVEDEST | sed '1d' | $MYAWK '{print $2}'`
                ALLCAPACITY=`expr $ALLCAPACITY / 1024`
            else
                LINENUM=`df -m $STRARCHIVEDEST | wc -l`
                if [ $LINENUM = "2" ]
                then
                    FREECAPACITY=`df -m $STRARCHIVEDEST | sed '1d' | $MYAWK '{print $4}'`
                    ALLCAPACITY=`df -m $STRARCHIVEDEST | sed '1d' | $MYAWK '{print $2}'`
                else
                    FREECAPACITY=`df -m $STRARCHIVEDEST | sed '1,2d' | $MYAWK '{print $3}'`
                    ALLCAPACITY=`df -m $STRARCHIVEDEST | sed '1,2d' | $MYAWK '{print $1}'`
                fi
             fi
            Log "file system capacity:ALLCAPACITY=$ALLCAPACITY,FREECAPACITY=$FREECAPACITY."
        fi

        #archive dest
        if [ "`RDsubstr $STRARCHIVEDEST 1 1`" = "+" ]
        then
            ASMSIDNAME=`ps -ef | grep "asm_...._${ASMSIDNAME}" | grep -v "grep" | ${MYAWK} -F '+' '{print $NF}'`
            ASMSIDNAME=`echo ${ASMSIDNAME} | ${MYAWK} -F " " '{print $1}'`
            ASMSIDNAME="+"${ASMSIDNAME}
            Log "Check ASM instance name $ASMSIDNAME."

            #check ASM instance status
            ASMINSTNUM=`ps -ef | grep "asm_...._${ASMSIDNAME}" | grep -v "grep" | wc -l`
            if [ "${ASMINSTNUM}" -eq "0" ]
            then
                Log "The ASM instance is not open."
                DeleteFile "${ARCHIVE_DEST_LIST}"
                exit ${ERROR_ORACLE_ASM_INSTANCE_NOSTART}
            fi

            ASM_DEST=`RDsubstr $STRARCHIVEDEST 2`
            ASM_DISKGROUPNAME=`echo $ASM_DEST | $MYAWK -F "/" '{print $1}'`
            
            GetASMUsage
            Log "ASM capacity:ALLCAPACITY=$ALLCAPACITY,FREECAPACITY=$FREECAPACITY."
        fi

        local used=$(( ${ALLCAPACITY} - ${FREECAPACITY} ))
        local usedRate=`${MYAWK} 'BEGIN{printf "%.0f\n",('$used'/'$ALLCAPACITY')*100}'`
         if [ ${FAST_ALLCAPACITY} -gt 0 ]  && [ ${FAST_USEDCAPACITY} -gt 0 ]; then
            local usedRate_Fast=`${MYAWK} 'BEGIN{printf "%.0f\n",('$FAST_USEDCAPACITY'/'$FAST_ALLCAPACITY')*100}'`
            if [ ${usedRate_Fast} -gt ${usedRate} ]; then
                usedRate=${usedRate_Fast}
            fi
         fi
         if [ ${usedRate} -ge ${DBARCHIVE_LIMEN} ]; then
            Log "archive dest(${STRARCHIVEDEST}) usedRate(${usedRate}%) is large than threshold."
            DeleteFile "${ARCHIVE_DEST_LIST}"
            exit ${ERROR_ORACLE_OVER_ARCHIVE_USING}
        fi
    done
    exec 0<&4 4<&-
    
    DeleteFile "${ARCHIVE_DEST_LIST}"
    DeleteFile "${ARCHIVEDESTSQL}" "${ARCHIVEDESTRST}"
    Log "success check archive dest directory using."
    return 0
}

GetASMUsage()
{
    if [ "$VERSION" -ge "112" ]; then
        ASMUser=${ORA_GRID_USER}
        SHELLTYPE=${GRID_SHELLTYPE}
    else
        ASMUser=${ORA_DB_USER}
        SHELLTYPE=${ORACLE_SHELLTYPE}
    fi
    SHELLTYPE=`cat /etc/passwd | grep "^${ASMUser}:" | ${MYAWK} -F "/" '{print $NF}'`
    if [ "$SHELLTYPE" = "csh" ]; then
        CMD_SETENV="setenv ORACLE_SID ${ASMSIDNAME}"
    else
        CMD_SETENV="ORACLE_SID=${ASMSIDNAME};export ORACLE_SID"
    fi
    DGCapInfo=`su - ${ASMUser} ${SHELLTYPE} -c "${EXPORT_GRID_ENV}${CMD_SETENV}; echo lsdg ${ASM_DISKGROUPNAME} | asmcmd" | grep "${ASM_DISKGROUPNAME}"`
    DGInfoHeader=`su - ${ASMUser} ${SHELLTYPE} -c "${EXPORT_GRID_ENV}${CMD_SETENV}; echo lsdg | asmcmd" | sed -n '1p' | ${MYAWK} '{$1="";print $0}'`
    local index=0
    local TotalNum=0
    local FreeNum=0
    for str in ${DGInfoHeader}; do
        let index+=1
        if [ "$str" == "Total_MB" ]; then
            TotalNum=${index}
        fi
        if [ "$str" == "Free_MB" ]; then
            FreeNum=${index}
        fi
    done
    index=0
    for str in ${DGCapInfo}; do
        let index+=1
        if [ ${index} -eq ${TotalNum} ]; then
            ALLCAPACITY=${str}
        fi
        if [ ${index} -eq ${FreeNum} ]; then
            FREECAPACITY=${str}
        fi
    done
}

#############################################################
DBINSTANCE=`GetValue "${PARAM_CONTENT}" InstanceName`
DBARCHIVE_LIMEN=`GetValue "${PARAM_CONTENT}" ArchiveThreshold`

test "$DBINSTANCE" = "${ERROR_PARAM_INVALID}"                          && ExitWithError "DBINSTANCE"
test "$DBARCHIVE_LIMEN" = "${ERROR_PARAM_INVALID}"                     && ExitWithError "DBARCHIVE_LIMEN"

if [ "$DBARCHIVE_LIMEN" = "" ]
then
    DBARCHIVE_LIMEN=0
fi
DBUSER=`GetValue "${PARAM_CONTENT}" UserName`
DBUSERPWD=`GetValue "${PARAM_CONTENT}" Password`
IN_ORACLE_HOME=`GetValue "${PARAM_CONTENT}" OracleHome`
ASMSIDNAME=`GetValue "${PARAM_CONTENT}" ASMInstanceName`

test "$DBUSER" = "${ERROR_PARAM_INVALID}"                          && ExitWithError "DBUSER"
test "$IN_ORACLE_HOME" = "${ERROR_PARAM_INVALID}"                  && ExitWithError "IN_ORACLE_HOME"
test "$ASMSIDNAME" = "${ERROR_PARAM_INVALID}"                      && ExitWithError "ASMSIDNAME"

if [ -z "${ASMSIDNAME}" ]
then
    ASMSIDNAME="+ASM"
fi

Log "DBINSTANCE=${DBINSTANCE};DBUSER=${DBUSER};DBARCHIVE_LIMEN=${DBARCHIVE_LIMEN};IN_ORACLE_HOME=$IN_ORACLE_HOME"


GetOracleUser
GetUserShellType ${ORA_DB_USER} ${ORA_GRID_USER}
GetOracleVersion ${ORA_DB_USER}
VERSION=`echo $PREVERSION | tr -d '.'`

CheckDbInvalid()
{
    ps -aef | sed 's/ *$//g' | grep "ora_...._${DBINSTANCE}$" | grep -v "grep" > /dev/null
    if [ "$?" = "0" ]; then
        GetArchiveLogMode ${DBINSTANCE}
        if [ "$?" = "1" ]; then
            Log "db ${DBINSTANCE} is open, and is Archive Mode."
            return 0
        fi
    fi
    return 1
}
CheckDbInvalid
if [ "$?" -ne "0" ]; then
    exit 0
fi

#check archive mode & archive using
if [ "${DBARCHIVE_LIMEN}" -ne "0" ]
then
    checkArchiveModeAndArchiveUsing
    if [ "$?" -ne "0" ]
    then
        Log "checkArchiveModeAndArchiveUsing exec failed."
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
else
    Log "DBARCHIVE_LIMEN is zero, not to check archive using."
fi

exit 0
