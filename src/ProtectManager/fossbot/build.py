#!/usr/bin/python
# -*- coding: UTF-8 -*-
import os
import shutil

ROOT_DIR = 'ProtectManager'
TMP_DIR = '100P'
CODE_DIR = 'source'
PIP_EXTRA = ' -i  https://emei-il.huawei.com/nexus/repository/emei-pypi/simple --cache-dir=open_soft_python/ --trusted-host  emei-il.huawei.com --disable-pip-version-check --exists-action  i '
OPEN_SOFT_DIR_PYTHON = 'open_soft_python/'
OPEN_SOFT_DIR_JAVA = 'open_soft_java/'
OPEN_SOFT_DIR_INFRAS = 'open_soft_infras/'

MODULE_OPEN_SOFT_DIR_PARENT = 'module/'
MODULE_OPEN_SOFT_DIR_PYTHON = MODULE_OPEN_SOFT_DIR_PARENT + '%s/open_soft_python/'
MODULE_OPEN_SOFT_DIR_JAVA = '%s/open_soft_java/'
MODULE_OPEN_SOFT_LIST_FILE = MODULE_OPEN_SOFT_DIR_PARENT + 'binary_list.txt'
CMC_HGH_OPENSOURCE_PATH = 'https://cmc-hgh-artifactory.cmc.tools.huawei.com/artifactory/opensource_general'
CMC_RND_PYPI_PATH = 'https://cmc.centralrepo.rnd.huawei.com/pypi'


def run_system_cmd(cmd):
    print('run command:\t' + cmd)
    result = os.system(cmd)
    if 0 != result:
        raise RuntimeError


def make_root_dir():
    if os.path.isdir(TMP_DIR):
        shutil.rmtree(TMP_DIR, ignore_errors=True)

    if os.path.isdir(ROOT_DIR):
        print(f'{ROOT_DIR} is already exist.')
    else:
        run_system_cmd('mkdir ' + ROOT_DIR)
    os.chdir(ROOT_DIR)


def git_download():
    if os.path.isdir(CODE_DIR):
        shutil.rmtree(CODE_DIR, ignore_errors=True)

    run_system_cmd('mkdir ' + CODE_DIR)
    os.chdir(CODE_DIR)

    run_system_cmd('git clone ssh://git@codehub-dg-y.huawei.com:2222/dpa/protectmanager/PM_Boot_Dependencies.git')
    run_system_cmd('git clone ssh://git@codehub-dg-y.huawei.com:2222/dpa/protectmanager/PM_Common.git')
    run_system_cmd('git clone ssh://git@codehub-dg-y.huawei.com:2222/dpa/protectmanager/PM_System_Base_Common_Service')
    run_system_cmd('git clone ssh://git@codehub-dg-y.huawei.com:2222/dpa/protectmanager/PM_Data_Protection_Service')
    run_system_cmd('git clone ssh://git@codehub-dg-y.huawei.com:2222/dpa/protectmanager/PM_DataMover_Access_Point')
    run_system_cmd('git clone ssh://git@codehub-dg-y.huawei.com:2222/dpa/protectmanager/PM_GUI')
    run_system_cmd('git clone ssh://git@codehub-dg-y.huawei.com:2222/dpa/protectmanager/PM_API_Gateway')
    run_system_cmd('git clone ssh://git@codehub-dg-y.huawei.com:2222/dpa/protectmanager/PM_Copies_Catalog.git')
    run_system_cmd('git clone ssh://git@codehub-dg-y.huawei.com:2222/dpa/protectmanager/PM_Live_Mount_Manager.git')
    run_system_cmd('git clone ssh://git@codehub-dg-y.huawei.com:2222/dpa/protectmanager/PM_Resource_Lock_Manager.git')
    run_system_cmd('git clone ssh://git@codehub-dg-y.huawei.com:2222/dpa/protectmanager/PM_Resource_Manager.git')
    run_system_cmd('git clone ssh://git@codehub-dg-y.huawei.com:2222/dpa/protectmanager/PM_Database_Version_Migration.git')

    # run_system_cmd('git clone ssh://git@codehub-dg-y.huawei.com:2222/dpa/protectmanager/es_ms_global_search')

    os.chdir("../")


