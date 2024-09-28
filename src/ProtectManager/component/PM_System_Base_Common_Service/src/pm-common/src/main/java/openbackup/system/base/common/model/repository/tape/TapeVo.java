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
package openbackup.system.base.common.model.repository.tape;

import lombok.Getter;
import lombok.Setter;

/**
 * 单个磁带信息
 *
 **/
@Getter
@Setter
public class TapeVo {
    /**
     * 磁带UUID
     */
    private String tapeUUID;

    /**
     * 磁带Label
     */
    private String tapeLabel;

    /**
     * 介质集id
     */
    private String mediaSetId;

    /**
     * 介质集Name
     */
    private String mediaSetName;

    /**
     * 所属带库的SN
     */
    private String tapeLibrarySn;

    /**
     * 磁带写状态
     * unknown：0；identifing：1；empty：2；written：3；full:4；error：5
     */
    private TapeWriteStatus writeStatus;

    /**
     * 已用空间 单位kb
     */
    private Integer usedCapacity;

    /**
     * 全部空间 单位kb
     */
    private Integer totalCapacity;

    /**
     * 磁带所属位置，例如drive 1；Storage Element 10
     */
    private String location;

    /**
     * worm：1； RW：0
     */
    private TapeWorm worm;

    /**
     * 最后写入时间
     */
    private String lastWriteTime;

    /**
     * 磁带状态
     */
    private TapeStatus status;

    /**
     * 加锁对象的 taskID
     */
    private String lockKey;

    /**
     * 磁带上一次写入位置
     */
    private Integer commitedWritePos;

    /**
     * 磁带过期回收标识
     */
    private Boolean isDepended;

    /**
     * 磁带补充信息
     */
    private String extend;
}
