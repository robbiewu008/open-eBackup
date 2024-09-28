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
package openbackup.system.base.common.exception;

import feign.FeignException;
import lombok.Getter;

/**
 * 设备管理器异常
 *
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