def find_python_open_soft_each(filepath, datas):
    file_names = os.listdir(filepath)  # 获取当前路径下的文件名，返回List
    for file in file_names:
        new_dir = filepath + '/' + file  # 将文件命加入到当前文件路径后面
        # if os.path.isdir(newDir): # 如果是文件夹
        if os.path.isfile(new_dir):  # 如果是文件
            if os.path.basename(new_dir) == "requirements":  # 判断是否是requirements
                print("pip requirement file:" + new_dir)
                f = open(new_dir)
                for line in f.readlines():
                    if line.strip() not in datas:
                        print("\t\tpip require for:" + line.strip())
                        datas.add(line.strip())  # 读文件
        else:
            find_python_open_soft_each(new_dir, datas)  # 如果不是文件，递归这个文件夹的路径


def find_python_open_soft():
    if os.path.isdir(OPEN_SOFT_DIR_PYTHON):
        shutil.rmtree(OPEN_SOFT_DIR_PYTHON, ignore_errors=True)

    requires = set()
    find_python_open_soft_each(CODE_DIR, requires)
    for require in requires:
        cmd = f'pip download {require} -d {OPEN_SOFT_DIR_PYTHON} {PIP_EXTRA}'
        print("pip download requirement:" + cmd)
        run_system_cmd(cmd)


def download_python_special_soft():
    gaussdb_url = 'https://cmc-szver-artifactory.cmc.tools.huawei.com/artifactory/cmc-software-release/' \
                  'Gauss100%20OLTP/Gauss100%20OLTP%20V100R003/Gauss100%20OLTP%20V100R003C10SPC119B110/' \
                  'euleros_v2.8_arm64/GaussDB-V100R003C10SPC119-aarch-64bit-ClientTools.tar.gz'
    run_system_cmd(f'wget {gaussdb_url}')

    ptvsd_url = 'https://cmc-ctu-artifactory.cmc.tools.huawei.com/artifactory/cmc-software-release/OceanStor%20100P/' \
                'ptvsd/5.0.0a12/ptvsd-5.0.0a12.tar.gz'
    run_system_cmd(f'wget {ptvsd_url}')


def download_python_central_soft():
    cmc_rnd_urls = (
        'Tempita/0.5.2/Tempita-0.5.2.tar.gz',
        'sqlparse/0.3.1/sqlparse-0.3.1.tar.gz',
        'decorator/4.4.2/decorator-4.4.2-py2.py3-none-any.whl',
        'pbr/5.4.4/pbr-5.4.4-py2.py3-none-any.whl',
        'sqlalchemy-migrate/0.13.0/sqlalchemy_migrate-0.13.0-py2.py3-none-any.whl',
        'certifi/2019.11.28/certifi-2019.11.28-py2.py3-none-any.whl',
        'chardet/3.0.4/chardet-3.0.4-py2.py3-none-any.whl',
        'idna/2.10/idna-2.10-py2.py3-none-any.whl',
        'urllib3/1.25.10/urllib3-1.25.10-py2.py3-none-any.whl'
    )
    cmc_rnd_urls_tmp = []
    for cmc_rnd_url in cmc_rnd_urls:
        cmc_rnd_urls_tmp.append(os.path.join(CMC_RND_PYPI_PATH, cmc_rnd_url))
    cmc_rnd_urls = set(cmc_rnd_urls_tmp)
    for rnd_url in cmc_rnd_urls:
        run_system_cmd(f'wget {rnd_url}')


