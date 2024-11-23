#
# This file is a part of the open-eBackup project.
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at
# http://mozilla.org/MPL/2.0/.
#
# Copyright (c) [2024] Huawei Technologies Co.,Ltd.
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
#

import os
import shlex
import shutil
import signal
import subprocess
from time import sleep

from tdsql.common.util import get_tdsql_config
from tdsql.logger import log


def live_mount(mount_path, mysql_port, cnf_path, pool_size, mysql_version):
    log.info("begin to make directories")
    alive_cnf = f"{mount_path}/temp_alive/alive.cnf"
    cmd = f"mkdir -p {mount_path}/temp_alive ; mkdir -p {mount_path}/temp_alive/logbin ; \
            mkdir -p {mount_path}/temp_alive/relaylog ; cp {cnf_path} {alive_cnf}"

    ret, output = subprocess.getstatusoutput(cmd)
    if ret:
        log.error(f"Exec to {cmd}, the output detail is {output}.")
        return False

    log.info(f"begin to prepare alive config file {cnf_path}")
    cmd = generate_live_mount_cmd(mount_path, mysql_port, pool_size, alive_cnf, mysql_version)
    ret, output = subprocess.getstatusoutput(cmd)
    if ret:
        log.error(f"Exec to {cmd}, the output detail is {output}.")
        return False

    cmd = f"chown -R tdsql {mount_path}"
    ret, output = subprocess.getstatusoutput(cmd)
    if ret:
        log.error(f"Exec to {cmd}, the output detail is {output}.")
        return False

    basedir = cnf_path[:cnf_path.rindex("/")]
    basedir = basedir[:basedir.rindex("/")]
    cmd = f"{basedir}/bin/mysqld --defaults-file={alive_cnf}"
    process = subprocess.Popen(shlex.split(cmd), stdout=subprocess.PIPE, stderr=subprocess.PIPE, encoding="utf-8")
    err_data = process.stderr.read()
    log.warn(f"Exec to {cmd}, the output detail is {err_data}.")
    return True


def generate_live_mount_cmd(mount_path, mysql_port, pool_size, alive_cnf, mysql_version):
    cmd = f"sed -i 's#innodb_data_home_dir.*$#innodb_data_home_dir={mount_path}#g' {alive_cnf} ; \
            sed -i 's#datadir.*$#datadir={mount_path}#g' {alive_cnf} ; \
            sed -i 's#log-bin.*$#log-bin={mount_path}/temp_alive/logbin#g' {alive_cnf} ; \
            sed -i 's#relay-log.*$#relay-log={mount_path}/temp_alive/relaylog#g' {alive_cnf} ; \
            sed -i 's#innodb_log_group_home_dir.*$#innodb_log_group_home_dir={mount_path}/temp_alive#g' {alive_cnf} ; \
            sed -i 's#tmpdir.*$#tmpdir={mount_path}/temp_alive#g' {alive_cnf} ; \
            sed -i 's#innodb_data_home_dir.*$#innodb_data_home_dir={mount_path}#g' {alive_cnf} ; \
            sed -i 's#tmpdir.*$#tmpdir={mount_path}/temp_alive#g' {alive_cnf} ; \
            sed -i 's#^[ ]*port.*$#port={mysql_port}#g' {alive_cnf} ; \
            sed -i '/admin_port/d' {alive_cnf} ; \
            sed -i '/report-host/d' {alive_cnf} ; \
            sed -i '/admin_address/d' {alive_cnf} ; \
            sed -i 's#pid-file.*$#pid-file={mount_path}/temp_alive/mysql.pid#g' {alive_cnf}  ; \
            sed -i 's#socket.*$#socket=/tmp/alive_{mysql_port}.sock#g' {alive_cnf} ; \
            sed -i 's#log-error.*$#log-error={mount_path}/temp_alive/mysqld.err#g' {alive_cnf} ; \
            sed -i '/!include/d' {alive_cnf} ; \
            echo character_set_server=utf8 >> {alive_cnf} ; \
            echo collation_server=utf8_general_ci >> {alive_cnf} ; \
            echo lower_case_table_names=1 >> {alive_cnf} ; \
            echo innodb_buffer_pool_size={pool_size}M >> {alive_cnf}  ; \
            echo sqlasyn=off >> {alive_cnf} ; \
            echo sqlasync_group_slave_ack=off >> {alive_cnf} ;"
    tdsql_config = get_tdsql_config()
    live_mount_conf = tdsql_config.get('liveMountConf').get(mysql_version)
    log.info(f"live_mount_conf {live_mount_conf}")
    for conf in live_mount_conf:
        cmd = cmd + conf + f" {alive_cnf} ;"
    return cmd


def cancel_live_mount(mount_path, job_pid, job_id, mysql_port):
    log.info(f"cancel livemount path is {mount_path}")
    cmd = f"ps -ef | grep '{mount_path}' |grep 'mysqld' |grep -v grep"
    ret, output = subprocess.getstatusoutput(cmd)
    if ret:
        log.warn(f"failed to get mysqld status")
    else:
        for line in output.splitlines():
            pid = int(line.split()[1])
            log.info(f"os.kill {pid} pid:{job_pid} jobId {job_id}")
            os.kill(pid, signal.SIGKILL)
            sleep(10)

    try:
        shutil.rmtree(os.path.join(mount_path, 'temp_alive'))
    except Exception as ex:
        log.error(f"cancel livemount exception is {ex}")

    if os.path.exists(f"/tmp/alive_{mysql_port}.sock"):
        os.remove(f"/tmp/alive_{mysql_port}.sock")
    return True
