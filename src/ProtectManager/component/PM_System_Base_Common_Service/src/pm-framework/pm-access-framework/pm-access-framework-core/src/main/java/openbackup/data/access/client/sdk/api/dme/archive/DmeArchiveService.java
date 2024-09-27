/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.dme.archive;

import openbackup.data.access.client.sdk.api.config.achive.DmeArchiveFeignConfiguration;
import openbackup.data.access.client.sdk.api.config.achive.DmeResponse;
import openbackup.data.access.client.sdk.api.dme.archive.model.ArchiveCountDetail;
import openbackup.data.access.client.sdk.api.dme.archive.model.ArchiveDetail;
import openbackup.data.access.client.sdk.api.dme.archive.model.ArchiveUpdateDetail;
import openbackup.data.access.client.sdk.api.dme.archive.model.DmeArchiveAbortRequest;
import openbackup.data.access.client.sdk.api.dme.archive.model.DmeArchiveRequest;
import openbackup.data.access.client.sdk.api.dme.archive.model.DmeArchiveUpdateSnapRequest;
import openbackup.data.access.client.sdk.api.dme.archive.model.DmeCleanScanTaskRequest;
import openbackup.data.access.client.sdk.api.dme.archive.model.DmeCloudArchiveResponse;
import openbackup.data.access.client.sdk.api.dme.archive.model.DmeCloudBackupCopyInfo;
import openbackup.data.access.client.sdk.api.dme.archive.model.DmeDelCloudArchiveCopyRequest;
import openbackup.data.access.client.sdk.api.dme.archive.model.DmeS3ConnectRequest;
import openbackup.data.access.client.sdk.api.dme.archive.model.DmeScanArchiveRequestRequest;
import openbackup.data.access.client.sdk.api.dme.archive.model.GetScanCopyDetail;
import openbackup.system.base.common.model.repository.S3ConnectCheckResponse;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseBody;

/**
 * DME 归档接口
 *
 * @author y00490893
 * @version [BCManager 8.0.0]
 * @since 2020-12-16
 */
@FeignClient(name = "dmeArchiveService", url = "${dme-archive.url}", configuration = DmeArchiveFeignConfiguration.class)
public interface DmeArchiveService {
    /**
     * 创建归档副本
     *
     * @param archiveRequest 请求体
     * @return ArchiveDetail
     */
    @PostMapping("/v1/dme_archive/snap/create")
    @ResponseBody
    DmeResponse<ArchiveDetail> createArchive(@RequestBody DmeArchiveRequest archiveRequest);

    /**
     * DME更新归档副本PM元数据
     *
     * @param archiveUpdateSnapReq 请求体
     * @return DmeResponse
     */
    @PutMapping("/v1/dme_archive/updatesnap")
    @ResponseBody
    DmeResponse<ArchiveUpdateDetail> updateSnap(@RequestBody DmeArchiveUpdateSnapRequest archiveUpdateSnapReq);

    /**
     * 扫描归档副本
     *
     * @param scanArchiveRequestReq 请求体
     * @return DmeResponse
     */
    @PostMapping("/v1/dme_archive/snap/scan")
    @ResponseBody
    DmeResponse<ArchiveDetail> scanArchive(@RequestBody DmeScanArchiveRequestRequest scanArchiveRequestReq);

    /**
     * 扫描归档副本
     *
     * @param taskId Task ID
     * @return 扫描归档副本总数返回体
     */
    @GetMapping("/v1/dme_archive/snap/getscancount")
    @ResponseBody
    DmeResponse<ArchiveCountDetail> getScanArchiveCount(@RequestParam("TaskID") String taskId);

    /**
     * 获取扫描归档副本信息
     *
     * @param taskId Task ID
     * @param pageSize 分页大小
     * @param page 页码
     * @return 扫描归档副本信息列表返回体
     */
    @GetMapping("/v1/dme_archive/snap/getscancopy")
    @ResponseBody
    DmeResponse<GetScanCopyDetail> getScanArchiveCopy(@RequestParam("TaskID") String taskId,
        @RequestParam("PageSize") int pageSize, @RequestParam("Page") int page);

    /**
     * DME清理扫描归档任务
     *
     * @param cleanScanTaskReq cleanScanTaskReq
     * @return 清理扫描归档任务返回体
     */
    @PostMapping("/v1/dme_archive/snap/cleanscantask")
    @ResponseBody
    DmeResponse<Void> cleanScanArchiveTask(@RequestBody DmeCleanScanTaskRequest cleanScanTaskReq);

    /**
     * 终止归档任务
     *
     * @param archiveAbortRequest archiveAbortRequest
     */
    @PostMapping("/v1/dme_archive/task/abort")
    @ResponseBody
    void stopArchiveTask(@RequestBody DmeArchiveAbortRequest archiveAbortRequest);

    /**
     * 删除归档到云的副本
     *
     * @param request 请求体
     * @return 返回删除结果
     */
    @PostMapping("/v1/dme_archive/deletesnap")
    @ResponseBody
    DmeResponse<DmeCloudArchiveResponse> deleteCloudArchiveCopy(@RequestBody DmeDelCloudArchiveCopyRequest request);

    /**
     * 检查s3连通性
     *
     * @param request 连通性检查请求
     * @return 回应
     */
    @PostMapping("/v1/dme_archive/s3/checkconnect")
    @ResponseBody
    DmeResponse<S3ConnectCheckResponse> checkS3Connect(@RequestBody DmeS3ConnectRequest request);

    /**
     * 查询云备份副本信息
     *
     * @param copyId 副本ID
     * @return cloud backup备份副本信息
     */
    @GetMapping("/v1/dme_archive/copy/query")
    @ResponseBody
    DmeResponse<DmeCloudBackupCopyInfo> queryCloudBackupCopyInfo(@RequestParam("ArchiveCopyId") String copyId);

    /**
     * 给s3添加路由
     *
     * @param request 连通性检查请求
     * @return void
     */
    @PostMapping("/v1/dme_archive/route/add")
    DmeResponse<Void> addRoute(@RequestBody DmeS3ConnectRequest request);

    /**
     * 删除s3路由
     *
     * @param request 连通性检查请求
     * @return void
     */
    @DeleteMapping("/v1/dme_archive/route/delete")
    DmeResponse<Void> deleteRoute(@RequestBody DmeS3ConnectRequest request);
}
