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
package openbackup.access.framework.resource.model;

import lombok.Getter;
import lombok.Setter;
import openbackup.system.base.sdk.copy.model.Copy;

import java.util.List;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * 功能描述
 *
 */
@Getter
@Setter
public class CopyListParams {
    /**
     * 归档副本list
     */
    List<Copy> archiveCopies;

    /**
     * 备份副本list
     */
    List<Copy> backupCopies;

    /**
     * 复制副本list
     */
    List<Copy> replicationCopies;

    /**
     * 获取全部副本
     *
     * @return 全部副本
     */
    public List<Copy> getTotalCopies() {
        return Stream.of(backupCopies, archiveCopies, replicationCopies)
            .flatMap(List::stream)
            .collect(Collectors.toList());
    }

    /**
     * 获取备份复制副本
     *
     * @return 全部副本
     */
    public List<Copy> getBackAndReplicationCopies() {
        return Stream.of(backupCopies, replicationCopies).flatMap(List::stream).collect(Collectors.toList());
    }
}
