# This file is a part of the open-eBackup project.
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at
# http://mozilla.org/MPL/2.0/.
#
# Copyright (c) [2024] Huawei Technologies Co.,Ltd.
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
from http import HTTPStatus

from fastapi import FastAPI
from fastapi.encoders import jsonable_encoder
from fastapi.exceptions import RequestValidationError
from starlette.requests import Request
from starlette.responses import JSONResponse

from app.common import logger
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException, IllegalParamException

log = logger.get_logger(__name__)


def api_validation_handler1(req: Request, exc: EmeiStorBizException):
    return JSONResponse(
        status_code=HTTPStatus.INTERNAL_SERVER_ERROR,
        content=jsonable_encoder(
            exc.get_error_response()
        )
    )


def illegal_param_exception_handler(req: Request, exc: IllegalParamException):
    return JSONResponse(
        status_code=HTTPStatus.BAD_REQUEST,
        content=jsonable_encoder(
            exc.get_error_response()
        )
    )


def request_validation_handler(request: Request, exc: RequestValidationError):
    params = []
    for e in exc.errors():
        params.append(' -> '.join(str(e) for e in e['loc']))
    return JSONResponse(
        status_code=HTTPStatus.BAD_REQUEST,
        content=jsonable_encoder({"errorCode": CommonErrorCodes.ILLEGAL_PARAMS.get('code'),
                                  "errorMessage": CommonErrorCodes.ILLEGAL_PARAMS.get('message'),
                                  "parameters": params})
    )


def request_exception_handler(request: Request, e: Exception):
    log.exception(f"Internal Server Error: {e}")
    return JSONResponse(
        status_code=HTTPStatus.BAD_REQUEST,
        content=jsonable_encoder({
            "errorCode": CommonErrorCodes.SYSTEM_ERROR.get('code'),
            "errorMessage": CommonErrorCodes.SYSTEM_ERROR.get('message'),
            "parameters": [],
            "retryable": True
        })
    )


def setup_exception_handlers(app: FastAPI) -> None:
    app.add_exception_handler(EmeiStorBizException, api_validation_handler1)
    app.add_exception_handler(IllegalParamException, illegal_param_exception_handler)
    app.add_exception_handler(RequestValidationError, request_validation_handler)
    app.add_exception_handler(Exception, request_exception_handler)
