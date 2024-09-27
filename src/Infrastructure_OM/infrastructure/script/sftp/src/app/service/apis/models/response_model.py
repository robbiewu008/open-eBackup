# coding: utf-8

from __future__ import absolute_import
from app.service.apis.models.base_model import Model


class DataResponse(Model):
    def __init__(self, success: bool = False, code: str = '', message: str = ''):
        # 类属性及其数据类型
        self.swagger_types = {
            'success': bool,
            'code': str,
            'message': str
        }

        # 类属性及其对应的REST字段
        self.attribute_map = {
            'success': 'success',
            'code': 'code',
            'message': 'message'
        }

        self._success = success
        self._code = code
        self._message = message

    @property
    def success(self) -> bool:
        return self._success

    @property
    def code(self) -> str:
        return self._code

    @property
    def message(self) -> str:
        return self._message
