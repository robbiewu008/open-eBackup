/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.protection.service.context;

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
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2022/1/14
 **/
public class ArchiveContextImpl implements ArchiveContext {
    private final Map<String, String> contextMap;

    public ArchiveContextImpl(Map<String, String> contextMap) {
        this.contextMap = contextMap;
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
        return getContextValue(contextMap, ArchiveContext.SLA_KEY, "sla can not find in context");
    }

    @Override
    public String getSlaName() {
        return JSONObject.fromObject(this.getSlaJson()).getString(ArchiveContext.SLA_NAME_KEY);
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
