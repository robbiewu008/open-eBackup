!bin/bash
set -ex
CUR_PATH=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
BASE_PATH=${CUR_PATH}/../
soft_name=$1
function set_env()
{
    [ -f ~/.config/pip/pip.conf ] && rm -rf ~/.config/pip/pip.conf
    pip3 config set global.index-url https://cmc.centralrepo.rnd.huawei.com/artifactory/pypi-central-repo/simple/
    pip3 config set global.disable-pip-version-check true
    pip3 config set global.trusted-host "cmc.centralrepo.rnd.huawei.com"
    pip3 config set global.cache-dir ~/.cache/pip
    cd ${BASE_PATH}
    #安装上传制品仓需要的工具
	pip3 install pbr==5.5.1
    pip3 install wheel==0.36.2
    pip3 install twine==3.4.1
    ls -l ${BASE_PATH}/output
}
function upload()
{
    # 上传制品仓
    echo '''
    [distutils]
    index-servers = a8000
    [a8000]
    repository: https://pypi.cloudartifact.dgg.dragon.tools.huawei.com/artifactory/api/pypi/pypi-oss/
    username: oceanprotect_a8000
    password: Oceanprotect@8000
    ''' > ${BASE_PATH}/CI/.pypirc
    cat ${BASE_PATH}/CI/.pypirc
    pkg_a8000_name=$(ls ${BASE_PATH}/output/*.whl|awk -F '/' '{print $NF}')
    twine upload --config-file ${BASE_PATH}/CI/.pypirc -r a8000 ${BASE_PATH}/output/${pkg_a8000_name}
}
function uploadgovd()
{
	#file_path="./TARGET/jsonpatch-1.25-py2.py3-none-any.whl"
    file_path=$(ls ${BASE_PATH}/output/*.whl)
    md5value=$(md5sum ${file_path})
    read -ra md5value <<< "$md5value"
    sha1value=$(sha1sum ${file_path})
    read -ra sha1value <<< "$sha1value"
    sha256value=$(sha256sum ${file_path})
    read -ra sha256value <<< "$sha256value"
    cd ${CUR_PATH}/${soft_name}/
	group_id=$(ls ${BASE_PATH}/output/*.whl|awk -F '/' '{print $NF}'|awk -F '-' '{print $1}')
    pkg_version=$(ls ${BASE_PATH}/output/*.whl|awk -F '/' '{print $NF}'|awk -F '-' '{print $2}')
    codehub_repo=$(git remote -v|head -1|awk '{print $2}')
    codehub_tag=$(git describe --tags --abbrev=0)
    pkg_name=$(echo ${codehub_repo}|awk -F '/' '{print $NF}'|awk -F '.' '{print $1}')
    echo "group_id:          ${group_id}"
    echo "pkg_version:       ${pkg_version}"
    echo "codehub_repo:      ${codehub_repo}"
    echo "codehub_tag:       ${codehub_tag}"
    echo "pkg_name:          ${pkg_name}"
json_data(){
cat << EOF
    {
        "name": "${pkg_name}",
        "version": "${pkg_version}",
        "groupId": "${group_id}",
        "language": "python",
        "checkSum": [
            {
                "type": "md5",
                "value": "${md5value}"
            },
            {
                "type": "sha1",
                "value": "${sha1value}"
            },
            {
                "type": "sha256",
                "value": "${sha256value}"
            }
        ],
        "repoUrl": "${codehub_repo}",
        "tag": "${codehub_tag}"
    }
EOF
}
    curl "https://apigw-04.huawei.com/api/govd/publicservices/govdService/customModifyImportService/saveArtifactPkgMetadata" \
    -X POST \
    -d "$(json_data)" \
    -H "X-HW-ID: com.huawei.ipd.noproduct.tenant_g00317626" \
    -H "X-HW-APPKEY: nIXNFE+wNiMX4A4sKNvr+w==" \
    -H "Content-Type: application/json"
}
function upload_to_artifact(){
	set_env
	upload
	uploadgovd
}
echo "#########################################################"
echo "   Begin upload opensource "
echo "#########################################################"
upload_to_artifact
echo "#########################################################"
echo "   upload opensource Success  "
echo "#########################################################"
