from abc import ABC, abstractmethod
from typing import List, Callable
from dataclasses import dataclass

from dataprotect_deployment.client_manager import ClientManager
from config import Config, DataBackupConfig


@dataclass
class DeployProcess:
    forward_process: Callable
    backward_process: Callable


def gen_deploy_process(fwd_p: Callable, bck_p: Callable):
    return DeployProcess(forward_process=fwd_p, backward_process=bck_p)


class ProcessManagerBase(ABC):
    no_rollback = bool
    config = [Config, DataBackupConfig]

    def __init__(self, config, no_rollback: bool):
        self.process_list = None
        self.config = config
        self.no_rollback = no_rollback

    @abstractmethod
    def define_process(self, task_type) -> List[DeployProcess]:
        pass

    def exec(self, task_type):
        self.process_list = self.define_process(task_type)
        for i, process in enumerate(self.process_list):
            try:
                process.forward_process()
            except Exception as e:
                # log error
                if not self.no_rollback:
                    self.rollback(i)
                break

    def rollback(self, step: int):
        try:
            while step:
                self.process_list[step].backward_process()
                step -= 1

        except Exception:
            raise Exception("rollback failed")
