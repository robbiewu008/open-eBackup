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
package openbackup.data.access.framework.core.entity;

import com.baomidou.mybatisplus.annotation.IdType;
import com.baomidou.mybatisplus.annotation.TableField;
import com.baomidou.mybatisplus.annotation.TableId;
import com.baomidou.mybatisplus.annotation.TableName;

import lombok.Getter;
import lombok.Setter;

/**
 * COPIES_ANTI_RANSOMWARE表实体类
 *
 **/
@Getter
@Setter
@TableName("COPIES_ANTI_RANSOMWARE")
public class CopiesAntiRansomware {
    /**
     * 副本Id
     */
    @TableId(value = "COPY_ID", type = IdType.INPUT)
    private String copyId;

    /**
     * 0：准备中；1:进行中；2：未感染；3：已感染；4：异常
     */
    private int status;

    /**
     * 侦测模型
     */
    private String model;

    /**
     * 侦测开始时间
     */
    private String detectionStartTime;

    /**
     * 侦测结束时间
     */
    private String detectionEndTime;

    /**
     * 保留时间
     */
    private int detectionDuration;

    /**
     * 侦测报告
     */
    private String report;

    /**
     * 副本检测结果 总文件大小
     */
    private long totalFileSize;

    /**
     * 副本检测结果 修改文件数量
     */
    private long changedFileCount;

    /**
     * 副本检测结果 新增文件数量
     */
    private long addedFileCount;

    /**
     * 副本检测结果 删除文件数量
     */
    private long deletedFileCount;

    /**
     * 副本检测结果 误报处理
     */
    @TableField("HANDLE_DETECT_INFECT")
    private boolean isHandleDetectInfect;

    /**
     * 副本检测结果生成方式。IO_DETECT：实时检测； COPY_DETECT：智能侦测
     */
    private String generateType;
}
