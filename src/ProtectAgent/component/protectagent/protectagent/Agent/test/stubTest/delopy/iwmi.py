import winrm
from winrm import Response
from winrm.exceptions import WinRMOperationTimeoutError

from connection import IConnection
from log import logger


class IProtocol(winrm.Protocol):
    def get_command_output(self, shell_id, command_id, except_str):
        """
        Get the Output of the given shell and command
        @param string shell_id: The shell id on the remote machine.
         See #open_shell
        @param string command_id: The command id on the remote machine.
         See #run_command
        #@return [Hash] Returns a Hash with a key :exitcode and :data.
         Data is an Array of Hashes where the cooresponding key
        #   is either :stdout or :stderr.  The reason it is in an Array so so
         we can get the output in the order it ocurrs on
        #   the console.
        """
        stdout_buffer, stderr_buffer = [], []
        command_done = False
        while not command_done:
            try:
                stdout, stderr, return_code, command_done = \
                    self._raw_get_command_output(shell_id, command_id)
                stdout_buffer.append(stdout.decode())
                stderr_buffer.append(stderr.decode())
                if except_str and ''.join(stdout_buffer).rfind(except_str) != -1:
                    command_done = True
                    return_code = 0
            except WinRMOperationTimeoutError as e:
                # this is an expected error when waiting for a long-running process, just silently retry
                command_done = True
                return_code = 0
        return ''.join(stdout_buffer), ''.join(stderr_buffer), return_code


class ISession(winrm.Session):
    def __init__(self, target, auth, **kwargs):
        user, pwd = auth
        self.url = self._build_url(target, kwargs.get('transport', 'plaintext'))
        self.protocol = IProtocol(self.url, username=user, password=pwd, **kwargs)
        self.shell_id = self.protocol.open_shell()

    def run_cmd(self, command, except_str='', args=(), console_mode_stdin=True, skip_cmd_shell=False):
        # TODO optimize perf. Do not call open/close shell every time

        command_id = self.protocol.run_command(self.shell_id, command,
                                               args, console_mode_stdin=console_mode_stdin,
                                               skip_cmd_shell=skip_cmd_shell)
        rs = Response(self.protocol.get_command_output(self.shell_id, command_id, except_str))
        self.protocol.cleanup_command(self.shell_id, command_id)

        return rs

    def close(self):
        self.protocol.close_shell(self.shell_id)


class IWmi(IConnection):
    def __init__(self, host, user_name, pass_word, port=5985):
        self.con = ISession('http://' + host + ':%d/wsman' % port, auth=(user_name, pass_word))

    def exec(self, cmd, except_str='#', timeout=10, console_mode_stdin=True, skip_cmd_shell=False):
        cmd_ret = self.con.run_cmd(cmd, except_str=except_str,
                                   console_mode_stdin=console_mode_stdin,
                                   skip_cmd_shell=skip_cmd_shell)
        if cmd_ret.status_code == 0:
            cmd_out = cmd_ret.std_out
            logger.info("Process started successfully: %s" % cmd_out)
            return cmd_out
        else:
            logger.debug("Process exec failed, error code [%d], error msg[%s]" % (cmd_ret.status_code, cmd_ret.std_err))
            return cmd_ret.std_err

    def send_msg(self, msg, except_str='#'):
        resp = self.con.protocol.send_message(msg)
        return resp

    def close(self):
        self.con = None


if __name__ == '__main__':
    ip = '10.183.52.105'
    username = "administrator"
    password = "Huawei@123"
    wmi = IWmi(ip, username, password)
    ret = wmi.exec('cd D:\\agent_install_for_auto_test\\Agent\\bin')
    print(ret)
    cmd = 'cmd.exe /c %s' % r'D:\agent_install_for_auto_test\Agent\bin\agent_uninstall.bat'
    ret = wmi.exec(cmd, '>>')
    print(ret)
    ret = wmi.exec('y\n', '#', console_mode_stdin=False, skip_cmd_shell=False)
    print(ret)
    wmi.close()
