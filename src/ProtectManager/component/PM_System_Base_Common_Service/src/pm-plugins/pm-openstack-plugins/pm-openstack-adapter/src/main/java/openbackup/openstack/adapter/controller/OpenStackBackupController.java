/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.openstack.adapter.controller;

import com.huawei.emeistor.kms.kmc.util.security.exterattack.ExterAttack;
import openbackup.openstack.adapter.constants.OpenStackConstants;
import openbackup.openstack.adapter.dto.BatchCreateBackupJobDto;
import openbackup.openstack.adapter.dto.OpenStackBackupJobDto;
import openbackup.openstack.adapter.dto.UpdateBackupJobDto;
import openbackup.openstack.adapter.enums.SwitchAction;
import openbackup.openstack.adapter.service.OpenStackBackupAdapter;

import lombok.extern.slf4j.Slf4j;

import org.springframework.http.HttpStatus;
import org.springframework.validation.annotation.Validated;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.ResponseStatus;
import org.springframework.web.bind.annotation.RestController;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import javax.servlet.http.HttpServletRequest;
import javax.validation.constraints.Size;

/**
 * 云核OpenStack备份北向接口
 * <p>
 *     提供第三方调用，不使用系统内部token校验机制
 * </p>
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2022-12-22
 */
@Slf4j
@Validated
@RestController
@RequestMapping("/v2/backup_jobs")
public class OpenStackBackupController {
    private final OpenStackBackupAdapter adapter;

    public OpenStackBackupController(OpenStackBackupAdapter adapter) {
        this.adapter = adapter;
    }

    /**
     * 开关备份任务，根据字段名执行开或关的操作
     *
     * @param resourceId 任务id，对应系统中资源id
     * @param action 开关动作
     */
    @ExterAttack
    @ResponseStatus(HttpStatus.ACCEPTED)
    @PostMapping("/{id}/action")
    public void switchJob(@PathVariable("id") String resourceId,
        @RequestBody @Size(min = 1, max = 1) Map<SwitchAction, Object> action) {
        log.info("Request action: {}", action.keySet());
        if (action.containsKey(SwitchAction.STOP)) {
            adapter.stopJob(resourceId);
        } else {
            adapter.startJob(resourceId);
        }
    }

    /**
     * 查询备份任务详情，路径参数id对应系统中资源id
     *
     * @param resourceId 资源id
     * @return {@link OpenStackBackupJobDto} OpenStackBackupJobDto
     */
    @ExterAttack
    @GetMapping("/{id}")
    public OpenStackBackupJobDto queryJob(@PathVariable("id") String resourceId) {
        log.info("Query openstack backup job: {}", resourceId);
        return adapter.queryJob(resourceId);
    }

    /**
     * 查询所有备份任务
     *
     * @param request HttpServletRequest
     * @return 备份任务列表
     */
    @ExterAttack
    @GetMapping
    public List<OpenStackBackupJobDto> queryJobs(HttpServletRequest request) {
        String projectId = request.getAttribute(OpenStackConstants.PROJECT_ID).toString();
        List<OpenStackBackupJobDto> backupJobs = adapter.queryJobs(projectId);
        log.info("Query openstack backup jobs size: {}", backupJobs.size());
        return backupJobs;
    }

    /**
     * 批量创建备份任务
     *
     * @param req 批量创建备份任务请求
     * @return 任务响应体
     */
    @ExterAttack
    @ResponseStatus(HttpStatus.CREATED)
    @PostMapping
    public BatchCreateBackupJobDto createJob(@RequestBody @Validated BatchCreateBackupJobDto req) {
        log.info("Batch create backup job, size: {}", req.getBackupJobs().size());
        List<OpenStackBackupJobDto> results = new ArrayList<>();
        req.getBackupJobs().forEach(backupJob -> results.add(adapter.createJob(backupJob)));
        return new BatchCreateBackupJobDto(results);
    }

    /**
     * 更新备份任务
     *
     * @param resourceId 资源id
     * @param req {@link OpenStackBackupJobDto} 修改备份任务
     * @return 更新任务响应请求体
     */
    @ExterAttack
    @ResponseStatus(HttpStatus.CREATED)
    @PutMapping("/{id}")
    public OpenStackBackupJobDto updateJob(@PathVariable("id") String resourceId,
        @RequestBody @Validated UpdateBackupJobDto req) {
        log.info("Openstack update job, resource: {}.", resourceId);
        return adapter.updateJob(resourceId, req.getBackupJob());
    }

    /**
     * 删除备份任务，路径参数id对应系统中资源id
     *
     * @param resourceId 资源id
     */
    @ExterAttack
    @DeleteMapping("/{id}")
    public void deleteJob(@PathVariable("id") String resourceId) {
        log.info("Delete backup job: {}.", resourceId);
        adapter.deleteJob(resourceId);
    }
}
