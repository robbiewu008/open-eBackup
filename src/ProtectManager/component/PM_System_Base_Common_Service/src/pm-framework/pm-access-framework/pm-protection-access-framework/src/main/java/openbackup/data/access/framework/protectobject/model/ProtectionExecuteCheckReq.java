/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.access.framework.protectobject.model;

import openbackup.data.protection.access.provider.sdk.protection.model.ProtectObjectReqBase;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 保护创建与修改check请求
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2023/11/10
 */
@Getter
@Setter
public class ProtectionExecuteCheckReq extends ProtectObjectReqBase {
    private List<String> resourceIds;

    /**
     * REMOVE时，只传resourceIds
     */
    private String type;

    /**
     * 保护流程阶段
     */
    public enum ProtectionPhaseType {
        /**
         * 保护创建前
         */
        BEFORE_CREATE("BeforeCreate"),

        /**
         * 保护更新前
         */
        BEFORE_UPDATE("BeforeUpdate"),

        /**
         * 保护创建更行等失败
         */
        FAILED_ON_CREATE_OR_UPDATE("FailedOnCreateOrUpdate"),

        /**
         * 保护移除
         */
        REMOVE("Remove");

        private final String value;

        ProtectionPhaseType(String value) {
            this.value = value;
        }

        public String getValue() {
            return value;
        }
    }
}
