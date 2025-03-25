from abc import ABC, abstractmethod
from typing import List, Callable
from dataclasses import dataclass
import logging as log
from pydantic import BaseModel

from client_exception import SmartkitException, E1000_CN_ENG_INSTALL_EXPAND_DICT
from dataprotect_deployment.client_manager import ClientManager
from config import Config, DataBackupConfig, SmartKitResponse


@dataclass
class DeployProcess:
    forward_process: Callable
    backward_process: Callable


def gen_deploy_process(fwd_p: Callable, bck_p: Callable):
    return DeployProcess(forward_process=fwd_p, backward_process=bck_p)


class ProcessManagerBase(ABC):
    no_rollback = bool

    def __init__(self, config: DataBackupConfig, no_rollback: bool):
        self.process_list = None
        self.config = config
        self.no_rollback = no_rollback

    @abstractmethod
    def define_process(self, task_type: str) -> List[DeployProcess]:
        pass

    def exec(self, task_type: str):
        self.process_list = self.define_process(task_type)
        total_step_nums = len(self.process_list)
        err_msg_cn = ""
        err_msg_en = ""
        suggestion = ""
        for i, process in enumerate(self.process_list):
            try:
                process.forward_process()
            except SmartkitException as e:
                err_msg_cn = f"{e.message_cn}"
                err_msg_en = f"{e.message_en}"
                suggestion = "Please collect logs and contact engineer support"
            except Exception as e:
                err_msg_cn = f"{E1000_CN_ENG_INSTALL_EXPAND_DICT[task_type][i][1]}阶段出现异常"
                err_msg_en = f"There are some errors happening in {E1000_CN_ENG_INSTALL_EXPAND_DICT[task_type][i][0]}"
                suggestion = "Please collect logs and contact engineer support"
            finally:
                r = SmartKitResponse(
                    step_id=str(i),
                    progress=f"{((i+ 1 )/ total_step_nums):.0%}",
                    process=process.forward_process.__name__,
                    errorMgCn=f"{err_msg_cn}",
                    errorMgEn=f"{err_msg_en}",
                    suggestion=suggestion,
                )
                if err_msg_cn == "":
                    log.info(f"{r.json()}")
                else:
                    log.error(f"{r.json()}")
                    if not self.no_rollback:
                        self.rollback(i)
                    break

    def upgrade_exec(self, task_type: str):
        self.process_list = self.define_process(task_type)
        for process in self.process_list:
            process.forward_process()

    def rollback(self, step: int):
        try:
            while step:
                self.process_list[step].backward_process()
                step -= 1

        except Exception:
            raise Exception("rollback failed")
