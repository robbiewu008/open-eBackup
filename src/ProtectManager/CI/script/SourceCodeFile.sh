#!bin/bash

CUR_PATH=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
BASE_PATH=${CUR_PATH}/../..

cd ${BASE_PATH}/
commit_address=$(git remote -v | grep fetch | awk '{print $2}')
commit_branch=$(git branch | awk -F ' ' '{print $2}')
commit_id=$(git log -1 | grep commit | awk '{print $2}')
commit_name="A8000_CI/component/PM_CI"
echo "<fileidentify repoBase=\"${commit_address}\" repoType=\"git\"  localpath=\"${commit_name}\" branch=\"${commit_branch}\" revision=\"${commit_id}\" />" > PM_version

cd ${BASE_PATH}/component
DIR=`ls`
for i in ${DIR[@]}
	do
        if [ -d ${i} ]; then
			cd ${i}
			commit_address=$(git remote -v | grep fetch | awk '{print $2}')
			commit_name="A8000_CI/component/PM_CI/component/${i}"
			commit_branch=$(git branch | awk -F ' ' '{print $2}')
			commit_id=$(git log -1 | grep commit | awk '{print $2}')
			echo "<fileidentify repoBase=\"${commit_address}\" repoType=\"git\"  localpath=\"${commit_name}\" branch=\"${commit_branch}\" revision=\"${commit_id}\" />" >> ${BASE_PATH}/PM_version
			cd ${BASE_PATH}/component
		fi
done
