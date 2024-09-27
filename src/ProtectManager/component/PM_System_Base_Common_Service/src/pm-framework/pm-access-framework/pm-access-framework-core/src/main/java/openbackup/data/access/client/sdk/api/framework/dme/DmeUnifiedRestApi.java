/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.framework.dme;

import openbackup.data.access.client.sdk.api.framework.dme.model.AgentLessActionRequest;
import openbackup.data.access.client.sdk.api.framework.dme.model.AgentLessRestoreRequest;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.v2.BaseTask;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountCreateTask;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.verify.CopyVerifyTask;
import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseBody;

import java.net.URI;
import java.util.List;
import java.util.Map;

/**
 * 统一备份框架DME相关接口定义
 *
 * @author y00559272
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-12-6
 **/
@FeignClient(
        name = "dmeTaskRestApi",
        url = "${services.endpoints.protectengine.ubc}",
        configuration = CommonFeignConfiguration.class)
public interface DmeUnifiedRestApi {
    /**
     * DME创建恢复任务接口
     *
     * @param uri uri
     * @param restoreTask 恢复任务参数
     */
    @PostMapping(path = "/v1/internal/dme-unified/tasks/restore")
    void createRestoreTask(URI uri, @RequestBody RestoreTask restoreTask);

    /**
     * DME创建备份任务接口
     *
     * @param uri uri
     * @param backupTask 备份任务参数
     */
    @PostMapping(path = "/v1/internal/dme-unified/tasks/backup")
    void createBackupTask(URI uri, @RequestBody BackupTask backupTask);

    /**
     * clone backup
     *
     * @param clone clone
     * @return copy id
     */
    @ExterAttack
    @PutMapping(path = "/v1/internal/dme-unified/tasks/backup/clone")
    String cloneBackup(@RequestBody DmeBackupClone clone);

    /**
     * 终止任务
     *
     * @param taskId 任务Id
     * @param requestId 请求Id，目前与requestId相同
     */
    @PutMapping(path = "/v1/internal/dme-unified/tasks/{task_id}/abort")
    void abortJob(@PathVariable("task_id") String taskId, @RequestParam("request_id") String requestId);

    /**
     * 查询数据面副本信息
     *
     * @param copyId 副本ID
     * @return 副本信息
     */
    @ExterAttack
    @GetMapping(path = "/v1/internal/dme-unified/copies/{copy_id}")
    DmeCopyInfo getCopyInfo(@PathVariable("copy_id") String copyId);

    /**
     * 删除副本
     *
     * @param copyId 副本ID
     * @param deleteCopyTask 删除副本任务
     */
    @DeleteMapping(path = "/v1/internal/dme-unified/copies/{copy_id}")
    void deleteCopy(@PathVariable("copy_id") String copyId, @RequestBody DeleteCopyTask deleteCopyTask);

    /**
     * 删除副本 指定uri
     *
     * @param uri 指定uri
     * @param copyId 副本ID
     * @param deleteCopyTask 删除副本任务
     */
    @DeleteMapping(path = "/v1/internal/dme-unified/copies/{copy_id}")
    void deleteCopyWithUri(URI uri, @PathVariable("copy_id") String copyId, @RequestBody DeleteCopyTask deleteCopyTask);

    /**
     * 执行校验副本任务
     *
     * @param uri uri
     * @param checkCopyTask 删除副本任务
     */
    @PostMapping(path = "/v1/internal/dme-unified/tasks/copy-verify")
    void checkCopy(URI uri, @RequestBody CopyVerifyTask checkCopyTask);

    /**
     * create live mount
     *
     * @param uri uri
     * @param task task
     * @return nothing
     */
    @PostMapping(path = "/v1/internal/dme-unified/tasks/livemount")
    Void createLiveMount(URI uri, @RequestBody LiveMountCreateTask task);

    /**
     * cancel live mount
     *
     * @param uri uri
     * @param task task
     */
    @PutMapping(path = "/v1/internal/dme-unified/tasks/livemount/cancel")
    void cancelLiveMountWithUri(URI uri, @RequestBody BaseTask task);

