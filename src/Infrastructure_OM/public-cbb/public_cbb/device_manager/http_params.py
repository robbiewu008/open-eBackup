#!/usr/bin/env python
# _*_ coding:utf-8 _*_

class SessionInfo:
    def __init__(
        self,
        base_url: str = '',
        device_id: str = '',
        token: str = '',
        cookie: str = '',
    ):
        self.base_url = base_url
        self.device_id = device_id
        self.token = token
        self.cookie = cookie
