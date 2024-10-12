#!/usr/bin/env python
# _*_ coding:utf-8 _*_

from enum import Enum


class BaseErrorCode(dict, Enum):
    pass


class GeneralErrorCodes(BaseErrorCode):
    ERR_OBJ_NOT_EXIST = {
        "code": "1677929217",
        "message": "The object does not exist."
    }

    ERR_INVALID_PARAM = {
        "code": "1677929218",
        "message": "The parameter is invalid."
    }

    ERR_OPERATION_FAILED = {
        "code": "1677929219",
        "message": "The operation fails."
    }

    ERR_SYSTEM_EXCEPTION = {
        "code": "1677929221",
        "message": "The system is abnormal."
    }

    ERR_SYSTEM_RESPONSE_TIMEOUT = {
        "code": "1677929226",
        "message": "The system response times out."
    }

    ERR_INTERNAL_ERROR = {
        "code": "1593987329",
        "message": "An internal error occurred."
    }

    ERR_NETWORK_EXCEPTION = {
        "code": "1593987350",
        "message": "The network is abnormal between internal components of the data protection engine."
    }

    ERR_TASK_SCHEDULE_TIMEOUT = {
        "code": "1593987462",
        "message": "Task schedule time out."
    }

    ERR_TASK_STATUS_NOT_MATCH = {
        "code": "1593987478",
        "message": "The network or process is abnormal, and the job status does not match."
    }

    ERR_CANNOT_ABORT = {
        "code": "1593987480",
        "message": "The current task phase cannot be aborted."
    }

    ERR_SUB_TASK_REACH_MAX = {
        "code": "1593987332",
        "message": ""
    }

    ERR_INVALID_SCN_RANGE = {
        "code": "1677874689",
        "message": "",
    }

    ERR_FILE_SYSTEM_NAME_EXISTS = {
        "code": "1677929237",
        "message": "The file system name already exists."
    }

