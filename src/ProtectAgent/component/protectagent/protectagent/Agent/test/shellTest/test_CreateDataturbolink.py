"""
运行方式：
export PYTHONPATH=${HOME}/Agent/bin/shell/
cd ${HOME}/Agent/test/shellTest/ && python3 -m unittest test_CreateDataturbolink
"""
import unittest
from unittest.mock import Mock, patch

import CreateDataturbolink as test_obj
from CreateDataturbolink import Result, LinkType, InputParam


class PopenRetMock:

    def __init__(self, ret_code=0, std_out="", std_err=""):
        self.returncode = ret_code
        self._std_out = std_out
        self._std_err = std_err

    def communicate(self):
        return self._std_out, self._std_err


class SpawnRetMock:

    def __init__(self, expect_ret=0):
        self.expect_ret = expect_ret

    def expect(self):
        return self.expect_ret

    def sendline(self):
        pass


IP_LINK_STATUS_NORMAL = [
    "   ", "Storage Name:	4e8ee854-1227-4011-a681-f4c675b3d68b  ", "User        :	data_turbo_account  ",
    "Ips         :	192.168.70.25,192.168.70.26,   ", "IpPair      : ",
    "ID	Local Address		Remote Address		Status   ",
    "--------------------------------------------------------------- ",
    "1	192.168.97.240		192.168.70.25		Normal   ",
    "2	192.168.97.240		192.168.70.26		Normal  ", "   "
]

FC_LINK_STATUS_NORMAL = [
    "   ", "Storage Name:	a5447695-86dd-4159-84cc-36a4fdf9cba4  ", "User        :	data_turbo_account  ",
    "ID     Initiator WWN		   Target WWN		Status   ",
    "--------------------------------------------------------------- ",
    "1      10000090faf01674		200074a063f89a6e		Normal   ",
    "2      10000090faf01675		201174a063f89a6e		Normal  ", "   "
]


def get_ip_link_status(err_type=0):
    if err_type == 1:
        IP_LINK_STATUS_NORMAL[3] = "Ips         :	,,  "
    if err_type == 2:
        IP_LINK_STATUS_NORMAL[7].replace("Normal", "Fault")
        IP_LINK_STATUS_NORMAL[8].replace("Normal", "Fault")
    if err_type == 3:
        return "\n".join([
            "   ", "Cause:The storage object does not exist and the command operation cannot be executed.", "",
            "Suggestion:Run a command to view existing storage objects and try again."
        ])
    if err_type == 4:
        tmp_arr = ["shell init: error 1", "shell init: error 2"] + IP_LINK_STATUS_NORMAL
        return '\n'.join(tmp_arr)
    return '\n'.join(IP_LINK_STATUS_NORMAL)


def get_fc_link_status(err_type=0):
    if err_type == 1:
        FC_LINK_STATUS_NORMAL[5].replace("Normal", "Fault")
        FC_LINK_STATUS_NORMAL[6].replace("Normal", "Fault")
    return '\n'.join(FC_LINK_STATUS_NORMAL)