def download_python_opensource_soft():
    cmc_hgh_file_urls = (
        'APScheduler/3.6.3/package/APScheduler-3.6.3-py2.py3-none-any.whl',
        'click/7.1.2/package/click-7.1.2-py2.py3-none-any.whl',
        'confluent-kafka-python/1.4.2/package/confluent-kafka-python-1.4.2.tar.gz',
        'confluent-kafka-python/1.4.2/package/confluent_kafka-1.4.2-cp38-cp38-manylinux1_i686.whl',
        'confluent-kafka-python/1.4.2/package/confluent_kafka-1.4.2-cp38-cp38-manylinux1_x86_64.whl',
        'SQLAlchemy/1.3.15/package/SQLAlchemy-1.3.15.tar.gz',
        'sqlalchemy-utils/0.36.3/package/sqlalchemy-utils-0.36.3.tar.gz',
        'FastAPI/0.57.0/package/fastapi-0.57.0-py3-none-any.whl',
        'pydantic/1.8.2/package/pydantic-1.8.2.tar.gz',
        'PyJWT/1.7.1/package/PyJWT-1.7.1-py2.py3-none-any.whl',
        'python-json-logger/2.0.1/package/python-json-logger-2.0.1.tar.gz',
        'pytz/2020.1/package/pytz-2020.1-py2.py3-none-any.whl',
        'redis-py/3.4.1/package/redis-py-3.4.1.tar.gz',
        'requests/2.23.0/package/requests-2.23.0-py2.py3-none-any.whl',
        'six/1.15.0/package/six-1.15.0-py2.py3-none-any.whl',
        'starlette/0.13.4/package/starlette-0.13.4-py3-none-any.whl',
        'tzlocal/2.1/package/tzlocal-2.1.tar.gz',
        'h11/v0.9.0/package/h11-0.9.0-py2.py3-none-any.whl',
        'websockets/9.1/package/websockets-9.1.tar.gz',
        'httptools/0.1.1/package/httptools-0.1.1_1.tar.gz',
        'uvloop/0.14.0/package/uvloop-0.14.0.tar.gz',
        'uvicorn/0.12.3/package/uvicorn-0.12.3-py3-none-any.whl',
        'psycopg/2.8.5/package/psycopg2-2.8.5.tar.gz',
        'librdkafka/1.4.2/package/librdkafka-1.4.2.tar.gz',
        'pyvmomi/6.7.3/package/pyvmomi-6.7.3.tar.gz',
        'watchdog/0.10.3/package/watchdog-0.10.3.tar.gz',
        'pathtools/0.1.2/package/pathtools-0.1.2.tar.gz',
        'aiohttp/3.5.4/package/aiohttp-3.5.4.tar.gz'
    )

    cmc_hgh_file_urls_tmp = []
    for cmc_hgh_url in cmc_hgh_file_urls:
        cmc_hgh_file_urls_tmp.append(os.path.join(CMC_HGH_OPENSOURCE_PATH, cmc_hgh_url))
    cmc_hgh_file_urls = set(cmc_hgh_file_urls_tmp)

    if os.path.isdir(OPEN_SOFT_DIR_PYTHON):
        shutil.rmtree(OPEN_SOFT_DIR_PYTHON, ignore_errors=True)

    run_system_cmd('mkdir ' + OPEN_SOFT_DIR_PYTHON)
    os.chdir(OPEN_SOFT_DIR_PYTHON)

    for hgh_url in cmc_hgh_file_urls:
        run_system_cmd(f'wget {hgh_url}')

    download_python_central_soft()
    download_python_special_soft()

    os.chdir("..")


# 校验pom文件是否需要被处理
def valid_pom(pomfile):
    if pomfile.find("Gen_Api") >= 0 or pomfile.find("integration") >= 0:
        return False
    else:
        return True


def find_java_open_soft_each(filepath, mvn_output_path, setting_xml):
    file_names = os.listdir(filepath)  # 获取当前路径下的文件名，返回List
    for file in file_names:
        new_dir = filepath + '/' + file  # 将文件命加入到当前文件路径后面
        # if os.path.isdir(newDir): # 如果是文件夹
        if os.path.isfile(new_dir):  # 如果是文件
            if os.path.basename(new_dir) == "pom.xml" and valid_pom(new_dir):  # 判断是否是pom.xml
                print("mvn dependency file:" + new_dir)
                cmd = f'mvn dependency:copy-dependencies -DoutputDirectory={mvn_output_path} -Dmaven.test.skip=true -DincludeScope=compile -Preal -f {new_dir} -s {setting_xml}'
                print("\t\tmvn execute:" + cmd)
                run_system_cmd(cmd)
        else:
            find_java_open_soft_each(new_dir, mvn_output_path, setting_xml)  # 如果不是文件，递归这个文件夹的路径


