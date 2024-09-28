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
package openbackup.data.access.client.sdk.api.framework.dme;

import openbackup.data.access.framework.core.common.enums.v2.CopyTypeEnum;
import openbackup.data.protection.access.provider.sdk.backup.v2.DataLayout;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;

import lombok.Data;

import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * DME 通用备份框架副本信息
 *
 */
@Data
public class DmeCopyInfo {
    private static final String KEY_COPY_VERIFY_FILE = "copyVerifyFile";

    // 副本Id
    private String id;

    // 副本对应的存储NAS快照信息
    private List<StorageSnapshot> snapshots;

    // 创建时间
    private long timestamp;

    // 状态：0：正常 1：删除中 2：归档中 3：复制中 4：无效 存入md
    private int status;

    // 原副本类型
    private String sourceCopyType;

    /**
     * 副本类型
     *
     * @see CopyTypeEnum
     */
    private String type;

    // 副本格式： 0：快照格式 1：目录格式 存入md
    private int format;

    private Long size;

    private TaskResource protectObject;

    private TaskEnvironment protectEnv;

    private List<TaskResource> protectSubObject;

    private List<StorageRepository> repositories;

    private DataLayout dataLayout;

    private Map<String, Object> extendInfo;

    /**
     * 根据dme副本中的校验文件获取副本校验状态
     *
     * @return 副本校验状态
     */
    public CopyVerifyStatusEnum getCopyVerifyStatus() {
        return Optional.of(this)
            .map(DmeCopyInfo::getExtendInfo)
            .map(extendInfo -> extendInfo.get(KEY_COPY_VERIFY_FILE))
            .map(copyVerifyFile -> {
                if (copyVerifyFile instanceof String && Boolean.parseBoolean((String) copyVerifyFile)) {
                    return CopyVerifyStatusEnum.NOT_VERIFY;
                }
                return CopyVerifyStatusEnum.VERIFY_FILE_NOT_EXIST;
            })
            .orElse(CopyVerifyStatusEnum.VERIFY_FILE_NOT_EXIST);
    }
}
