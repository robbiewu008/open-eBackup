from app.common.logger import log
from app.service.apis.models.response_model import DataResponse


def send_backup_response(success: bool = False, code: str = '', message: str = ''):
    rsp = DataResponse(success, code, message).to_dict()
    log.debug(f'Response success:{rsp.get("success")}, Response code:{rsp.get("code")}')
    return rsp
