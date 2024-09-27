#  Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
import argparse
import json
import logging
import os
import shlex
import stat
import sys
import subprocess
import ipaddress
import time
from datetime import datetime

ALLOWED_FUNCTIONS = ["config_mgt_net", "config_biz_net", "is_100ge_card_available"]
LOG_FILE = "/var/log/init_network.log"
MY_NAME = os.path.basename(__file__)
LOGGER = None
TMP_FILE_SUFFIX = ".init_network.tmp"

BUS_INFO_0 = "0000:83:00.0"
BUS_INFO_1 = "0000:84:00.0"

USAGE = '''
Usage:
{name:s} command [--args]

Available Commands:
    config_mgt_net             config management network eth0 and k8s nodeIP
    config_biz_net             config business network eth2/eth3
    is_100ge_card_available    valid 100GE network card
Args:
    ip_address                 ip address, only IPv4 format supported
    netmask                    netmask, CIDR format unsupported
    gateway                    gateway, required with config_mgt_net command
    vlan_id                    1-4094, only config_biz_net command supported
    interface                  eth2/eth3, only config_biz_net command supported
    '''.format(name=MY_NAME)


class ErrorCode:
    UNKNOWN_ERROR = 10001
    WRONG_USAGE = 10002
    ILLEGAL_FUNC_NAME = 10003
    PARAM_PARSE_ERROR = 10004
    TIMEOUT_ERROR = 10005
    EXECUTE_ERROR = 10006
    NET_CARD_NOT_EXIST = 10011
    UNSUPPORTED_CPU = 10012
    UNSUPPORTED_OS = 10013
    NET_CARD_NOT_AT_CORRECT_SLOT = 10014
    NET_CARD_DRIVER_NOT_CORRECT_INSTALLED = 10015
    INVALID_VLAN_ID = 10021
    INVALID_IP = 10022
    INVALID_NETMASK = 10023
    OCCUPIED_IP = 10024
    IP_CONFLICT_WITH_CLUSTER = 10025
    NOT_BONDED_ERROR = 10026
    REWRITE_ERROR = 10027
    ACTIVE_FAILED = 10028
    UPDATE_MGT_IP_FAILED = 10031
    K8S_ERROR = 10032


class CustomResult:
    def __init__(self, begin_time=None, end_time=None, err_msg=None, method_name=None, data=None, status=0):
        self.begin_time = begin_time
        self.end_time = end_time
        self.err_msg = err_msg
        self.method_name = method_name
        self.data = data
        self.status = status

    def __str__(self):
        return (f"Begin Time: {self.begin_time}, End Time: {self.end_time}, Method Name: {self.method_name}, "
                f"Data: {self.data}, Status: {self.status}, Error Message: {self.err_msg}")


class MyExecuteException(Exception):
    def __init__(self, error_info, execute_result: CustomResult):
        super(MyExecuteException, self).__init__(error_info)
        self.error_info = error_info
        self.execute_result = execute_result

    def __str__(self):
        return self.error_info


class CustomArgumentParser(argparse.ArgumentParser):
    def error(self, message):
        """重写error方法，自定义错误处理逻辑。"""
        LOGGER.error("param parse error: %s", message)
        _construct_result(f"param parse error: {message}", ErrorCode.PARAM_PARSE_ERROR)


# 定义全局变量result
custom_result = CustomResult()


def _new_logger(log_path):
    """
    日志构造器：仅将日志记录于LOG_FILE，不输出至控制台
    :param log_path: LOG_PATH
    :return: logger
    """
    formatter = logging.Formatter("[%(asctime)s][%(levelname)s][%(message)s][%(filename)s, %(lineno)d]")
    # 日志输出至文件handler
    handler = logging.FileHandler(log_path)
    handler.setFormatter(formatter)
    logger = logging.getLogger()
    # 添加文件handler
    logger.addHandler(handler)
    # 记录INFO及之上级别的日志
    logger.setLevel(logging.INFO)
    return logger


def _get_datetime_by_timezone():
    """
    根据当前时区获取时间，精确至毫秒
    :return: datetime
    """
    return datetime.now(tz=datetime.utcnow().astimezone().tzinfo).strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]


def _is_valid_ipv4(ips):
    """
    校验IPv4地址是否有效
    :param ips: ips
    :return: None
    """
    for ip in ips:
        try:
            ipaddress.IPv4Address(ip)
        except ipaddress.AddressValueError:
            LOGGER.error("%s is not a valid IPv4", ip)
            _construct_result(f"{ip} is not a valid IPv4", ErrorCode.INVALID_IP)


