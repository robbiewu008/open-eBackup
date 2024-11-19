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
package openbackup.mysql.resources.access.provider;

import openbackup.access.framework.resource.provider.DefaultResourceProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.mysql.resources.access.enums.MysqlResourceSubTypeEnum;

import org.springframework.stereotype.Component;

/**
 * MysqlDatabaseResourceProvider
 *
 */
@Component
public class MysqlDatabaseResourceProvider extends DefaultResourceProvider {
    @Override
    public boolean supplyDependency(ProtectedResource resource) {
        return true;
    }

    @Override
    public boolean applicable(ProtectedResource object) {
        return MysqlResourceSubTypeEnum.MYSQL_DATABASE.getType().equals(object.getSubType());
    }
}