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
import base64
import http
import json
import urllib.parse
from ipaddress import ip_address, IPv6Address

import urllib3
from requests import Session

from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.resource_error_codes import ResourceErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.logger import get_logger
from app.resource.common.constants import DmaProxy

log = get_logger(__name__)


class VMTagReader:
    def __init__(self, vcenter_ip: str, port, username: str, password: str):
        urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)
        self.host = 'https://' + self.format_ip(vcenter_ip) + ":" + str(port)
        self.session = Session()
        self.session.verify = False
        self.session.auth = (username.encode(), password.encode())
        self.username = username
        self.password = password
        self.session_id = ''

        # "vm_id" -> list[tag_name]
        self._vm_tag_dict = {}
        self._vm_tag_dict_inited = False

    @staticmethod
    def format_ip(vcenter_ip):
        try:
            connect_ip = ip_address(vcenter_ip)
        except ValueError as e:
            raise EmeiStorBizException(CommonErrorCodes.ERR_PARAM,
                                       message="The vcenter ip isn't either a v4 or a v6 address.") from e
        if isinstance(connect_ip, IPv6Address):
            connect_ip = '[' + str(connect_ip) + ']'
        return str(connect_ip)

    def get_vm_tags(self, vm_id: str) -> str:
        # lazy init, do not touch
        self._init_vm_tag_dict()
        tag_names = []
        if self._vm_tag_dict.__contains__(vm_id):
            tag_names = sorted(self._vm_tag_dict[vm_id])
        return ','.join(tag_names)

    def get_tags(self) -> list:
        res = self.session.post(f'{self.host}/rest/com/vmware/cis/tagging/batch?~action=get-all-tags', timeout=20)
        return res.json()['value']

    def list_all_attached_objects_on_tags(self) -> list:
        res = self.session.post(
            f'{self.host}/rest/com/vmware/cis/tagging/batch?~action=list-all-attached-objects-on-tags', timeout=20)
        return res.json()['value']

    def get_tag_info(self, tag_id):
        log.debug(f"[VMWare Scan] Get tag info, tagId: {tag_id}")
        res = self.session.get(f'{self.host}/rest/com/vmware/cis/tagging/tag/id:{tag_id}', timeout=20)
        return res.json()['value']

    def list_vm_attached_tag_ids(self, vm_id) -> list[str]:
        vm_param = {"object_id": {"id": vm_id, "type": "VirtualMachine"}}
        res = self.session.post(
            f'{self.host}/rest/com/vmware/cis/tagging/tag-association?~action=list-attached-tags',
            data=json.dumps(vm_param), timeout=20)
        return res.json()['value']

    @staticmethod
    def get_tag_dict(tags: list) -> dict:
        return {tag["id"]: tag["name"] for tag in tags}

    def _init_vm_tag_dict(self):
        if self._vm_tag_dict_inited:
            return

        tag_dict = self.get_tag_dict(self.get_tags())

        for entry in self.list_all_attached_objects_on_tags():
            raw_tag_name = tag_dict.get(entry['tag_id'])
            if raw_tag_name is None:
                raw_tag_name = self.get_tag_info(entry['tag_id']).get('name')
            tag_name = urllib.parse.quote(raw_tag_name)
            for obj in entry['object_ids']:
                if obj['type'] != "VirtualMachine":
                    continue
                if self._vm_tag_dict.__contains__(obj['id']):
                    self._vm_tag_dict[obj['id']].add(tag_name)
                else:
                    self._vm_tag_dict[obj['id']] = {tag_name}

        self._vm_tag_dict_inited = True

    def connect(self):
        authorization = base64.b64encode((self.username + ":" + self.password).encode('utf-8'))
        self.session.headers.update({'Authorization': 'Basic ' + str(authorization)})

        try:
            res = self.session.post(f'{self.host}/rest/com/vmware/cis/session', timeout=20)
        except BaseException as e:
            log.debug(f"Got exception: {e} while trying to connect to host directly, retrying with proxy.")
            try:
                self.session.proxies.update({
                    'https': "http://" + DmaProxy.host + ":" + str(DmaProxy.port),
                    'http': "http://" + DmaProxy.host + ":" + str(DmaProxy.port)
                })
                res = self.session.post(f'{self.host}/rest/com/vmware/cis/session', timeout=20)
            except BaseException as be:
                log.debug(f"Got exception: {be} while trying to connect to host with proxy.")
                raise EmeiStorBizException(ResourceErrorCodes.NETWORK_CONNECTION_TIMEDOUT,
                                           message="Network connection timed out.")

        if res is not None and res.status_code == http.HTTPStatus.OK:
            self.session_id = res.json()['value']
        else:
            log.error(f"Failed to get session from vcenter server. ")
        self.session.headers.update({'vmware-api-session-id': self.session_id})
        self.session.headers.update({'content-type': 'application/json'})

    def disconnect(self):
        if self.session_id:
            res = self.session.delete(f'{self.host}/rest/com/vmware/cis/session', timeout=20)
            if res.status_code != http.HTTPStatus.OK:
                log.error(f"Failed delete session from vcenter server.")

    def __enter__(self):
        self.connect()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.disconnect()
        self.session.close()
