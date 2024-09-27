#!/bin/sh
    BACKUP="/home/ip1;/home/ip2;/home/ip3"
    QueryDataRst="./test.txt"
    MYAWK=awk
    rm -rf ${QueryDataRst}
    cat >> $QueryDataRst <<EOF
    233792	    2 SOE2			     /u01/app/oracle/oradata/soe/soe2.dbf
    194496	   10 SOE5			     /u01/app/oracle/oradata/soe/soe5.dbf
    186880	    9 SOE4			     /u01/app/oracle/oradata/soe/soe4.dbf
    186240	   11 SOE6			     /u01/app/oracle/oradata/soe/soe6.dbf
    185664	   12 SOE7			     /u01/app/oracle/oradata/soe/soe7.dbf
    184832	    5 SOE1			     /u01/app/oracle/oradata/soe/soe1.dbf
    175361.25	    8 SOE3			     /u01/app/oracle/oradata/soe/soe3.dbf
      7985	    4 UNDOTBS1			     /u01/app/oracle/oradata/ORCL/datafile/o1_mf_undotbs1_homo0fok_.dbf
      1700	    3 SYSAUX			     /u01/app/oracle/oradata/ORCL/datafile/o1_mf_sysaux_homnznlv_.dbf
       900	    1 SYSTEM			     /u01/app/oracle/oradata/ORCL/datafile/o1_mf_system_homny7hg_.dbf
    5    7 USERS			     /u01/app/oracle/oradata/ORCL/datafile/o1_mf_users_homo0gqo_.dbf
   
EOF
    
    dataPaths=`echo $BACKUP | sed 's/;/ /g'`
    pathNum=0
    for dataPath in ${dataPaths}; do
        [ -z "$dataPath" ] && continue
        pathNum=`expr $pathNum + 1`
    done

    # set the Ip path order by data file size desc
    # 100G File1  200G File2 300G File3
    # there are 2 ip paths: ip1 ip2
    # assign result: File1-ip1 File2-ip2 File3-ip1
    pathNo=0
    fileSQL=
    while read line; do
        [ -z "$line" ] && continue
        fSize="`echo $line | ${MYAWK} '{print $1}'`"
        fNo="`echo $line | ${MYAWK} '{print $2}'`"
        tsName="`echo $line | ${MYAWK} '{print $3}'`"
        fsFile="`echo $line | ${MYAWK} '{print $4}'`"

        # confirm ip path NO
        selPathNo=$(($pathNo % $pathNum))
        selPath=
        curPath=0
        for dataPath in ${dataPaths}; do
            [ -z "$dataPath" ] && continue
            if [ "$curPath" = "$selPathNo" ]; then
                selPath=$dataPath
                break
            fi
            curPath=`expr $curPath + 1`
        done

        if [ -z "${selPath}" ]; then
            Log "sel path is NULL, curPath=$curPath, selPathNo=$selPathNo."
            exit 1
        fi

        if [ -z "$fileSQL" ]; then
            fileSQL="'${fsFile}','${selPath}/${fNo}/FNO-${fNo}_TS-${tsName}'"
        else
            fileSQL="${fileSQL},'${fsFile}','${selPath}/${fNo}/FNO-${fNo}_TS-${tsName}'"
        fi

        pathNo=`expr $pathNo + 1`
    done < ${QueryDataRst}
    MultiIPSQL="backup as copy incremental level 0 tag 'EBACKUP-orcl-DATA' DB_FILE_NAME_CONVERT=(${fileSQL}) database;"
    
    echo MultiIPSQL=$MultiIPSQL
    