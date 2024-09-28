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
package openbackup.openstack.adapter.controller;

import com.huawei.emeistor.kms.kmc.util.security.exterattack.ExterAttack;
import openbackup.openstack.adapter.constants.OpenStackConstants;
import openbackup.openstack.adapter.dto.OpenStackRestoreJobDto;
import openbackup.openstack.adapter.dto.RestoreJobsDto;
import openbackup.openstack.adapter.service.OpenStackRestoreAdapter;

import lombok.extern.slf4j.Slf4j;

import org.springframework.http.HttpStatus;
import org.springframework.validation.annotation.Validated;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.ResponseStatus;
import org.springframework.web.bind.annotation.RestController;

import javax.servlet.http.HttpServletRequest;

/**
 * 云核OpenStack恢复北向接口
 * <p>
 *     提供第三方调用，不使用系统内部token校验机制
 * </p>
 *
 */
@Slf4j
@RestController
@RequestMapping("/v2/backup_restore")
public class OpenStackRestoreController {
    private final OpenStackRestoreAdapter adapter;

    public OpenStackRestoreController(OpenStackRestoreAdapter adapter) {
        this.adapter = adapter;
    }

    /**
     * 创建恢复任务
     *
     * @param restoreJob 北向接口创建恢复任务请求体
     * @return 创建恢复任务响应体
     */
    @ExterAttack
    @ResponseStatus(HttpStatus.ACCEPTED)
    @PostMapping
    public OpenStackRestoreJobDto createJob(@RequestBody @Validated OpenStackRestoreJobDto restoreJob) {
        log.info(
                "Openstack create restore job, copy: {}, type: {}, instance: {}.",
                restoreJob.getCopyId(),
                restoreJob.getType().getType(),
                restoreJob.getInstanceId());
        return adapter.createJob(restoreJob);
    }

    /**
     * 查询恢复任务
     *
     * @param jobId 任务id
     * @return 恢复任务详情
     */
    @ExterAttack
    @GetMapping("/{id}")
    public OpenStackRestoreJobDto queryJob(@PathVariable("id") String jobId) {
        log.info("Openstack query restore job: {}.", jobId);
        return adapter.queryJob(jobId);
    }

    /**
     * 查询所有恢复任务
     *
     * @param request HttpServletRequest
     * @return 项目下所有资源的恢复任务
     */
    @ExterAttack
    @GetMapping
    public RestoreJobsDto queryJobs(HttpServletRequest request) {
        String projectId = request.getAttribute(OpenStackConstants.PROJECT_ID).toString();
        log.info("Openstack query all restore jobs of project: {}.", projectId);
        return new RestoreJobsDto(adapter.queryJobs(projectId));
    }
}
