#!bin/bash
set -ex
CUR_PATH=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
BASE_PATH=${CUR_PATH}/../..
function compile(){
	cd ${CUR_PATH}/
	python3 setup.py sdist bdist_wheel
	if [ $? -ne 0 ]; then
		echo "opensource apscheduler compile failed."
		exit 1
	fi
	echo "${CUR_PATH}/dist contains:"
	ls -l ${CUR_PATH}/dist
	 [ -d ${BASE_PATH}/output/ ] && rm -rf ${BASE_PATH}/output/
	mkdir -p ${BASE_PATH}/output/
	cp -rf ${CUR_PATH}/dist/* ${BASE_PATH}/output/
	
}
function upload(){
	cd ${CUR_PATH}/../
	sh upload_py.sh apscheduler
}
function main(){
	compile
	upload
}
echo "#########################################################"
echo "   Begin compile opensource apscheduler "
echo "#########################################################"
main
echo "#########################################################"
echo "   apscheduler Compile Success  "
echo "#########################################################"
