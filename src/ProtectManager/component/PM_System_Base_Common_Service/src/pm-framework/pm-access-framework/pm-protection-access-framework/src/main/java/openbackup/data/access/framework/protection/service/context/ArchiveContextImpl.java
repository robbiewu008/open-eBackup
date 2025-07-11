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
package openbackup.data.access.framework.protection.service.context;

import com.huawei.oceanprotect.sla.sdk.api.SlaQueryService;
import com.huawei.oceanprotect.sla.sdk.dto.SlaDto;

import openbackup.data.access.framework.protection.service.archive.ArchiveContext;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.protection.model.PolicyBo;

import org.apache.commons.lang3.StringUtils;

import java.util.Map;
import java.util.Optional;

/**
 * 归档业务上下文
 *
 **/
public class ArchiveContextImpl implements ArchiveContext {
    private final Map<String, String> contextMap;

    private final SlaQueryService slaQueryService;

    public ArchiveContextImpl(Map<String, String> contextMap, SlaQueryService slaQueryService) {
        this.contextMap = contextMap;
        this.slaQueryService = slaQueryService;
    }

    @Override
    public PolicyBo getPolicy() {
        final String policy = getContextValue(contextMap, ArchiveContext.ARCHIVE_POLICY_KEY,
                "archive policy can not find in context");
        return JSONObject.fromObject(policy).toBean(PolicyBo.class);
    }

    private String getContextValue(Map<String, String> contextMap, String contextKey, String errorMessage) {
        final String contextValue = contextMap.get(contextKey);
        if (StringUtils.isBlank(contextValue)) {
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, errorMessage);
        }
        return contextValue;
    }

    @Override
    public String getPolicyExtParams() {
        final String policy = getContextValue(contextMap, ArchiveContext.ARCHIVE_POLICY_KEY,
                "archive policy can not find in context");
        return JSONObject.fromObject(policy).getString(ArchiveContext.ARCHIVE_POLICY_EXT_PARAMS_KEY);
    }

    @Override
    public String getSlaJson() {
        SlaDto sla = slaQueryService.querySlaByName(getSlaName());
        return JSONObject.writeValueAsString(sla);
    }

    @Override
    public String getSlaName() {
        return getContextValue(contextMap, ArchiveContext.SLA_NAME_KEY, "sla name can not find in context");
    }

    @Override
    public boolean getManualArchiveTag() {
        String contextValue = contextMap.get(ArchiveContext.MANUAL_ARCHIVE);
        return !VerifyUtil.isEmpty(contextValue);
    }

    @Override
    public String getOriginalCopyId() {
        return getContextValue(contextMap, ArchiveContext.ORIGINAL_COPY_ID_KEY,
                "originalCopyId can not find in context");
    }

    @Override
    public String setArchiveCopyId(String archiveCopyId) {
        contextMap.put(ArchiveContext.ARCHIVE_COPY_ID_KEY, archiveCopyId);
        return contextMap.get(ArchiveContext.ARCHIVE_COPY_ID_KEY);
    }

    @Override
    public Optional<String> getArchiveCopyId() {
        return Optional.ofNullable(contextMap.getOrDefault(ArchiveContext.ARCHIVE_COPY_ID_KEY, null));
    }

    @Override
    public String getJobId() {
        return getContextValue(contextMap, ArchiveContext.JOB_ID, "job_id can not find in context");
    }

    @Override
    public int getRetryTimes() {
        return Optional.ofNullable(contextMap.get(ArchiveContext.AUTO_RETRY_TIMES)).map(Integer::valueOf).orElse(0);
    }
}
