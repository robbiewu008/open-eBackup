/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.system.base.sdk.dmelog;

import openbackup.system.base.sdk.dmelog.model.DmeBatchTenantTrace;
import openbackup.system.base.sdk.dmelog.model.DmeTenantTrace;

/**
 * DME操作日志记录
 *
 */
public interface DmeTenantTranceService {
    /**
     * 根据OP操作日志记录DME操作利润
     *
     * @param dmeBatchTenantTrace 操作日志
     */
    void createDmeTenantTrance(DmeBatchTenantTrace dmeBatchTenantTrace);

    /**
     * AlarmObject 增加DME日志对象
     *
     * @param dmeTenantTrace DmeTenantTrace
     * @param dmeTokenStr DME token
     * @return BatchTenantTrace
     */
    DmeBatchTenantTrace getDmeTenantTrance(DmeTenantTrace dmeTenantTrace, String dmeTokenStr);
}
