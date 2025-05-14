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

from fastapi import HTTPException
from sqlalchemy.orm.exc import NoResultFound

from app.resource.models.resource_models import log


def clean_except():
    def decorate(f):
        def applicator(*args, **kwargs):
            try:
                return f(*args, **kwargs)
            except NoResultFound:
                return None
            except Exception as e:
                log.exception("refresh error.")
                raise HTTPException(HTTPStatus.INTERNAL_SERVER_ERROR, e)

        return applicator

    return decorate
