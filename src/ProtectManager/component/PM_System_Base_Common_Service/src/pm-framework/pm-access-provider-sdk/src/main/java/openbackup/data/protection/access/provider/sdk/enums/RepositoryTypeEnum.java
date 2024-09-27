/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.enums;

import java.util.Arrays;

/**
 * 存储库类型的枚举类
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/12/9
 **/
public enum RepositoryTypeEnum {
    /**
     * 元数据存储库，用户存储备份元数据
     */
    META(0),
    /**
     * 数据存储库，用户存储备份数据
     */
    DATA(1),
    /**
     * 缓存存储库
     */
    CACHE(2),

    /**
     * 日志存储库
     */
    LOG(3);

    private final int type;

    RepositoryTypeEnum(int type) {
        this.type = type;
    }

    public int getType() {
        return type;
    }

    /**
     * 根据存储库型获取存储库类型枚举类
     *
     * @param type 存储库类型
     * @return 存储库类型枚举类 {@code RepositoryTypeEnum}
     */
    public static RepositoryTypeEnum getByType(int type) {
        return Arrays.stream(RepositoryTypeEnum.values())
                .filter(repositoryType -> repositoryType.type == type)
                .findFirst()
                .orElseThrow(IllegalArgumentException::new);
    }
}
