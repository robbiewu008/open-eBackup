#!bin/bash
T_CODE_BRANCH=$1

if [ -z ${T_CODE_BRANCH} ];then
    echo "No branch parameter, please specify"
    exit 1
fi

echo master > ${BASE_PATH}/CI/conf/main_branch.txt

while read line;
do
   if [ "$T_CODE_BRANCH" = "$line" ]; then
      git checkout $T_CODE_BRANCH
      git pull origin $T_CODE_BRANCH
      cd ${BASE_PATH}/CI/conf
      bep_env.sh -c bep_env.conf
      git add ${BASE_PATH}/CI/conf/bep_env.conf
	  git commit -m "
[AR/SR/Story/Defects/CR] UADP123456; submit bep_env.conf
[ModifyDesc] submit bep_env.conf
[Author/ID] zhangling WX538034"
      git push origin $T_CODE_BRANCH
    else
       echo "this branch is not main_branch,so do not need to submit bep_env.conf"
   fi 
done < ${BASE_PATH}/CI/conf/main_branch.txt
