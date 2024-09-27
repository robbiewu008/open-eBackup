#!/bin/bash

CUR_PATH=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
cd ${CUR_PATH}/src/service/console
content=`cat repeat-i18n.json |grep "}" | sed 's/{//g' | sed 's/}//g'`
if [ "$content" = "" ]; then
	exit 0
else 
    echo "repeat-i18n.json 内容不为空，异常退出。"
    echo "repeat-i18n.json 内容如下："
    cat repeat-i18n.json
    exit 1
fi
