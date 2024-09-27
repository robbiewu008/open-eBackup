import multiprocessing as mp
import signal
import sys
import time
import threading
from contextlib import contextmanager
from multiprocessing.context import Process
from abc import ABC, abstractmethod
import uvicorn
from fastapi import FastAPI, Depends

from server.api.rest.routers import register
from server.common import consts


class Context:
    def __init__(self):
        self.sub_processes_dict = {}
        signal.signal(signal.SIGTERM, self.sigterm_handler)

    def register_subprocess(self, p_name: str, process: Process, exist_ok=False):
        if p_name in self.sub_processes_dict.keys():
            if not exist_ok:
                raise Exception
            if exist_ok and self.sub_processes_dict[p_name].is_alive():
                raise Exception

        self.sub_processes_dict[p_name] = process

    def unregister_subprocess(self, p_name: str):
        if p_name not in self.sub_processes_dict.keys():
            raise Exception
        if self.sub_processes_dict[p_name].is_alive():
            raise Exception
        self.sub_processes_dict.pop(p_name)

    def sigterm_handler(self, signum, frame):
        for subprocess in self.sub_processes_dict.values():
            if subprocess:
                subprocess.terminate()
                subprocess.join()
        sys.exit(0)


class DpServerBase(ABC):
    """
    DPserver functions as a server, providing API interfaces to external clients.
    It comes pre-installed on the system and is managed using systemctl.
    """

    def __init__(self, address):
        """
        Create a new dpserver service.

        :param str address:
            the ip address of this dpserver service listening. (always use node's management ip)
        """
        self.address = address

    @abstractmethod
    def prepare_env(self, context: Context):
        """
        Some settings for the env, e.g. cgroup settings and firewall settings.
        """
        pass

    @abstractmethod
    def credentials(self):
        """
        Provide verify method. e.g. verify token interface or verify by passwd.
        .. note::
            Dpserver doesn't create new authentication methods, but rather reuses existing ones.
            e.g. E6000 verify token method using Pacific's token verify api interface.
        """
        pass

    @abstractmethod
    def generate_route(self, api):
        pass

    def run_api_server(self, passwd, context: Context):
        """
        Run Dpserver api service which use fastapi framework and combined with uvicorn.
        """
        api = FastAPI(dependencies=[Depends(self.credentials)])
        self.generate_route(api)

        def uvicorn_service():
            uvicorn.run(
                api,
                host=self.address,
                port=consts.DPSERVER_PORT,
                ssl_keyfile=consts.SSL_KEYFILE_PATH_DPSERVER,
                ssl_certfile=consts.SSL_CERTFILE_PATH_DPSERVER,
                ssl_ca_certs=consts.SSL_CA_CERTFLE_PATH_DPSERVER,
                ssl_keyfile_password=passwd,
                ssl_ciphers=consts.SSL_CIPHER
            )

        p = mp.Process(target=uvicorn_service)
        p.start()
        context.register_subprocess("dpserver_subprocess", p, exist_ok=True)


class DpserverCertManager(ABC):
    def __init__(self):
        self.passwd = None

    @abstractmethod
    def get_private_cert_passwd(self):
        """
        Not only decrypt the ssl keyfile,
        but the whole process to get private cert passwd
        """
        pass

    @abstractmethod
    def prepare_cert(self):
        """
        Mainly used to make ssl_certfile and ssl_ca_cert.
        """
        pass

    @abstractmethod
    def get_cert_sha256(self, cert_dir: str = None):
        pass

    @abstractmethod
    def renew_cert(self):
        pass


class Bridge:
    def __init__(self, cert_manager: DpserverCertManager, dpserver: DpServerBase):
        self.cert_manage = cert_manager
        self.dpserver = dpserver
        self.bridge_run_handler = None
        self.sub_processes_dict = {}
        self.cert_dir_sha256 = None

    def run(self, context: Context):
        self.dpserver.prepare_env(context)
        self.cert_manage.get_private_cert_passwd()
        self.cert_manage.prepare_cert()
        self.cert_dir_sha256 = self.cert_manage.get_cert_sha256()

        self.dpserver.run_api_server(self.cert_manage.passwd, context)
        while True:
            now_cert_dir_sha256 = self.cert_manage.get_cert_sha256()
            if self.cert_dir_sha256 != now_cert_dir_sha256:
                dpserver_handler = context.sub_processes_dict["dpserver_subprocess"]
                dpserver_handler.terminate()
                dpserver_handler.join()
                passwd = self.cert_manage.get_private_cert_passwd()
                self.dpserver.run_api_server(passwd, context)
                self.cert_dir_sha256 = now_cert_dir_sha256
            time.sleep(10)
