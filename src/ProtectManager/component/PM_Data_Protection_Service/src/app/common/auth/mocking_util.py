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
from app.common import logger

log = logger.get_logger(__name__)


class MockResponse:
    streaming_file = None  # Used by license export to file

    def __init__(self, json_data, status_code):
        self.json_data = json_data
        self.status_code = status_code
        # Used by license export to file
        self.headers = {'Content-Type': 'application/x-download;charset=UTF-8'}
        self.encoding = "utf-8"

    def raise_for_status(self):
        pass

    @property
    def content(self):
        return self.json_data

    def iter_content(self, chunk_size=128):
        # Used by license export to file
        data = MockResponse.streaming_file[:]
        while data:
            chunk = data[:chunk_size]
            data = data[len(chunk):]
            log.info("Yielding chunk")
            yield bytes(chunk, encoding='UTF-8')

    def json(self):
        return self.json_data

    @property
    def text(self):
        return self.json_data


def set_streaming_file(file_data):
    MockResponse.streaming_file = file_data


def mock_response(cooked_responses, arg0, source_mock):
    if '?' in arg0:
        arg0 = arg0.split('?')[0]
    arg0 = arg0.replace('http://', '').replace('https://', '')
    first_slash = arg0.find('/')
    arg0 = arg0[first_slash:]
    log.debug(f'arg0={arg0}')
    if arg0 in cooked_responses:
        log.debug(f'{source_mock}: Found exactly: {arg0}')
        resp_data, status = cooked_responses[arg0]
        return MockResponse(resp_data, status)
    for key in cooked_responses.keys():
        if arg0.startswith(key):
            log.debug(f'{source_mock}: Found by prefix: {key}')
            resp_data, status = cooked_responses[key]
            return MockResponse(resp_data, status)

    log.error(f'{source_mock}: Could not find for {arg0}')
    return MockResponse(None, 404)