def _is_valid_netmask(netmask):
    """
    校验子网掩码是否合法
    :param netmask:  netmask
    :return: True or False
    """
    # 分割子网掩码为四个部分，并验证数量和值范围
    mask_fragments = netmask.split('.')
    if len(mask_fragments) != 4 or not all(0 <= int(fragment) <= 255 for fragment in mask_fragments):
        return False
    # 将每个片段转换为二进制字符串后拼接
    binary_subnet = ''.join(f'{int(fragment):08b}' for fragment in mask_fragments)
    # 如果存在01或全0的情况，则不合法
    if binary_subnet.find('01') != -1 or binary_subnet.find('1') == -1:
        return False
    return True


def _is_ip_conflict(ip):
    """
    校验IP是否冲突
    :param ip: ip
    :return: None
    """
    net_card = _execute_piped_command(f"ip a | grep {ip}" + " | awk '{print $NF}'")
    if net_card:
        LOGGER.error("'%s' have been occupied by '%s'", ip, net_card)
        _construct_result(f"{ip} have been occupied by {net_card}", ErrorCode.OCCUPIED_IP)
    response = os.system(f"ping -c 1 -W 1 {ip} > /dev/null 2>&1")
    if response == 0:
        LOGGER.error("'%s' have been occupied", ip)
        _construct_result(f"{ip} have been occupied", ErrorCode.OCCUPIED_IP)


def _update_network_config(interface, ip, netmask, gateway=None, vlan_id=None):
    """
    修改指定网络接口的IP地址、子网掩码和默认网关配置文件
    :param interface: interface
    :param ip: ip
    :param netmask: netmask
    :param gateway: gateway
    :param vlan_id: vlan_id
    :return: None
    """
    base_contents = [
        f"DEVICE={interface}",
        "BONDING_MASTER=yes",
        "STARTMODE=auto",
        "BONDING_OPTS='mode=1 miimon=200'",
        "BOOTPROTO=static",
        "ONBOOT=yes"
    ]
    if vlan_id:
        # 刷新子接口网络配置文件
        vlan_contents = [
            f"DEVICE={interface}.{str(vlan_id)}",
            "ONBOOT=yes",
            "VLAN=yes",
            "BOOTPROTO=static",
            "STARTMODE=auto",
            f"IPADDR={ip}",
            f"NETMASK={netmask}"
        ]
        if gateway:
            vlan_contents.append(f"GATEWAY={gateway}")
        _re_write_file(interface + "." + str(vlan_id), vlan_contents)
    else:
        # 构造对应文件内容
        base_contents.append(f"IPADDR={ip}")
        base_contents.append(f"NETMASK={netmask}")
        if gateway:
            base_contents.append(f"GATEWAY={gateway}")
    # 刷新主接口网络配置文件
    _re_write_file(interface, base_contents)


def _re_write_file(file_name, contents):
    """
    以644权限创建新文件并写入contents
    :param file_name: file_name
    :param contents: contents
    :return: None
    """
    # step1:新建临时文件
    temp_file_name = f'/etc/sysconfig/network-scripts/ifcfg-{file_name}{TMP_FILE_SUFFIX}'
    if os.path.exists(temp_file_name):
        os.remove(temp_file_name)
    # 只写 | 新建 | 存在则报错
    flags = os.O_WRONLY | os.O_CREAT | os.O_EXCL
    # 权限为644
    modes = stat.S_IROTH | stat.S_IWUSR | stat.S_IRUSR | stat.S_IRGRP
    with os.fdopen(os.open(temp_file_name, flags, modes), 'w', encoding='utf-8') as out:
        for content in contents:
            out.write(content + os.linesep)
    # step2：用临时文件覆盖旧文件并保留旧文件备份
    try:
        _execute_piped_command(f"mv -f -b {temp_file_name} {temp_file_name.replace(TMP_FILE_SUFFIX, '')}")
    except MyExecuteException as error:
        LOGGER.error("rewrite %s failed: %s", temp_file_name.replace(TMP_FILE_SUFFIX, ''), error.error_info)
        _construct_result(f"rewrite {temp_file_name.replace(TMP_FILE_SUFFIX, '')} failed: {error.error_info}",
                          ErrorCode.REWRITE_ERROR)


