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
package openbackup.data.access.framework.core.common.enums;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import lombok.Getter;

/**
 * DMC Job状态枚举类
 *
 */
@Getter
public enum DmcJobStatus {
    SUCCESS(3),
    ABORTED(4),
    FAIL(6),
    PARTIAL_SUCCESS(13),
    ABORT_FAILED(15);

    private final Integer status;

    DmcJobStatus(Integer status) {
        this.status = status;
    }

    /**
     * 根据状态获取枚举类
     *
     * @param status 状态值
     * @return DmcJobStatus
     */
    public static DmcJobStatus getByStatus(Integer status) {
        for (DmcJobStatus value : DmcJobStatus.values()) {
            if (value.status - status == 0) {
                return value;
            }
        }
        // throw exception
        return FAIL;
    }

    /**
     * get job status enum by str
     *
     * @param str str
     * @return job status enum
     */
    public static DmcJobStatus getByName(String str) {
        for (DmcJobStatus statusEnum : DmcJobStatus.values()) {
            if (statusEnum.name().equals(str)) {
                return statusEnum;
            }
        }
        throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "get job status enum fail");
    }

    /**
     * 判断当前状态是否是正确状态
     *
     * @return True/False
     */
    public boolean isSuccess() {
        return this == SUCCESS || this == PARTIAL_SUCCESS;
    }

    /**
     * 获取对应的保护任务状态
     *
     * @return 状态值
     */
    public int getProtectionStatus() {
        switch (this) {
            case SUCCESS:
                return 1;
            case ABORTED:
                return 3;
            case FAIL:
                return 0;
            case PARTIAL_SUCCESS:
                return 2;
        }
        return 0;
    }
}
