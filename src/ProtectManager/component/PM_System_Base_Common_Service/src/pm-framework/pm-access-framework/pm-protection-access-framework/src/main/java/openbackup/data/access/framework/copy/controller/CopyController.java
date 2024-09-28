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
package openbackup.data.access.framework.copy.controller;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.client.sdk.api.framework.dme.AvailableTimeRanges;
import openbackup.data.access.framework.copy.controller.req.CatalogQueryReq;
import openbackup.data.access.framework.copy.controller.req.CopyVerifyRequest;
import openbackup.data.access.framework.copy.mng.service.CopyService;
import openbackup.data.access.framework.copy.verify.service.CopyVerifyTaskManager;
import openbackup.data.access.framework.core.copy.CopyManagerService;
import openbackup.data.access.framework.core.model.CopySummaryCount;
import openbackup.data.access.framework.core.model.CopySummaryResource;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.sdk.common.model.UuidObject;
import openbackup.system.base.sdk.dee.model.FineGrainedRestore;
import openbackup.system.base.sdk.user.enums.OperationTypeEnum;
import openbackup.system.base.sdk.user.enums.ResourceSetTypeEnum;
import openbackup.system.base.security.context.Context;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.security.journal.Logging;
import openbackup.system.base.security.permission.Permission;
import openbackup.system.base.util.DefaultRoleHelper;

import org.hibernate.validator.constraints.Range;
import org.springframework.validation.annotation.Validated;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import java.util.List;

import javax.validation.Valid;
import javax.validation.constraints.NotBlank;

/**
 * 副本相关的接口
 *
 */
@Slf4j
@RestController
@Validated
public class CopyController {
    // 指定副本资源查询的分页最大值
    private static final int MAX_PAGE_COPY_SUMMARY_RESOURCE = 200;

    private final CopyService copyService;

    private final CopyVerifyTaskManager copyVerifyTaskManager;

    private final CopyManagerService copyManagerService;

    /**
     * 副本控制器构造函数
     *
     * @param copyService 副本服务
     * @param copyVerifyTaskManager 副本校验管理器
     * @param copyManagerService 副本管理服务
     */
    public CopyController(CopyService copyService, CopyVerifyTaskManager copyVerifyTaskManager,
        CopyManagerService copyManagerService) {
        this.copyService = copyService;
        this.copyVerifyTaskManager = copyVerifyTaskManager;
        this.copyManagerService = copyManagerService;
    }