def config_mgt_net(args):
    """
    配置管理面网络
    :param args: args
    :return: None
    """
    mgt_ip = args.ip_address
    mgt_netmask = args.netmask
    mgt_gateway = args.gateway
    interface = "eth0"
    # 参数合法性检查
    LOGGER.info("start valid params")
    _is_valid_ipv4([mgt_ip, mgt_netmask, mgt_gateway])
    LOGGER.info("validated ipv4 format")
    if not _is_valid_netmask(mgt_netmask):
        LOGGER.error("Invalid netmask format: %s", mgt_netmask)
        _construct_result(f"Invalid netmask format: {mgt_netmask}", ErrorCode.INVALID_NETMASK)
    LOGGER.info("validated netmask format")
    # ip冲突检查
    _is_ip_conflict(mgt_ip)
    LOGGER.info("validated ip address")
    # 网段冲突检查
    _is_same_subnet(mgt_ip, mgt_netmask, interface)
    LOGGER.info("validated subnet address")
    # 先查询是否bond了该interface
    bond_status = _execute_piped_command("nmcli connection | grep -i 'Bond eth0'")
    if len(bond_status) == 0:
        LOGGER.error("Please bond the service port [eth0] first")
        _construct_result("Please bond the service port [eth0] first", ErrorCode.NOT_BONDED_ERROR)
    LOGGER.info("bond eth0 exist")
    LOGGER.info("step1: config k8s nodeIP")
    _config_node_ip(mgt_ip)
    LOGGER.info("step2: config eth0 network")
    # 修改对应网卡参数
    _update_network_config(interface=interface, ip=mgt_ip, netmask=mgt_netmask, gateway=mgt_gateway)
    # 重启网卡并检查网卡状态
    _active_network(interface=interface, ip=mgt_ip)
    LOGGER.info("eth0 config success")


def _config_node_ip(node_ip):
    """
    修改容器nodeIP
    :param node_ip: node_ip
    :return: None
    """
    # 修改前检查容器运行状态
    pod_before_status = _execute_piped_command("kubectl get sts -n dpa | grep -w '1/1' | wc -l")
    LOGGER.info("k8s pods running as %s before update management IP", pod_before_status)
    if pod_before_status != '5':
        LOGGER.warning("k8s pods running status is abnormal")
    # 修改traefik的管理ip为mgt_ip;需先修改容器管理IP，成功后修改eth0，否则连接断开，若修改容器管理IP失败，调用方无法获取错误信息
    _execute_piped_command(f"smartkube set managementIP --nodeIP=node-0={node_ip}")
    # 检查容器管理ip修改结果
    traefik_result = _execute_piped_command(f"cat /opt/k8s/run/conf/traefik/traefik.yaml | grep '{node_ip}' | wc -l")
    if str(traefik_result) != '2':
        LOGGER.error("Update the management IP of traefik failed")
        _construct_result("Update the management IP of traefik failed", ErrorCode.UPDATE_MGT_IP_FAILED)
    LOGGER.info("k8s nodeIP config success")
    # 修改后检查容器运行状态
    pod_after_status = _execute_piped_command("kubectl get sts -n dpa | grep -w '1/1' | wc -l")
    LOGGER.info("k8s pods running as %s before after management IP", pod_before_status)
    if pod_after_status != pod_before_status:
        LOGGER.error("k8s pods running status was changed after update management IP")
        _construct_result("k8s pods running status was changed after update management IP", ErrorCode.K8S_ERROR)
    LOGGER.info("k8s pods running status is normally")


def _active_network(interface, ip, vlan_id=None):
    """
    激活网络：当给主接口挂IP时，其下所有子接口（vlan）会down掉；当给子接口挂IP时，对应主接口的IP会被去掉
    :param interface: interface
    :param ip: ip
    :param vlan_id: vlan_id
    :return: None
    """
    bond_name = interface
    # 需先激活bond，若有vlan，再激活vlan
    if vlan_id is None and os.path.exists(f"/proc/net/bonding/{bond_name}"):
        _execute_piped_command(f"ifdown {bond_name}")
        time.sleep(1)
    _execute_piped_command(f"ifup {bond_name}")
    time.sleep(1)
    if vlan_id is not None:
        bond_name = bond_name + "." + str(vlan_id)
        _execute_piped_command(f"ifup {bond_name}")
        time.sleep(1)
    # 检查激活结果
    active_result = _execute_piped_command(f"ip a | grep {ip}" + " | awk '{print $NF}'")
    if active_result != bond_name:
        LOGGER.error("The network '%s' active failed", bond_name)
        _construct_result(f"The network '{bond_name}' active failed", ErrorCode.ACTIVE_FAILED)


