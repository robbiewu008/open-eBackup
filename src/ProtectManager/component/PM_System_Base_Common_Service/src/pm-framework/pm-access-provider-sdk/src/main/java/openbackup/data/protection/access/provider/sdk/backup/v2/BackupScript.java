/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.backup.v2;

/**
 * 备份脚本
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-11-19
 */
public class BackupScript {
    // 前置脚本
    private String preScript;

    // 后置脚本
    private String postScript;

    // 失败脚本
    private String failPostScript;

    public String getPreScript() {
        return preScript;
    }

    public void setPreScript(String preScript) {
        this.preScript = preScript;
    }

    public String getPostScript() {
        return postScript;
    }

    public void setPostScript(String postScript) {
        this.postScript = postScript;
    }

    public String getFailPostScript() {
        return failPostScript;
    }

    public void setFailPostScript(String failPostScript) {
        this.failPostScript = failPostScript;
    }
}