def find_java_open_soft():
    if os.path.isdir(OPEN_SOFT_DIR_JAVA):
        shutil.rmtree(OPEN_SOFT_DIR_JAVA, ignore_errors=True)
    MVN_JAVA_PATH = os.path.join(os.getcwd(), OPEN_SOFT_DIR_JAVA)
    MVN_SETTING_XML = os.path.join(os.getcwd(), "..", "settings.xml")
    find_java_open_soft_each(CODE_DIR, MVN_JAVA_PATH, MVN_SETTING_XML)


def mvn_install_and_copy_thridsoft():
    base_dir = os.getcwd()
    if os.path.isdir(OPEN_SOFT_DIR_JAVA):
        shutil.rmtree(OPEN_SOFT_DIR_JAVA, ignore_errors=True)
    MVN_JAVA_PATH = os.path.join(os.getcwd(), OPEN_SOFT_DIR_JAVA)
    run_system_cmd(f'mkdir -p {OPEN_SOFT_DIR_JAVA}')
    MVN_SETTING_XML = os.path.join(os.getcwd(), "..", "settings.xml")

    common_path = f"{base_dir}/{CODE_DIR}/PM_Boot_Dependencies/"
    os.chdir(common_path)
    run_system_cmd(f'mvn -Preal clean        -Dmaven.test.skip=true     -s {MVN_SETTING_XML}')
    run_system_cmd(f'mvn -Preal install -nsu -Dmaven.test.skip=true     -s {MVN_SETTING_XML}')

    common_path = f"{base_dir}/{CODE_DIR}/PM_Common/"
    os.chdir(common_path)
    run_system_cmd(f'mvn -Preal install -nsu -Dmaven.test.skip=true     -s {MVN_SETTING_XML}')
    run_system_cmd(f'cp -rf {common_path}/target/Common-*.jar {MVN_JAVA_PATH}')
    run_system_cmd(f'mvn -Preal clean        -Dmaven.test.skip=true     -s {MVN_SETTING_XML}')

    gui_path = f"{base_dir}/{CODE_DIR}/PM_GUI/src/service"
    os.chdir(gui_path)
    run_system_cmd(f'mvn -Preal install -nsu -Dmaven.test.skip=true     -s {MVN_SETTING_XML}')
    run_system_cmd(f'cp -rf {gui_path}/target/thirdPartSoft/* {MVN_JAVA_PATH}')
    run_system_cmd(f'mvn -Preal clean        -Dmaven.test.skip=true     -s {MVN_SETTING_XML}')

    system_base_path = f"{base_dir}/{CODE_DIR}/PM_System_Base_Common_Service/src/"
    os.chdir(system_base_path)
    run_system_cmd(f'mvn -Preal install -nsu -Dmaven.test.skip=true     -s {MVN_SETTING_XML}')
    run_system_cmd(f'cp -rf {system_base_path}/MainServer/target/thirdPartSoft/* {MVN_JAVA_PATH}')
    run_system_cmd(f'mvn -Preal clean        -Dmaven.test.skip=true     -s {MVN_SETTING_XML}')

    dm_access_path = f"{base_dir}/{CODE_DIR}/PM_DataMover_Access_Point/src/"
    os.chdir(dm_access_path)
    run_system_cmd(f'mvn -Preal install -nsu -Dmaven.test.skip=true     -s {MVN_SETTING_XML}')
    run_system_cmd(f'cp -rf {dm_access_path}/DataMoverMain/target/thirdPartSoft/* {MVN_JAVA_PATH}')
    run_system_cmd(f'mvn -Preal clean        -Dmaven.test.skip=true     -s {MVN_SETTING_XML}')

    live_mount_path = f"{base_dir}/{CODE_DIR}/PM_Live_Mount_Manager/src/"
    os.chdir(live_mount_path)
    run_system_cmd(f'mvn -Preal install -nsu -Dmaven.test.skip=true     -s {MVN_SETTING_XML}')
    run_system_cmd(f'cp -rf {live_mount_path}/Live_Mount_Main/target/thirdPartSoft/* {MVN_JAVA_PATH}')
    run_system_cmd(f'mvn -Preal clean        -Dmaven.test.skip=true     -s {MVN_SETTING_XML}')

    # 删除自研jar包
    run_system_cmd(f'rm -rf {MVN_JAVA_PATH}/*-0.0.1.jar')
    run_system_cmd(f'rm -rf {MVN_JAVA_PATH}/*kmc-*.jar')
    os.chdir(base_dir)


