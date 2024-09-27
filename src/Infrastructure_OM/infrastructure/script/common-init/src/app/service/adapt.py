import os
import subprocess
import shlex
import logging
import sys
from app.config import config


LOG_PATH = sys.argv[1]
LOG_FORMATTER = "[%(asctime)s][%(levelname)s][%(message)s][%(funcName)s][%(process)d][%(thread)d]" \
                "[%(name)s][%(threadName)s][%(filename)s][%(lineno)s]"


def handle_logger(logger, log_path):
    logger.setLevel(logging.INFO)
    stream_handler = logging.StreamHandler()
    _formatter = logging.Formatter(LOG_FORMATTER)
    stream_handler.setFormatter(_formatter)
    file_handler = logging.FileHandler(log_path, mode='a')
    file_handler.setFormatter(_formatter)
    logger.addHandler(stream_handler)
    logger.addHandler(file_handler)
    stream_handler.close()
    file_handler.close()


logger = logging.getLogger()
handle_logger(logger, LOG_PATH)


ports_dict = {
    "tcp_allow_ports": "",
    "udp_allow_ports": "",
    "tcp_allow_ports_ipv6": "",
    "udp_allow_ports_ipv6": ""
}


def execute_cmd(command):
    process = subprocess.Popen(shlex.split(command), stdout=subprocess.PIPE, stderr=subprocess.PIPE, encoding="utf-8")
    (output, err) = process.communicate()
    logger.info(f"output:{output}, err:{err}")
    process.wait()
    if process.poll() == 0:
        logger.info(f"the shell command of {command} is success")
        return True
    else:
        logger.info(f"the shell command of {command} is failed")
        return False


def get_config_map_data():
    """
    获取指定config_map的数据
    :return:
    """
    for port in ports_dict.keys():
        with open(os.path.join(config.CONFIG_MAP_DATA_PATH, port), "r") as file:
            data = file.read()
            ports_dict[port] = data
    return ports_dict


def main():
    logger.info("start to set firewall")
    ports_dict = get_config_map_data()
    execute_cmd(f'''sed -i 's/tcp_allow_ports=.*/tcp_allow_ports="{ports_dict["tcp_allow_ports"]}"/' {config.NET_CNET_FIREWALL_FILE}''')
    execute_cmd(f'''sed -i 's/udp_allow_ports=.*/udp_allow_ports="{ports_dict["udp_allow_ports"]}"/' {config.NET_CNET_FIREWALL_FILE}''')
    execute_cmd(f'''sed -i 's/tcp_allow_ports_ipv6=.*/tcp_allow_ports_ipv6="{ports_dict["tcp_allow_ports_ipv6"]}"/' {config.NET_CNET_FIREWALL_FILE}''')
    execute_cmd(f'''sed -i 's/udp_allow_ports_ipv6=.*/udp_allow_ports_ipv6="{ports_dict["udp_allow_ports_ipv6"]}"/' {config.NET_CNET_FIREWALL_FILE}''')
    execute_cmd(f"bash {config.NET_CNET_CONFIG_FIREWALL_FILE} {config.NET_CNET_FIREWALL_FILE}")
    logger.info("set firewall successfully")


if __name__ == "__main__":
    main()
