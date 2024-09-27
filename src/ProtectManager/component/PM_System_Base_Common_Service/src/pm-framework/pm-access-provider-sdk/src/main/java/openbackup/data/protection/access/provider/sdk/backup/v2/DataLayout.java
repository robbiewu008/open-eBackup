/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.backup.v2;

import openbackup.data.protection.access.provider.sdk.base.v2.BaseDataLayout;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * 数据布局
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-11-19
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