def download_infras_opensource_soft():
    urls = [
        'https://emei-il.huawei.com/nexus/repository/files/3rd-party/kafka_2.12-2.4.0.tgz',
        'https://emei-il.huawei.com/nexus/repository/files/3rd-party/kafkacat-1.5.0.tar.bz2',
        'https://emei-il.huawei.com/nexus/repository/files/3rd-party/kafka-connect-elasticsearch-5.4.0.tgz',
        'https://emei-il.huawei.com/nexus/repository/files/3rd-party/kubernetes-client-linux-v1.17.4-arm64.tar.gz',
        'https://emei-il.huawei.com/nexus/repository/files/3rd-party/elastic/elasticsearch-7.6.2-no-jdk-linux-aarch64.tar.gz',
        'https://emei-il.huawei.com/nexus/repository/files/3rd-party/elastic/elasticsearch-analysis-ik-7.6.2.zip',
        'https://emei-il.huawei.com/nexus/repository/files/3rd-party/elastic/kibana-7.6.2-linux-aarch64.tar.gz',
        'https://emei-il.huawei.com/nexus/repository/files/3rd-party/elastic/OpenJDK13U-jre_aarch64_linux_hotspot_13.0.2_8.tar.gz',
        'https://emei-il.huawei.com/nexus/repository/files/3rd-party/elastic/node-v10.19.0-linux-arm64.tar.gz',
        'https://emei-il.huawei.com/nexus/repository/files/3rd-party/elastic/dumb-init-v1.2.2.tar.gz',
        'https://cmc-hgh-artifactory.cmc.tools.huawei.com/artifactory/opensource_general/Apache-ZooKeeper/3.4.13/package/zookeeper-release-3.4.13.zip',
        'https://cmc-hgh-artifactory.cmc.tools.huawei.com/artifactory/opensource_general/redis/5.0.8/package/redis-5.0.8.tar.gz',
    ]

    if os.path.isdir(OPEN_SOFT_DIR_INFRAS):
        shutil.rmtree(OPEN_SOFT_DIR_INFRAS, ignore_errors=True)

    run_system_cmd('mkdir ' + OPEN_SOFT_DIR_INFRAS)
    os.chdir(OPEN_SOFT_DIR_INFRAS)

    for url in urls:
        run_system_cmd(f'wget {url}')

    os.chdir("..")


def gen_mvn_dependency_tree_each(filepath, export_path, setting_xml, temp):
    file_names = os.listdir(filepath)  # 获取当前路径下的文件名，返回List
    for file in file_names:
        new_dir = filepath + '/' + file  # 将文件命加入到当前文件路径后面
        # if os.path.isdir(newDir): # 如果是文件夹
        if os.path.isfile(new_dir):  # 如果是文件
            if os.path.basename(new_dir) == "pom.xml" and valid_pom(new_dir):  # 判断是否是pom.xml
                print("mvn dependency file:" + new_dir)
                temp[0] = temp[0] + 1
                depenTree = os.path.join(export_path, new_dir.split('/')[1] + '-' + str(temp[0]) + '.tree')
                cmd = f'mvn dependency:tree -Dmaven.test.skip=true -DincludeScope=compile -Preal -f {new_dir} -s {setting_xml} > {depenTree}'
                print("\t\tmvn execute:" + cmd)
                run_system_cmd(cmd)
        else:
            gen_mvn_dependency_tree_each(new_dir, export_path, setting_xml, temp)  # 如果不是文件，递归这个文件夹的路径


