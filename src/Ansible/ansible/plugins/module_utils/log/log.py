import datetime
import logging
import os
# bash_path = os.path.split(os.path.realpath(__file__))[0]
bash_path = "/var/log/oceanprotect/"

if not os.path.exists(bash_path):
    os.mkdir(bash_path)
today = datetime.datetime.today()
log_file = os.path.join(bash_path, '%s%s%s%s%s.log' % (today.year, str(today.month).zfill(2),
                                                       str(today.day).zfill(2), str(today.hour).zfill(2),
                                                       str(today.minute).zfill(2)))
logger = logging.getLogger(bash_path)
logger.setLevel(logging.DEBUG)
handler1 = logging.StreamHandler()
handler1.setLevel(logging.DEBUG)

fh = logging.FileHandler(log_file)
fmt = '%(asctime)s - %(threadName)s - %(levelname)s - %(message)s'
formatter = logging.Formatter(fmt)
fh.setFormatter(formatter)
handler1.setFormatter(formatter)
logger.addHandler(fh)
logger.addHandler(handler1)