    /**
     * cancel live mount
     *
     * @param task task
     */
    @PutMapping(path = "/v1/internal/dme-unified/tasks/livemount/cancel")
    void cancelLiveMount(@RequestBody BaseTask task);

    /**
     * modify Qos
     *
     * @param copyId copy id
     * @param mountQos mount qos
     */
    @PutMapping(path = "/v1/internal/dme-unified/copies/{copy_id}/qos")
    void modifyQos(@RequestParam("copy_id") String copyId, DmeMountQos mountQos);

    /**
     * delete qos
     *
     * @param copyId copy id
     */
    @DeleteMapping(path = "/v1/internal/dme-unified/copies/{copy_id}/qos")
    void deleteQos(@RequestParam("copy_id") String copyId);

    /**
     * 查询指定时间范围可用于恢复的时间段
     *
     * @param resourceId 资源id
     * @param startTime 开始时间
     * @param endTime 结束时间
     * @param pageSize 分页大小
     * @param pageNo 开始页
     * @return 可用日志事件范围
     */
    @ExterAttack
    @GetMapping("/v1/internal/dme-unified/copies/timerange")
    @ResponseBody
    PageListResponse<AvailableTimeRanges> listAvailableTimeRanges(@RequestParam("protect_uuid") String resourceId,
        @RequestParam("start_time") long startTime, @RequestParam("end_time") long endTime,
        @RequestParam("page_size") int pageSize, @RequestParam("page_no") int pageNo);

    /**
     * scn 查询副本
     *
     * @param filterType 筛选条件
     * @param filterValue 筛选值
     * @param resourceId 资源类型
     * @param copyType 副本类型
     * @return 副本信息
     */
    @ExterAttack
    @GetMapping("/v1/internal/dme-unified/copies/all")
    @ResponseBody
    List<DmeCopyInfo> listCopiesInfo(@RequestParam("protect_uuid") String resourceId,
        @RequestParam("filter_type") String filterType, @RequestParam("filter_value") String filterValue,
        @RequestParam("copy_type") String copyType);

    /**
     * 移除资源的日志存储仓白名单
     *
     * @param params 请求体参数
     */
    @PostMapping(path = "/v1/internal/dme-unified/tasks/delete/resource")
    void removeRepoWhiteListOfResource(@RequestBody Map<String, Object> params);

    /**
     * 创建commonShare
     *
     * @param agentLessActionRequest 存储仓信息
     */
    @PostMapping(path = "/v1/internal/dme-unified/resource/file_system")
    void createCommonShare(@RequestBody AgentLessActionRequest agentLessActionRequest);

    /**
     * 更新commonShare 白名单
     *
     * @param agentLessActionRequest 存储仓信息
     */
    @PutMapping(path = "/v1/internal/dme-unified/resource/white_list")
    void updateCommonShareWhiteList(@RequestBody AgentLessActionRequest agentLessActionRequest);

    /**
     * 删除commonShare
     *
     * @param agentLessActionRequest 存储仓信息
     */
    @DeleteMapping(path = "/v1/internal/dme-unified/resource/file_system")
    void deleteCommonShare(@RequestBody AgentLessActionRequest agentLessActionRequest);

    /**
     * commonShare 创建备份任务
     *
     * @param commonShareBackupTask 备份任务
     */
    @PostMapping(path = "/v1/internal/dme-unified/tasks/agentless")
    void createCommonShareBackupTask(@RequestBody BackupTask commonShareBackupTask);

    /**
     * commonShare 创建克隆文件系统
     *
     * @param agentLessRestoreRequest 存储仓信息
     */
    @PostMapping(path = "/v1/internal/dme-unified/resource/clone_share")
    void createCommonShareCloneFilesystem(@RequestBody AgentLessRestoreRequest agentLessRestoreRequest);

    /**
     * commonShare 创建归档副本共享
     *
     * @param agentLessRestoreRequest 存储仓信息
     */
    @PostMapping(path = "/v1/internal/dme-unified/resource/archive_filesystem_share")
    void createArchiveCommonShare(@RequestBody AgentLessRestoreRequest agentLessRestoreRequest);
}
