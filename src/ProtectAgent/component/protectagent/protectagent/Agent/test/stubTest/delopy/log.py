import logging
import sys


def init_log():
    log = logging.getLogger(__name__)

    formatter = logging.Formatter('%(asctime)s %(levelname)-3s: [%(filename)s] [%(lineno)s] [%(funcName)s] %(message)s ')
    file_handler = logging.FileHandler('install.log')
    file_handler.setFormatter(formatter)

    console_handler = logging.StreamHandler(sys.stdout)
    console_handler.formatter = formatter

    log.setLevel(logging.DEBUG)

    file_handler.setLevel(logging.DEBUG)
    console_handler.setLevel(logging.DEBUG)

    log.addHandler(file_handler)
    log.addHandler(console_handler)

    return log


logger = init_log()
