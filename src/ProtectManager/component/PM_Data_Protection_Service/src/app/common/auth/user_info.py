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
import jwt


def get_user_info_from_token(token):
    # Senthil: Currently we dont verify the token here.
    # Later we add support to get the public key of certificate and verify the token.
    token_parsed = jwt.decode(token, verify=False)
    is_admin = "false"
    is_auditor = "false"
    if 'user_roles' in token_parsed:
        roles = token_parsed['user_roles']
        if 'admin' in roles or 'sys_admin' in roles:
            is_admin = "true"
        elif 'auditor' in roles:
            is_auditor = "true"
    return {
        'user-name': token_parsed['user']['name'],
        'user-id': token_parsed['user']['id'],
        'es-admin-role': is_admin,
        'es-auditor-role': is_auditor,
        'es-valid-token': "true",
    }


def user_info_from_token(token: str, def_uid="1111", def_name="souschef"):
    try:
        return get_user_info_from_token(token)
    except:
        return {
            'user-name': def_name,
            'user-id': def_uid,
            'es-admin-role': "false",
            'es-auditor-role': "false",
            'es-valid-token': "false",
        }