def gen_mvn_dependency_tree():
    MVN_SETTING_XML = os.path.abspath(os.path.join(os.getcwd(), "..", "settings.xml"))
    gen_mvn_dependency_tree_each(CODE_DIR, os.path.join(os.getcwd(), MODULE_OPEN_SOFT_DIR_PARENT), MVN_SETTING_XML, [0])


def delet_other_files():
    file_names = os.listdir(os.getcwd())  # 获取当前路径下的文件名，返回List
    for file in file_names:
        if os.path.basename(file).startswith("="):  # 判断是否垃圾文件
            print("delete rubbish file:\t" + file)
            os.remove(file)


def module_find_python_open_soft_each(filepath):
    file_names = os.listdir(filepath)  # 获取当前路径下的文件名，返回List
    for file in file_names:
        new_dir = filepath + '/' + file  # 将文件命加入到当前文件路径后面
        # if os.path.isdir(newDir): # 如果是文件夹
        if os.path.isfile(new_dir):  # 如果是文件
            if os.path.basename(new_dir) == "requirements":  # 判断是否是requirements
                print("pip requirement file:" + new_dir)
                f = open(new_dir)
                for line in f.readlines():
                    require = line.strip()
                    module = new_dir.split('/')[1]
                    export_dir = MODULE_OPEN_SOFT_DIR_PYTHON % module
                    cmd = f'pip download {require} -d {export_dir} {PIP_EXTRA}'
                    print("pip download requirement:" + cmd)
                    run_system_cmd(cmd)
        else:
            module_find_python_open_soft_each(new_dir)  # 如果不是文件，递归这个文件夹的路径


# 按模块导出python依赖
def module_find_python_open_soft():
    module_find_python_open_soft_each(CODE_DIR)


def module_find_java_open_soft_each(filepath, mvn_output_path, setting_xml):
    file_names = os.listdir(filepath)  # 获取当前路径下的文件名，返回List
    for file in file_names:
        new_dir = filepath + '/' + file  # 将文件命加入到当前文件路径后面
        # if os.path.isdir(newDir): # 如果是文件夹
        if os.path.isfile(new_dir):  # 如果是文件
            if os.path.basename(new_dir) == "pom.xml" and valid_pom(new_dir):  # 判断是否是pom.xml
                print("mvn dependency file:" + new_dir)
                module = new_dir.split('/')[1]
                export_dir = MODULE_OPEN_SOFT_DIR_JAVA % module
                export_dir = mvn_output_path + export_dir
                cmd = f'mvn dependency:copy-dependencies -DoutputDirectory={export_dir} -Dmaven.test.skip=true -DincludeScope=compile -Preal -f {new_dir} -s {setting_xml}'
                print("\t\tmvn execute:" + cmd)
                run_system_cmd(cmd)
        else:
            module_find_java_open_soft_each(new_dir, mvn_output_path, setting_xml)  # 如果不是文件，递归这个文件夹的路径


# 按模块导出java依赖
def module_find_java_open_soft():
    MVN_JAVA_PATH = os.path.join(os.getcwd(), MODULE_OPEN_SOFT_DIR_PARENT)
    MVN_SETTING_XML = os.path.join(os.getcwd(), "..", "settings.xml")
    module_find_java_open_soft_each(CODE_DIR, MVN_JAVA_PATH, MVN_SETTING_XML)


def module_file_list_each(filepath, file_open):
    file_names = os.listdir(filepath)  # 获取当前路径下的文件名，返回List
    for file in file_names:
        new_dir = filepath + '/' + file  # 将文件命加入到当前文件路径后面
        if os.path.isfile(new_dir):  # 如果是文件
            print(new_dir)
            file_open.write(new_dir + '\n')
        else:
            module_file_list_each(new_dir, file_open)


