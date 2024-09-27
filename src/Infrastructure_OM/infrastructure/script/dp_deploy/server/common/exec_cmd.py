import shlex
import subprocess
import os
from paramiko import SSHClient

from server.common.logger.logger import logger

node_name = os.getenv("NODE_NAME")
RUN_CMD_DEFAULT_TIMEOUT = 600
KUBECT_TIMEOUIT=60


def exec_cmd(cmd, timeout=RUN_CMD_DEFAULT_TIMEOUT):
    if type(cmd) != list:
        cmd_list = shlex.split(cmd)
    else:
        cmd_list = cmd
    process = subprocess.Popen(cmd_list, shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE, encoding="utf-8")
    stdout, stderr = process.communicate(timeout=timeout)
    return process.returncode, stdout, stderr


def exec_cmds_with_pipe(*cmds, timeout=RUN_CMD_DEFAULT_TIMEOUT):
    procs = []
    for k, cmd in enumerate(cmds):
        if k > 0:
            procs.append(subprocess.Popen(shlex.split(cmd),
                                          stdin=procs[k - 1].stdout, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                          shell=False))
        else:
            procs.append(subprocess.Popen(shlex.split(cmd),
                                          stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                          shell=False))
    stdout = ''
    try:
        for k, proc in enumerate(procs):
            if proc.returncode:
                stdout, stderr = proc.communicate(timeout=timeout)
                return 1, stderr.decode('utf-8')

            if k == len(procs) - 1:
                stdout, stderr = proc.communicate(timeout=timeout)
            else:
                proc.stdout.close()
    except Exception as ex:
        return 1, ex

    return 0, stdout.decode('utf-8').strip()


def ssh_exec_cmd(client: SSHClient, cmd):
    stdin, stdout, stderr = client.exec_command(cmd)
    stderr = stderr.read().decode()
    if stderr:
        logger.error(f"Failed to exec command {cmd}, error: {stderr}")