def _is_same_subnet(ip, netmask, cur_interface):
    """
    校验是否存在子网冲突
    :param ip: ip
    :param netmask: netmask
    :param cur_interface: cur_interface
    :return: None
    """
    k8s_interface = "eth1"
    cidr = _subnet_mask_to_cidr(netmask)
    subnet = _calculate_network_address(ip, cidr)
    exist_ips = _execute_piped_command(f"ip addr show | grep {k8s_interface} | grep -oP 'inet \\K[\\d.]+/[0-9]+'")\
        .split('\n')
    same_subnets = []
    for exist_ip in exist_ips:
        exist_ip_config = exist_ip.split('/')
        if subnet == _calculate_network_address(exist_ip_config[0], exist_ip_config[1]):
            interface = _execute_piped_command(f"ip address | grep {exist_ip} " + " | awk '{print $NF}'")
            if cur_interface != interface.split(".", 1)[0]:
                same_subnets.append(interface + ":" + exist_ip)
    if same_subnets:
        LOGGER.error("IP config [%s/%s] is the same subnets with cluster IP config %s", ip, netmask, same_subnets)
        _construct_result(f"IP config [{ip}/{netmask}] is the same subnets with cluster IP config {same_subnets}",
                          ErrorCode.IP_CONFLICT_WITH_CLUSTER)


def _subnet_mask_to_cidr(subnet_mask):
    """
    子网掩码转换为CIDR
    :param subnet_mask: subnet_mask
    :return: CIDR
    """
    # 将子网掩码的每个部分转换为二进制，并连接起来形成一个完整的二进制字符串
    binary_subnet = ''.join(f'{int(x):08b}' for x in subnet_mask.split('.'))
    # 计算二进制字符串中连续的"1"的个数，即CIDR值
    cidr = binary_subnet.index('0') if '0' in binary_subnet else len(binary_subnet)
    return str(cidr)


def _calculate_network_address(ip, cidr):
    """
    根据IP地址和子网掩码计算网络地址
    :param ip: ip
    :param cidr: cidr
    :return:
    """
    network = ipaddress.ip_interface(f"{ip}/{cidr}")
    return str(network.network)


def _valid_vlan_id(vlan_id):
    """
    校验VLAN ID是否为1到4094之间的整数
    :param vlan_id: vlan_id
    :return: None
    """
    if vlan_id < 1 or vlan_id > 4094:
        LOGGER.error("'%s' is not a valid vlan id. It must be an integer between 1 and 4094", str(vlan_id))
        _construct_result(f"'{str(vlan_id)}' is not a valid vlan id. It must be an integer between 1 and 4094.",
                          ErrorCode.INVALID_VLAN_ID)


def config_biz_net(args):
    """
    配置业务面网络
    :param args: args
    :return: None
    """
    interface = args.interface
    ip_address = args.ip_address
    netmask = args.netmask
    gateway = args.gateway
    vlan_id = args.vlan_id
    LOGGER.info("start valid params")
    if vlan_id:
        _valid_vlan_id(vlan_id)
        LOGGER.info("validated vlan id")
    if gateway:
        _is_valid_ipv4([ip_address, netmask, gateway])
    else:
        _is_valid_ipv4([ip_address, netmask])
    LOGGER.info("validated ipV4 format")
    if not _is_valid_netmask(netmask):
        LOGGER.error("Invalid netmask format: %s", netmask)
        _construct_result(f"Invalid netmask format: {netmask}", ErrorCode.INVALID_NETMASK)
    LOGGER.info("validated netmask format")
    _is_ip_conflict(ip_address)
    LOGGER.info("validated ip address")
    _is_same_subnet(ip_address, netmask, interface)
    LOGGER.info("validated subnet format")
    bond_status = _execute_piped_command(f"nmcli connection | grep -i 'Bond {interface}'")
    if len(bond_status) == 0:
        LOGGER.error("Please bond the service port [%s] first", interface)
        _construct_result(f"Please bond the service port [{interface}] first.", ErrorCode.NOT_BONDED_ERROR)
    LOGGER.info("bond %s exist", interface)
    LOGGER.info("start config %s", interface)
    _update_network_config(interface=interface, ip=ip_address, netmask=netmask, gateway=gateway, vlan_id=vlan_id)
    _active_network(interface=interface, ip=ip_address, vlan_id=vlan_id)
    LOGGER.info("%s config success", interface)


