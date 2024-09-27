/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.common.enums;

import openbackup.system.base.common.utils.files.FileCheckInterface;
import openbackup.system.base.common.utils.files.FileUtil;
import openbackup.system.base.common.utils.files.FileZip;

/**
 * 文件类型枚举类
 *
 * @author w00607005
 * @since 2023-09-21
 */
public enum FileTypeEnum {
    /**
     * zip文件
     */
    ZIP(new FileZip()),

    /**
     * 其他文件
     */
    OTHERS(new FileUtil());

    private final FileCheckInterface function;

    FileTypeEnum(FileCheckInterface function) {
        this.function = function;
    }

    public FileCheckInterface getFunction() {
        return function;
    }
}