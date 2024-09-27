/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.framework.dee;

import openbackup.data.protection.access.provider.sdk.restore.v2.DeeCopiesRelatedTask;
import openbackup.system.base.common.rest.CommonFeignConfiguration;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;

/**
 * DEE侧实时侦测安全快照管理API
 *
 * @author f00809938
 * @since 2023-06-12
 * @version OceanCyber 300 1.1.0
 **/
@FeignClient(name = "copiesManagementRestApi",
    url = "${data-enable-engine-server.url}/v1/internal/anti/ransomware/snapshot/",
    configuration = CommonFeignConfiguration.class)
public interface DeeCopiesManagementRestApi {
    /**
     * 实时侦测安全快照恢复
     *
     * @param deeRestoreCopyTask deeRestoreCopyTask
     */
    @PostMapping("restore")
    void restoreFsSnapshot(@RequestBody DeeCopiesRelatedTask deeRestoreCopyTask);

    /**
     * 实时侦测安全快照删除
     *
     * @param deeDeleteCopyTask deeDeleteCopyTask
     */
    @DeleteMapping("delete")
    void createDeleteFsSnapshot(@RequestBody DeeCopiesRelatedTask deeDeleteCopyTask);

    /**
     * 实时侦测安全快照创建
     *
     * @param deeRestoreCopyTask deeRestoreCopyTask
     */
    @PostMapping("backup")
    void createBackupFsSnapshot(@RequestBody DeeCopiesRelatedTask deeRestoreCopyTask);
}
