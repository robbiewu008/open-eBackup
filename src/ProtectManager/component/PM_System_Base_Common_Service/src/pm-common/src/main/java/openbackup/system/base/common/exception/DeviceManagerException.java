/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.common.exception;

import feign.FeignException;
import lombok.Getter;

/**
 * 设备管理器异常
 *
 * @author w00493811
 * @since 2020-12-15
 */
@Getter
public class DeviceManagerException extends FeignException {
    private static final long serialVersionUID = 3760442843894510473L;

    private long code;

    private String desc;

    private String errorParam;

    /**
     * 默认构造函数
     *
     * @param code 异常编码
     * @param desc 异常信息
     * @param errorParam 异常参数
     */
    public DeviceManagerException(long code, String desc, String errorParam) {
        super(0, "");
        this.code = code;
        this.desc = desc;
        this.errorParam = errorParam;
    }

    /**
     * 转化为LegoCheckedException
     *
     * @return LegoCheckedException
     */
    public LegoCheckedException toLegoException() {
        // 底座错误码参数需要分割
        return new LegoCheckedException(code, errorParam.split("//,"), getDesc(), this);
    }
}