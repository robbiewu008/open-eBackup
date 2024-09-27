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
import requests

from log import logger


class Http(object):
    def get(self, remote_url, user, password):
        try:
            headers = {"Accept": 'application/json;version=1.0; charset=UTF-8',
                       "Content-Type": "application/json,charset=UTF-8",
                       'x-auth-user': user,
                       'x-auth-key': password
                       }
            response = requests.get(remote_url, headers=headers, verify=False)
            if response.status_code != 200:
                logger.debug("get data from remote host, error code is %d, error: %s" %
                             (response.status_code, response.content.decode()))
            # return value
            return response.status_code, response.content.decode()

        except Exception as error:
            import traceback
            logger.debug(traceback.print_exc())
            logger.debug("Cannot get response from remote, error: %s" % error)
            return None, None

    def post(self, remote_url, user, password, **kwargs):
        try:
            headers = {"Accept": 'application/json;version=1.0; charset=UTF-8',
                       "Content-Type": "application/json,charset=UTF-8",
                       'x-auth-user': user,
                       'x-auth-key': password
                       }
            response = requests.post(remote_url, data=kwargs, headers=headers, verify=False)
            if response.status_code != 200:
                logger.debug("Post data to remote host error, error code is %d, error: %s" %
                             (response.status, response.content.decode()))
            # return value
            return response.status_code, response.content.decode()

        except Exception as error:
            import traceback
            logger.debug(traceback.print_exc())
            logger.debug("Cannot get response from remote, error: %s" % error)
            return None, None
