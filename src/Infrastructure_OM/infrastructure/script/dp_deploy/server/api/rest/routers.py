# !/usr/bin/env python
# _*_ coding:utf-8 _*_
# /*$$$!!Warning: Huawei key information asset. No spread without permission.$$$*/
# /*CODEMARK:rLr9evwLcHFK0ocH1vI5dv79JXrhoRrQ3pUSCsHdOM/puzvFwKILKNePe4bG/B98pz4V5doHi56R
# 0f3JfrYASX9dpcZzoFk8wkkpq5MAD4QH7vM9CsfnwL0y/c+rHjXKrQ8MVGc+XqcBtj8Yza8MaIQ4
# /ST8U6y6HofoEwJrgIV1PqdwKuSGnbCr26ut0vTK/X7j0meFoplM2zB9Q+wy44RIjvsz2jXnpNMi
# 0AoM4CsvOo9RvT/lahddqlVxb7zV#*/
# /*$$$!!Warning: Deleting or modifying the preceding information is prohibited.$$$*/

from server.api.rest import package_rest_handler, application_rest_handler, prepare_rest_handler, upgrade_rest_handler

EXTERNAL_PREFIX = '/v1/dp_deploy'


def register(api):
    api.include_router(prepare_rest_handler.external_router, prefix=EXTERNAL_PREFIX)
    api.include_router(package_rest_handler.external_router, prefix=EXTERNAL_PREFIX)
    api.include_router(application_rest_handler.external_router, prefix=EXTERNAL_PREFIX)
    api.include_router(upgrade_rest_handler.external_router, prefix=EXTERNAL_PREFIX)


def databackup_register(api):
    api.include_router(prepare_rest_handler.databackup_router, prefix=EXTERNAL_PREFIX)
    api.include_router(package_rest_handler.databackup_router, prefix=EXTERNAL_PREFIX)
    api.include_router(application_rest_handler.databackup_router, prefix=EXTERNAL_PREFIX)
    api.include_router(upgrade_rest_handler.external_router, prefix=EXTERNAL_PREFIX)
