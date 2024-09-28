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
package openbackup.tdsql.resources.access.util;

import openbackup.tdsql.resources.access.dto.instance.DataNode;

import org.apache.commons.collections4.Equator;

import java.util.Objects;

/**
 * 功能描述
 *
 */
public class DataNodeEquator implements Equator<DataNode> {
    @Override
    public boolean equate(DataNode dataNode1, DataNode dataNode2) {
        if (dataNode1 == null && dataNode2 == null) {
            return true;
        }

        if (dataNode1 == null || dataNode2 == null) {
            return false;
        }

        if (dataNode1 == dataNode2) {
            return true;
        }

        return Objects.equals(dataNode1.getIp(), dataNode2.getIp())
            && Objects.equals(dataNode1.getPort(), dataNode2.getPort());
    }

    @Override
    public int hash(DataNode dataNode) {
        return Objects.hash(dataNode.getIp(), dataNode.getPort(), dataNode.getIsMaster());
    }
}