def _construct_result(err_msg, err_code=1):
    """
    构造自定义result
    :param err_msg: err_msg
    :param err_code: err_code
    :return: None
    """
    custom_result.err_msg = err_msg
    custom_result.end_time = _get_datetime_by_timezone()
    custom_result.status = err_code
    raise MyExecuteException(error_info=err_msg, execute_result=custom_result)


def _execute_piped_command(command_str):
    """
    执行一个带有管道的shell命令字符串
    :param command_str: 包含管道的shell命令字符串，如 "ls | grep python"
    :return: 命令执行后的输出，如果有错误则抛出异常
    """
    # 按管道符分割命令
    commands = command_str.split('|')
    processes = []

    # 使用shlex.split来正确处理空格和引号
    args = shlex.split(commands[0])
    # 创建第一个命令的Popen实例
    prev_process = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    processes.append(prev_process)

    # 为剩余的命令创建Popen实例，并连接到前一个命令的输出
    for cmd in commands[1:]:
        args = shlex.split(cmd)
        curr_process = subprocess.Popen(args, stdin=prev_process.stdout, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        # 防止死锁
        prev_process.stdout.close()
        prev_process = curr_process
        processes.append(curr_process)

    output, error = ["", ""]
    try:
        # 获取最后一个命令的输出
        output, error = prev_process.communicate(timeout=5)
    except subprocess.TimeoutExpired as timeout_error:
        LOGGER.error("timeout when execute: %s", timeout_error)
        _construct_result(f"timeout when execute: {timeout_error}", ErrorCode.TIMEOUT_ERROR)
    finally:
        # 关闭所有进程
        for p in processes:
            p.kill()

    # 检查是否有错误发生
    if error:
        LOGGER.error("An error occurred while executing command '%s': %s", command_str, error.decode().strip())
        _construct_result(f"An error occurred while executing command '{command_str}': {error.decode().strip()}",
                          ErrorCode.EXECUTE_ERROR)

    # 返回解码后的输出
    return output.decode().strip()


def is_100ge_card_available(args):
    """
    校验是否存在100GE网卡且是否可用
    :param args: args
    :return: None
    """
    LOGGER.info("start check the 100ge network card")
    pci_result = _execute_piped_command("lspci | grep -c 0222")
    # 计算匹配"0222"的行数
    if int(pci_result) == 0:
        LOGGER.error("the 100GE network card is not exist")
        _construct_result("the 100GE network card is not exist", ErrorCode.NET_CARD_NOT_EXIST)
    # 检查CPU架构是否为aarch64
    cpu_info = _execute_piped_command("uname -r | grep 'aarch64' | awk -F'.' '{print $NF}'")
    if cpu_info != 'aarch64':
        LOGGER.error("the CPU does not support 100GE network cards")
        _construct_result("the CPU does not support 100GE network cards", ErrorCode.UNSUPPORTED_CPU)
    # 检查操作系统版本
    os_release = _execute_piped_command("uname -r | grep 'euleros' | awk -F'.' '{print $(NF-1)}' "
                                        "| awk -F'r' '{print $NF}'")
    if int(os_release) < 12:
        LOGGER.error("the os version does not support 100GE network cards")
        _construct_result("the os version does not support 100GE network cards", ErrorCode.UNSUPPORTED_OS)
    port_0 = _get_100ge_card_port(BUS_INFO_0)
    port_1 = _get_100ge_card_port(BUS_INFO_1)
    if port_0 == ErrorCode.NET_CARD_NOT_AT_CORRECT_SLOT or port_1 == ErrorCode.NET_CARD_NOT_AT_CORRECT_SLOT:
        LOGGER.error("100GE service port not exist. Please check whether the network card is inserted into "
                     "the correct slot")
        _construct_result("100GE service port not exist. Please check whether the network card is inserted into "
                          "the correct slot", ErrorCode.NET_CARD_NOT_AT_CORRECT_SLOT)
    if port_0 == ErrorCode.NET_CARD_DRIVER_NOT_CORRECT_INSTALLED \
            or port_1 == ErrorCode.NET_CARD_DRIVER_NOT_CORRECT_INSTALLED:
        LOGGER.error("100GE service port not exist. Please check whether the network card driver is installed "
                     "correctly")
        _construct_result("100GE service port not exist. Please check whether the network card driver is installed "
                          "correctly", ErrorCode.NET_CARD_DRIVER_NOT_CORRECT_INSTALLED)
    LOGGER.info("the 100ge network card is available")


def _get_100ge_card_port(bus_info):
    """
    查询是否存在100GE网口信息
    :param bus_info: bus_info
    :return: 10014(网卡位置错误) or 10015(网卡驱动未安装)
    """
    bus_path = _execute_piped_command(f"find '/sys/devices' -name {bus_info}")
    if bus_path == '':
        return ErrorCode.NET_CARD_NOT_AT_CORRECT_SLOT
    if not os.path.exists(bus_path + "/net"):
        return ErrorCode.NET_CARD_DRIVER_NOT_CORRECT_INSTALLED
    return 0


def _is_valid_function_name(func_name):
    """
    校验函数名是否在白名单内
    :param func_name: func_name
    :return: True or False
    """
    return func_name.strip().lower() in (name.lower() for name in ALLOWED_FUNCTIONS)


def _parse_params_and_execute():
    """
    解析参数并执行
    :return: None
    """
    parser = CustomArgumentParser(prog='PROG')
    subparsers = parser.add_subparsers(help='sub-command help')
    # 添加子命令 config_mgt_net
    parser_mgt = subparsers.add_parser('config_mgt_net', description="Configure management network interfaces.")
    parser_mgt.add_argument("--ip_address", required=True, help="The IP address to assign.")
    parser_mgt.add_argument("--netmask", required=True, help="The netmask for the interface.")
    parser_mgt.add_argument("--gateway", required=True, help="The gateway address.")
    parser_mgt.set_defaults(func=config_mgt_net)
    # 添加子命令 config_biz_net
    parser_biz = subparsers.add_parser('config_biz_net', description="Configure business network interfaces.")
    parser_biz.add_argument("--interface", required=True, choices=['eth2', 'eth3'],
                            help="The name of the network interface.")
    parser_biz.add_argument("--ip_address", required=True, help="The IP address to assign.")
    parser_biz.add_argument("--netmask", required=True, help="The netmask for the interface.")
    parser_biz.add_argument("--gateway", help="The gateway address.")
    parser_biz.add_argument("--vlan_id", type=int, help="The VLAN ID (optional).")
    parser_biz.set_defaults(func=config_biz_net)
    # 添加子命令 is_100GE_card_available
    parser_check = subparsers.add_parser('is_100ge_card_available', description="Validate 100GE network card.")
    parser_check.set_defaults(func=is_100ge_card_available)
    # 解析参数
    args = parser.parse_args()
    # 执行函数功能
    args.func(args)


def _execute(func_name):
    """
    执行函数
    :param func_name: func_name
    :return: custom_result
    """
    if not _is_valid_function_name(func_name):
        LOGGER.error("illegal function of %s", func_name)
        _construct_result(f"illegal function of {func_name}", ErrorCode.ILLEGAL_FUNC_NAME)
    LOGGER.info("start run %s", func_name)
    custom_result.method_name = func_name
    # 解析入参并执行
    _parse_params_and_execute()
    # 构造执行结果并以状态0退出
    custom_result.err_msg = None
    custom_result.end_time = _get_datetime_by_timezone()
    LOGGER.info("run %s success", func_name)
    return custom_result


def _main(args):
    """
    主函数入口
    :param args: args
    :return: custom_result
    """
    if len(args) < 2:
        LOGGER.error("wrong usage: %s", USAGE)
        custom_result.err_msg = f"wrong usage: {USAGE}"
        custom_result.end_time = _get_datetime_by_timezone()
        custom_result.status = ErrorCode.WRONG_USAGE
        return custom_result

    function_name = args[1]
    try:
        return _execute(function_name)
    except MyExecuteException as my_exception:
        return my_exception.execute_result
    except Exception as e:
        LOGGER.error("unknown error: %s", e)
        custom_result.err_msg = e.__cause__
        custom_result.end_time = _get_datetime_by_timezone()
        custom_result.status = ErrorCode.UNKNOWN_ERROR
        return custom_result


if __name__ == "__main__":
    LOGGER = _new_logger(LOG_FILE)
    custom_result.begin_time = _get_datetime_by_timezone()
    custom_result.data = ' '.join(sys.argv)

    sys.exit(json.dumps(_main(sys.argv).__dict__))
