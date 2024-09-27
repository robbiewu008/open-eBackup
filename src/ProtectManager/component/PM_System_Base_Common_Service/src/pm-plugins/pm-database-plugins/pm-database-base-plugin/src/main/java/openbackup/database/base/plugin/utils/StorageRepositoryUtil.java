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
package openbackup.database.base.plugin.utils;

import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import java.util.List;
import java.util.Objects;

/**
 * 存储库相关工具类
 *
 * @author fwx1022842
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/7/5
 */
public class StorageRepositoryUtil {
    /**
     * 获取data存储库
     *
     * @param storageRepositories 存储库list
     * @return data 存储库
     */
    public static StorageRepository getDataStorageRepository(List<StorageRepository> storageRepositories) {
        return storageRepositories.stream()
                .filter(Objects::nonNull)
                .filter(repository -> repository.getType() == RepositoryTypeEnum.DATA.getType())
                .findFirst()
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM,
                        "Data repository info is empty!"));
    }
}
