/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.sdk.resource;

import com.huawei.emeistor.kms.kmc.util.security.exterattack.ExterAttack;
import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.sdk.common.model.UuidObject;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.resource.model.ManualBackupReq;
import openbackup.system.base.sdk.resource.model.ProtectedObjectInfo;
import openbackup.system.base.sdk.resource.model.ProtectionBatchOperationReq;
import openbackup.system.base.sdk.resource.model.ProtectionCreationDto;
import openbackup.system.base.sdk.resource.model.ProtectionModifyDto;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestParam;

import java.util.List;

/**
 * Protect Object RestApi
 *
 * @author l00272247
 * @since 2021-01-11
 */
@FeignClient(name = "ProtectObjectRestApi", url = "${services.endpoints.protectmanager.protection-service}/v1/internal",
    configuration = CommonFeignConfiguration.class)
public interface ProtectObjectRestApi {
    /**
     * page query protect object
     *
     * @param slaId sla id
     * @param page page
     * @param size size
     * @return page data
     */
    @ExterAttack
    @GetMapping("/protected-objects")
    BasePage<ProtectedObjectInfo> pageQueryProtectObject(@RequestParam("sla_id") String slaId,
        @RequestParam("page_no") int page, @RequestParam("page_size") int size);

    /**
     * 根据资源id获取保护对象
     *
     * @param resourceId 资源id
     * @return 保护对象信息
     */
    @ExterAttack
    @GetMapping("/protected-objects/{resource_id}")
    ProtectedObjectInfo getProtectObject(@PathVariable("resource_id") String resourceId);

    /**
     * 禁用保护
     *
     * @param batchOperationReq 保护批处理操作请求体
     */
    @ExterAttack
    @PutMapping("/protected-objects/status/action/deactivate")
    void deactivate(@RequestBody ProtectionBatchOperationReq batchOperationReq);

    /**
     * 激活保护
     *
     * @param batchOperationReq 保护批处理操作请求体
     */
    @ExterAttack
    @PutMapping("/protected-objects/status/action/activate")
    void activate(@RequestBody ProtectionBatchOperationReq batchOperationReq);

    /**
     * 虚拟机组激活保护
     *
     * @param batchOperationReq 保护批处理操作请求体
     */
    @ExterAttack
    @PutMapping("/protected-objects/status/action/activate-resource-group")
    void activateResourceGroup(@RequestBody ProtectionBatchOperationReq batchOperationReq);

    /**
     * 虚拟机组禁用保护
     *
     * @param batchOperationReq 保护批处理操作请求体
     */
    @ExterAttack
    @PutMapping("/protected-objects/status/action/deactivate-resource-group")
    void deactivateResourceGroup(@RequestBody ProtectionBatchOperationReq batchOperationReq);

    /**
     * 执行手动备份
     *
     * @param resourceId 保护对象资源id
     * @param userId 用户id
     * @param backupReq 手动备份请求参数
     * @return 手动备份任务id列表
     */
    @ExterAttack
    @PostMapping("/protected-objects/{resource_id}/action/backup")
    List<String> manualBackup(@PathVariable("resource_id") String resourceId, @RequestParam("user_id") String userId,
        @RequestBody ManualBackupReq backupReq);

    /**
     * 删除受保护的资源
     *
     * @param protectedObjectsReq 请求体
     */
    @ExterAttack
    @DeleteMapping("/protected-objects")
    void deleteProtectedObjects(@RequestBody ProtectionBatchOperationReq protectedObjectsReq);

    /**
     * 删除受保护的复制资源
     *
     * @param protectedObjectsReq 请求体
     */
    @ExterAttack
    @DeleteMapping("/protected-copy-objects")
    void deleteCopyProtectedObjects(@RequestBody ProtectionBatchOperationReq protectedObjectsReq);

    /**
     * 创建保护对象
     *
     * @param userId 用户id
     * @param createReq 创建保护对象请求体
     * @return 创建保护任务id
     */
    @ExterAttack
    @PostMapping("/protected-objects/batch")
    List<String> createProtectedObject(@RequestParam("user_id") String userId,
        @RequestBody ProtectionCreationDto createReq);

    /**
     * 创建保护对象
     *
     * @param createReq 创建保护对象请求体
     * @return 创建保护任务id
     */
    @ExterAttack
    @PostMapping("/protected-objects")
    UuidObject createProtectedObjectInternal(@RequestBody ProtectionModifyDto createReq);

    /**
     * 修改保护对象
     *
     * @param userId 用户id
     * @param modifyReq 修改保护对象请求体
     * @return 修改保护任务id
     */
    @ExterAttack
    @PutMapping("/protected-objects")
    String modifyProtectedObject(@RequestParam("user_id") String userId, @RequestBody ProtectionModifyDto modifyReq);

    /**
     * 修改保护对象扩展字段
     *
     * @param modifyReq 修改保护对象请求体
     */
    @ExterAttack
    @PutMapping("/protected-objects/backup-esn")
    void modifyProtectedObjectExtParam(@RequestBody ProtectionModifyDto modifyReq);
}
