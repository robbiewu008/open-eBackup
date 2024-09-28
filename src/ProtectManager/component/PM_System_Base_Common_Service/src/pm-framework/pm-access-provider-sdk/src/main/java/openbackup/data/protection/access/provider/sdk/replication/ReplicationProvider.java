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
package openbackup.data.protection.access.provider.sdk.replication;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.CopyReplicationImport;

/**
 * Replication Provider
 *
 */
public interface ReplicationProvider extends DataProtectionProvider<String> {
    /**
     * replicate backup object
     *
     * @param context context
     */
    void replicate(IReplicateContext context);

    /**
     * build copy info
     *
     * @param copy copy
     * @param importParam import param
     */
    void buildCopyProperties(CopyInfoBo copy, CopyReplicationImport importParam);

    /**
     * check copy whether exist
     *
     * @param chainId chainId
     * @param timestamp timestamp
     * @return 待入库副本是否已存在
     */
    boolean checkCopyWhetherExist(String chainId, long timestamp);
}
