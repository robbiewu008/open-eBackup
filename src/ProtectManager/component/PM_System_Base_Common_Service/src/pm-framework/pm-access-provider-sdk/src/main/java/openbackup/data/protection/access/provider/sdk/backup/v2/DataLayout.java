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
package openbackup.data.protection.access.provider.sdk.backup.v2;

import openbackup.data.protection.access.provider.sdk.base.v2.BaseDataLayout;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * 数据布局
 *
 */
public class DataLayout extends BaseDataLayout {
    // 是否开启目标端加密
    @JsonProperty("dstEncryption")
    private boolean isDstEncryption = Boolean.FALSE;

    // 是否开启目标端去重，默认开启，使用存储底座的去重功能
    @JsonProperty("dstDeduption")
    private boolean isDstDeduption = Boolean.TRUE;

    // 是否开启目标端压缩，默认开启，使用存储底座的压缩功能
    @JsonProperty("dstCompression")
    private boolean isDstCompression = Boolean.TRUE;

    // 是否开启源端加密
    @JsonProperty("srcEncryption")
    private boolean isSrcEncryption = Boolean.FALSE;

    // 是否开启源标端去重
    @JsonProperty("srcDeduption")
    private boolean isSrcDeduption = Boolean.FALSE;

    // 是否开启源标端压缩
    @JsonProperty("srcCompression")
    private boolean isSrcCompression = Boolean.FALSE;

    // 是否开启备份链路加密
    @JsonProperty("linkEncryption")
    private boolean isLinkEncryption = Boolean.FALSE;

    public boolean isDstEncryption() {
        return isDstEncryption;
    }

    public void setDstEncryption(boolean isDstEncryption) {
        this.isDstEncryption = isDstEncryption;
    }

    public boolean isDstDeduption() {
        return isDstDeduption;
    }

    public void setDstDeduption(boolean isDstDeduption) {
        this.isDstDeduption = isDstDeduption;
    }

    public boolean isDstCompression() {
        return isDstCompression;
    }

    public void setDstCompression(boolean isDstCompression) {
        this.isDstCompression = isDstCompression;
    }

    public boolean isSrcEncryption() {
        return isSrcEncryption;
    }

    public void setSrcEncryption(boolean isSrcEncryption) {
        this.isSrcEncryption = isSrcEncryption;
    }

    public boolean isSrcDeduption() {
        return isSrcDeduption;
    }

    public void setSrcDeduption(boolean isSrcDeduption) {
        this.isSrcDeduption = isSrcDeduption;
    }

    public boolean isSrcCompression() {
        return isSrcCompression;
    }

    public void setSrcCompression(boolean isSrcCompression) {
        this.isSrcCompression = isSrcCompression;
    }

    public boolean isLinkEncryption() {
        return isLinkEncryption;
    }

    public void setLinkEncryption(boolean isLinkEncryption) {
        this.isLinkEncryption = isLinkEncryption;
    }
}