class TestCreateDataturbolink(unittest.TestCase):

    @patch("CreateDataturbolink.subprocess.Popen", Mock(return_value=PopenRetMock(std_out="abc.txt")))
    def test_exec_shell_cmd_success(self):
        """
        用例名称：执行shell命令
        前置条件：shell命令执行成功并且有输出信息
        check点：返回值为SUCCESS
        """
        cmd = "ls"
        ret, out, _ = test_obj.exec_shell_cmd(cmd)
        self.assertEqual(ret, Result.SUCCESS)
        self.assertEqual(out, "abc.txt")

    @patch("CreateDataturbolink.subprocess.Popen", Mock(side_effect=Exception("Permission denied")))
    def test_should_failed_if_cmd_raise_exception_when_exec_shell_cmd(self):
        """
        用例名称：执行shell命令失败
        前置条件：shell命令执行成功并且有输出信息
        check点：返回值不为SUCCESS
        """
        cmd = "ls"
        ret, _, err = test_obj.exec_shell_cmd(cmd)
        self.assertNotEqual(ret, Result.SUCCESS)

    @patch("CreateDataturbolink.exec_shell_cmd", Mock(return_value=(0, get_ip_link_status(4), "")))
    def test_get_ip_dataturbo_link_status_succes_with_jam_info(self):
        """
        用例名称：当执行命令的返回结果有干扰信息时，获取链接状态成功，并且链接类型是IP
        前置条件：shell命令执行成功，并且获取到了有效的链接信息，但是里面有一些多余的字段
        check点：返回的链接信息
        """
        ret = test_obj.get_dataturbo_link_status("4e8ee854-1227-4011-a681-f4c675b3d68b")
        self.assertTrue(ret.object_create_flag)
        self.assertFalse(ret.all_link_fault_flag)
        self.assertEqual(ret.link_type, LinkType.IP)
        self.assertEqual(ret.link_ip_list, ["192.168.70.25", "192.168.70.26"])

    @patch("CreateDataturbolink.exec_shell_cmd", Mock(return_value=(0, get_ip_link_status(), "")))
    def test_get_ip_dataturbo_link_status_succes(self):
        """
        用例名称：获取链接状态成功，并且链接类型是IP
        前置条件：shell命令执行成功，并且获取到了有效的链接信息
        check点：返回的链接信息
        """
        ret = test_obj.get_dataturbo_link_status("4e8ee854-1227-4011-a681-f4c675b3d68b")
        self.assertTrue(ret.object_create_flag)
        self.assertFalse(ret.all_link_fault_flag)
        self.assertEqual(ret.link_type, LinkType.IP)
        self.assertEqual(ret.link_ip_list, ["192.168.70.25", "192.168.70.26"])

    @patch("CreateDataturbolink.exec_shell_cmd", Mock(return_value=(0, get_fc_link_status(), "")))
    def test_get_FC_dataturbo_link_status_succes(self):
        """
        用例名称：获取链接状态成功，并且链接类型是FC
        前置条件：shell命令执行成功，并且获取到了有效的链接信息
        check点：返回的链接信息
        """
        ret = test_obj.get_dataturbo_link_status("a5447695-86dd-4159-84cc-36a4fdf9cba4")
        self.assertTrue(ret.object_create_flag)
        self.assertFalse(ret.all_link_fault_flag)
        self.assertEqual(ret.link_type, LinkType.FC)
        self.assertEqual(ret.link_ip_list, [])

    @patch("CreateDataturbolink.exec_shell_cmd", Mock(return_value=(1, "", "Command not found")))
    def test_should_ret_not_create_if_exec_shell_cmd_failed_when_get_dataturbo_link_status(self):
        """
        用例名称：执行shell命令失败时，返回的链接状态应该是未创建
        前置条件：shell命令执行失败
        check点：返回的链接状态为未创建
        """
        ret = test_obj.get_dataturbo_link_status("4e8ee854-1227-4011-a681-f4c675b3d68b")
        self.assertFalse(ret.object_create_flag)

    @patch("CreateDataturbolink.exec_shell_cmd", Mock(return_value=(0, get_ip_link_status(3), "Command not found")))
    def test_should_ret_not_create_if_cmd_result_too_less_when_get_dataturbo_link_status(self):
        """
        用例名称：shell执行命令的返回结果小于3行时，返回的链接状态应该是未创建
        前置条件：1、shell命令执行成功，但是返回的结果信息不足3行
        check点：返回的链接状态为未创建
        """
        ret = test_obj.get_dataturbo_link_status("4e8ee854-1227-4011-a681-f4c675b3d68b")
        self.assertFalse(ret.object_create_flag)

    @patch("CreateDataturbolink.exec_shell_cmd", Mock(return_value=(0, get_ip_link_status(1), "Command not found")))
    def test_should_ret_all_link_fault_if_get_ip_list_err_when_get_dataturbo_link_status(self):
        """
        用例名称：解析IP地址的list失败时，返回的已建链，但是所有链接都已损坏
        前置条件：1、shell命令执行成功，2、解析IP地址失败
        check点：返回链接创建状态和所有链接状态
        """
        ret = test_obj.get_dataturbo_link_status("4e8ee854-1227-4011-a681-f4c675b3d68b")
        self.assertTrue(ret.object_create_flag)
        self.assertTrue(ret.all_link_fault_flag)

    @patch("CreateDataturbolink.pexpect.spawn", Mock(return_value=(SpawnRetMock(0))))
    def test_create_ip_dataturbo_object_success(self):
        """
        用例名称：创建IP类型的链接对象
        前置条件：1、shell命令执行成功，2、账号密码输入成功
        check点：返回SUCCESS
        """
        input_param = InputParam(
            "storageName=4e8ee854-1227-4011-a681-f4c675b3d68b\nipList=192.168.97.140, 192.168.97.142\n")
        ret = test_obj.create_dataturbo_object(input_param)
        self.assertTrue(ret)

    @patch("CreateDataturbolink.pexpect.spawn", Mock(return_value=(SpawnRetMock(0))))
    def test_create_fc_dataturbo_object_success(self):
        """
        用例名称：创建FC类型的链接
        前置条件：1、shell命令执行成功，2、账号密码输入成功
        check点：返回SUCCESS
        """
        input_param = InputParam(
            "storageName=4e8ee854-1227-4011-a681-f4c675b3d68b\nipList=192.168.97.140, 192.168.97.142\nlinkType=FC\n")
        ret = test_obj.create_dataturbo_object(input_param)
        self.assertTrue(ret)

    @patch("CreateDataturbolink.pexpect.spawn", Mock(return_value=(SpawnRetMock(0))))
    def test_delete_dataturbo_object_success(self):
        """
        用例名称：删除Dataturbo链路成功
        前置条件：1、shell命令执行成功
        check点：返回SUCCESS
        """
        ret = test_obj.delete_dataturbo_object("4e8ee854-1227-4011-a681-f4c675b3d68b")
        self.assertTrue(ret)