# 按模块导出依赖
def module_file_list():
    if os.path.isdir(MODULE_OPEN_SOFT_DIR_PARENT):
        shutil.rmtree(MODULE_OPEN_SOFT_DIR_PARENT, ignore_errors=True)
    module_find_python_open_soft()
    module_find_java_open_soft()
    delet_other_files()
    if os.path.isfile(MODULE_OPEN_SOFT_LIST_FILE):
        os.remove(MODULE_OPEN_SOFT_LIST_FILE)
    with open(MODULE_OPEN_SOFT_LIST_FILE, mode='w') as fileOpen:
        module_file_list_each(MODULE_OPEN_SOFT_DIR_PARENT, fileOpen)


def gen_module_open_soft():
    dir = ""
    binary_title = None
    binary_code_map = {}
    with open(dir + 'binary.txt', mode='r', encoding='UTF-8') as file:
        index = 0
        for line in file.readlines():
            index += 1
            if index <= 1:
                binary_title = line.strip()
            else:
                strs = line.strip().split('\t')
                if len(strs) > 6:
                    binary_code_map[strs[2]] = strs[6]
    code_soft_map = {}
    with open(dir + 'softDetails.txt', mode='r', encoding='UTF-8') as file:
        index = 0
        for line in file.readlines():
            index += 1
            if index <= 1:
                binary_title = line.strip()
            else:
                strs = line.strip().split('\t')
                if len(strs) > 4:
                    code_soft_map[strs[4]] = line.strip()

    with open(dir + 'binary_list.txt', mode='r', encoding='UTF-8') as listFile:
        with open(dir + 'export.txt', mode='w', encoding='UTF-8') as fileOpen:
            index = 0
            for line in listFile.readlines():
                index += 1
                if index <= 1:
                    fileOpen.write('模块' + '\t' + '文件' + '\t' + binary_title + '\n')
                else:
                    strs = line.strip().split('/')
                    filename = strs[len(strs) - 1]
                    module = strs[2]
                    if filename in binary_code_map:
                        # 能匹配
                        if binary_code_map[filename] != '':
                            fileOpen.write(
                                module + '\t' + line.strip() + '\t' + code_soft_map[binary_code_map[filename]] + '\n')
                        # 匹配不了
                        else:
                            fileOpen.write(module + '\t' + line.strip() + '\n')
                    # 找不到二进制
                    else:
                        print('找不到二进制:\t' + module + '\t' + line.strip())


def statistic():
    dir = ""
    binary_map = {}
    module_count = {}
    with open(dir + '/export.txt', mode='r', encoding='UTF-8') as file:
        index = 0
        for line in file.readlines():
            index += 1
            if index <= 1:
                print("")
            else:
                strs = line.strip().split('\t')
                module = strs[0]
                file_dirs = strs[1].split('/')
                binary_file = file_dirs[len(file_dirs) - 1]
                rank = ''
                if len(strs) > 11:
                    rank = strs[11]
                if 'DXX' == rank or '' == rank:
                    binary_map[binary_file] = line.strip()
                    if module in module_count:
                        module_count[module] = module_count[module] + 1
                    else:
                        module_count[module] = 1

    for key, value in binary_map.items():
        print(key + "\t" + value)
    print("\n")
    print("all\t" + str(len(binary_map)))
    for key, value in module_count.items():
        print(key + "\t" + str(value))


def gen_opensource_soft():
    # 下载源码
    git_download()
    # python下载二进制
    # find_python_open_soft()
    download_python_opensource_soft()
    # java下载二进制
    # find_java_open_soft()
    mvn_install_and_copy_thridsoft()
    # 下载基础组件开源软件
    # download_infras_opensource_soft()
    # 删除垃圾文件
    delet_other_files()


if __name__ == "__main__":
    print('start build:' + os.getcwd())
    # 创建根目录
    make_root_dir()
    # 下载开源二进制依赖，供Fossbot扫描
    gen_opensource_soft()

    # 按模块导出二进制依赖
    # module_file_list()
    # 导出maven依赖树
    # gen_mvn_dependency_tree()
    # 模块二进制情况列表-本地
    # gen_module_open_soft()
    # 统计二进制缺陷-本地
    # statistic()
