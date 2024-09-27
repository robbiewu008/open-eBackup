/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.framework.protection.common.util;

import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;

/**
 * Storage Repository Util
 *
 * @author l00272247
 * @since 20221-02-10
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
