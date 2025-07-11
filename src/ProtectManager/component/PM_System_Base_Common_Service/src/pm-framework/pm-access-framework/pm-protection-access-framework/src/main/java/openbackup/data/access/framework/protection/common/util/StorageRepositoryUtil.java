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
package openbackup.data.access.framework.protection.common.util;

import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;

/**
 * Storage Repository Util
 *
 */
public class StorageRepositoryUtil {
    /**
     * 对本地的NFS/CFS协议进行转换，否则恢复时找不到对应协议的存储库
     *
     * @param repository 存储库信息
     * @return 返回实际的存储库协议
     */
    public static Integer getRepositoryProtocol(StorageRepository repository) {
        Integer protocol = repository.getProtocol();

        // 非本地的不进行转换
        if (!repository.getLocal()) {
            return protocol;
        }

        // 处理NFS和CIFS
        if (RepositoryProtocolEnum.NFS.getProtocol() == protocol) {
            protocol = RepositoryProtocolEnum.NATIVE_NFS.getProtocol();
        }
        if (RepositoryProtocolEnum.CIFS.getProtocol() == protocol) {
            protocol = RepositoryProtocolEnum.NATIVE_CIFS.getProtocol();
        }
        return protocol;
    }
}