    /**
     * 浏览副本中文件和目录信息
     *
     * @param copyId 副本id
     * @param parentPath 根路径
     * @param pageSize 分页大小
     * @param pageNo 起始页
     * @param conditions 查询条件
     * @return 副本文件和目录信息
     */
    @ExterAttack
    @GetMapping("/v2/copies/{copyId}/catalogs")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN}, resources = "copy:$1",
        resourceSetType = ResourceSetTypeEnum.COPY, operation = OperationTypeEnum.QUERY, target = "#copyId")
    public PageListResponse<FineGrainedRestore> listCopyCatalogs(@PathVariable("copyId") String copyId,
        @NotBlank @RequestParam("parentPath") String parentPath,
        @RequestParam(value = "pageSize", defaultValue = "200", required = false) int pageSize,
        @RequestParam(value = "pageNo", defaultValue = "0", required = false) int pageNo,
        @RequestParam(value = "conditions", defaultValue = "{}", required = false) String conditions) {
        return copyService.listCopyCatalogs(copyId, parentPath, pageSize, pageNo, conditions);
    }

    /**
     * 浏览副本中文件和目录信息
     *
     * @param copyId 副本id
     * @param catalogQueryReq 查询参数
     * @return 副本文件和目录信息
     */
    @ExterAttack
    @GetMapping("/v2/copies/{copyId}/catalogs-name")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN}, resources = "copy:$1",
        resourceSetType = ResourceSetTypeEnum.COPY, operation = OperationTypeEnum.QUERY, target = "#copyId")
    public PageListResponse<FineGrainedRestore> listCopyCatalogs(@PathVariable("copyId") String copyId,
        @Valid CatalogQueryReq catalogQueryReq) {
        return copyService.listCopyCatalogsName(copyId, catalogQueryReq);
    }

    /**
     * 查询指定时间范围可用于恢复的时间段
     *
     * @param resourceId 资源id
     * @param startTime 开始时间
     * @param endTime 结束时间
     * @param pageSize 分页大小
     * @param pageNo 开始页
     * @return 可用于恢复的时间段
     */
    @ExterAttack
    @GetMapping("/v2/copies/available-time-ranges")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN}, resources = "resource:$1",
        enableCheckAuth = false)
    public PageListResponse<AvailableTimeRanges> listAvailableTimeRanges(@RequestParam("resourceId") String resourceId,
        @Range() @RequestParam("startTime") long startTime, @Range() @RequestParam("endTime") long endTime,
        @RequestParam(value = "pageSize", defaultValue = "100", required = false) int pageSize,
        @RequestParam(value = "pageNo", defaultValue = "0", required = false) int pageNo) {
        return copyService.listAvailableTimeRanges(resourceId, startTime, endTime, pageSize, pageNo);
    }

    /**
     * 校验副本
     *
     * @param copyId 副本id
     * @param copyVerifyRequest 副本校验参数
     * @return 任务响应对象
     */
    @ExterAttack
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN}, resources = "copy:$1",
        resourceSetType = ResourceSetTypeEnum.COPY, operation = OperationTypeEnum.MODIFY, target = "#copyId")
    @Logging(name = "0x2064033A0008", target = "CopyCatalog",
        details = {"$copy.resourceName", "$copy.resourceId", "$copy.displayTimestamp"},
        context = @Context(name = "copy", statement = "@copy_context_loader_get_by_id.call($1)", required = true))
    @PostMapping("/v2/copies/{copyId}/action/verify")
    public UuidObject verifyCopy(
        @PathVariable("copyId") String copyId,
        @RequestBody @Validated CopyVerifyRequest copyVerifyRequest) {
        return new UuidObject(copyVerifyTaskManager.init(copyId, copyVerifyRequest.getAgents()));
    }

    /**
     * 查询副本的关联副本
     *
     * @param copyId 副本ID
     * @return 关联的副本
     */
    @ExterAttack
    @GetMapping("/v1/internal/copies/associated")
    public List<String> queryAssociatedCopy(@RequestParam("copy_id") String copyId) {
        return copyManagerService.getAssociatedCopies(copyId);
    }

    /**
     * 副本资源列表查询v2
     *
     * @param pageSize 分页数据条数
     * @param pageNo 分页页面编码
     * @param orders 排序字段
     * @param conditions 条件参数
     * @return 副本资源列表
     */
    @ExterAttack
    @Permission(
        roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN, Constants.Builtin.ROLE_AUDITOR},
        enableCheckAuth = false)
    @GetMapping("/v2/copies/summary/resources")
    public PageListResponse<CopySummaryResource> queryCopySummaryResource(
        @RequestParam(name = "pageSize", defaultValue = "10") int pageSize,
        @RequestParam(name = "pageNo", defaultValue = "0") int pageNo,
        @RequestParam(name = "orders", required = false) String[] orders,
        @RequestParam(name = "conditions", required = false) String conditions) {
        if (pageSize < 0 || pageSize > MAX_PAGE_COPY_SUMMARY_RESOURCE) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "page size is invalid.");
        }
        return copyService.listCopyResourceSummary(pageNo, pageSize, conditions, orders);
    }

    /**
     * 查询副本统计信息
     *
     * @return 副本统计信息
     */
    @ExterAttack
    @Permission(
        roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN, Constants.Builtin.ROLE_AUDITOR},
        enableCheckAuth = false)
    @GetMapping("/v2/copies/summary/count")
    public List<CopySummaryCount> queryCopyCount() {
        TokenBo.UserBo userBo = TokenBo.get().getUser();
        String userId = userBo.getId();
        String domainId = userBo.getDomainId();
        if (DefaultRoleHelper.isAdmin(userId)) {
            domainId = null;
        }
        return copyManagerService.queryCopyCount(domainId);
    }

    /**
     * 关闭副本guest system
     *
     * @param copyId 副本id
     */
    @ExterAttack
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN}, resources = "copy:$1",
        resourceSetType = ResourceSetTypeEnum.COPY, operation = OperationTypeEnum.QUERY, target = "#copyId")
    @PutMapping("/v2/copies/{copyId}/close/guest-system")
    public void closeCopyGuestSystem(@PathVariable("copyId") String copyId) {
        copyService.closeCopyGuestSystem(copyId);
    }
}
