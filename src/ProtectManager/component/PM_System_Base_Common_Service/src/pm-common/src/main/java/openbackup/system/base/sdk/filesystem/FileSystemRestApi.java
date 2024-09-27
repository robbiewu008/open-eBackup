/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.sdk.filesystem;

import openbackup.system.base.common.constants.HyperMetroPair;
import openbackup.system.base.common.constants.RemoteFileSystem;
import openbackup.system.base.common.rest.CommonFeignConfiguration;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseBody;

import java.util.List;

/**
 * fileSystem REST调用接口
 *
 * @author jwx701567
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-09-05
 */
@FeignClient(
        name = "fileSystemRestApi",
        url = "${services.endpoints.protectmanager.system-base}",
        configuration = CommonFeignConfiguration.class)
public interface FileSystemRestApi {
    /**
     * 验证挂载文件系统的所有权
     *
     * @param userId 用户id
     * @param uuidList 文件系统的挂载点id
     */
    @GetMapping("/v1/internal/plugins/storage/nas/action/verify")
    @ResponseBody
    void verifyNasFileSystemMountIdOwnership(
            @RequestParam("userId") String userId, @RequestParam("uuidList") List<String> uuidList);

    /**
     * 查询文件系统信息
     *
     * @param deviceId 设备ID
     * @param filesystemId 文件系统ID
     * @return 文件系统信息
     */
    @GetMapping("/v1/internal/plugins/storage/filesystems/{filesystemId}")
    RemoteFileSystem getFileSystemDetail(
        @RequestParam(value = "deviceId") String deviceId, @PathVariable(value = "filesystemId") String filesystemId);

    /**
     * 通过双活pair id查询双活域信息
     *
     * @param deviceId 设备ID
     * @param id 双活Hyper metro pair ID
     * @return 文件系统双活域信息
     */
    @GetMapping("/v1/internal/plugins/storage/hypermetropair/{id}")
    HyperMetroPair getHyperMetroPair(
        @RequestParam(value = "deviceId") String deviceId, @PathVariable(name = "id") String id);
}
