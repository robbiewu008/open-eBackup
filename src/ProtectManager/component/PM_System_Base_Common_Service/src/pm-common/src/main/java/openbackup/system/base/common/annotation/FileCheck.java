/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.common.annotation;

import openbackup.system.base.common.enums.FileTypeEnum;
import openbackup.system.base.common.validator.FileValidator;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Inherited;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

import javax.validation.Constraint;
import javax.validation.Payload;

/**
 * 文件上传防护注解（需要与@Validated配合使用）
 *
 * @author t30028453
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-27
 */
@Target({ElementType.PARAMETER, ElementType.FIELD, ElementType.TYPE, ElementType.METHOD, ElementType.TYPE_USE})
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Constraint(validatedBy = {FileValidator.class})
@Inherited
public @interface FileCheck {
    String message() default "Invalid file.";

    Class<?>[] groups() default {};

    Class<? extends Payload>[] payload() default {};

    /**
     * 文件类型
     * 默认OTHERS
     *
     * @return 文件类型
     */
    FileTypeEnum value() default FileTypeEnum.OTHERS;

    /**
     * 文件大小上限
     * 默认4G
     *
     * @return 文件大小上限
     */
    long maxSize() default 4L * 1024L * 1024L * 1024L;

    /**
     * 文件名长度上限
     * 默认512
     *
     * @return 文件名长度上限
     */
    int maxNameLength() default 512;

    /**
     * 压缩文件内文件数量上限
     * 默认1024
     *
     * @return 压缩文件内文件数量上限
     */
    int maxEntryNum() default 1024;

    /**
     * 压缩文件内单个文件大小上限
     * 默认4G
     *
     * @return 压缩文件内单个文件大小上限
     */
    long maxEntrySize() default 4L * 1024L * 1024L * 1024L;

    /**
     * 压缩文件深度上限
     * 默认1024
     *
     * @return 压缩文件深度上限
     */
    int maxDepth() default 1024;

    /**
     * 临时文件存放目录
     *
     * @return 临时文件存放目录
     */
    String tempPath() default "/tmp/zip/pre";

    /**
     * 允许的文件类型
     * 默认为空
     *
     * @return 允许的文件类型
     */
    String[] allowedFormats() default {};

    /**
     * 压缩文件解压缩大小上限
     * 默认4G
     *
     * @return 压缩文件解压缩大小上限
     */
    long maxUnzipSize() default 4L * 1024L * 1024L * 1024L;

    /**
     * zip包内文件白名单
     * 默认为空不校验
     *
     * @return zip包内文件白名单
     */
    String[] zipWhiteList() default {};
}
