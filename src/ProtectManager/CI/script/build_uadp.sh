#!bin/bash

CUR_PATH=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
BASE_PATH=${CUR_PATH}/../..

function main()
{
	cd ${BASE_PATH}/component
	DIR=`ls`
	for i in ${DIR[@]}
		do
			cd $i/CI
			sh build.sh
			if [ $? -ne 0 ]; then
				echo "package $i failed!"
				exit 1
			fi
			cd ${BASE_PATH}/component
	done

}


main
