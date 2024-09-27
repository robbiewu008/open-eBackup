/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.backup.handler.v1;

import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.data.protection.access.provider.sdk.backup.v1.BackupFollowUpProvider;
import openbackup.system.base.common.msg.NotifyManager;
import openbackup.system.base.common.utils.JSONObject;

import com.google.common.collect.Lists;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

/**
 * The DefualtBackupFollowUpProvider
 *
 * @author g30003063
 * @since 2022/2/9
 */
@Component
public class DefaultBackupFollowUpProvider implements BackupFollowUpProvider {
    private final NotifyManager notifyManager;

    /**
     * 全参数构造
     *
     * @param notifyManager 消息提示器
     */
    public DefaultBackupFollowUpProvider(final NotifyManager notifyManager) {
        this.notifyManager = notifyManager;
    }

    @Override
    public boolean applicable(final String object) {
        return false;
    }

    @Override
    public void handleSuccess(final String requestId, final String jobId, final Integer status, final String copyId) {
        backupDone(requestId, jobId, status, copyId);
    }

    @Override
    public void handleFailure(final String requestId, final String jobId, final Integer status) {
        backupDone(requestId, jobId, status, null);
    }

    private void backupDone(String requestId, String jobId, Integer status, String copyId) {
        JSONObject backupDoneMap = new JSONObject();
        backupDoneMap.put(ContextConstants.REQUEST_ID, requestId);
        backupDoneMap.put("job_id", jobId);
        backupDoneMap.put("status", status);
        if (StringUtils.isBlank(copyId)) {
            backupDoneMap.put("copy_ids", "");
        } else {
            backupDoneMap.put("copy_ids", Lists.newArrayList(copyId));
        }
        notifyManager.send(TopicConstants.EXECUTE_BACKUP_DONE, backupDoneMap.toString());
    }
}
