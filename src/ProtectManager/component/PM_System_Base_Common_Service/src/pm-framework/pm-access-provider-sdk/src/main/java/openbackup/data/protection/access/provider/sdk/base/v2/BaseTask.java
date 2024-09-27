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
package openbackup.data.protection.access.provider.sdk.base.v2;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.system.base.common.utils.StringUtil;
import openbackup.system.base.common.utils.VerifyUtil;

import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

/**
 * Live Mount Common Task Param
 *
 * @author l00272247
 * @since 2022-01-06
 */
public class BaseTask {
    private static final String SECRET = "secret";
    private static final String HCS_TOKEN = "hcs_token";

    private String requestId;

    private String taskId;

    private String copyId;

    private TaskEnvironment targetEnv;

    private TaskResource targetObject;

    private List<StorageRepository> repositories;

    private List<Endpoint> agents;

    /**
     * 数据布局
     */
    private BaseDataLayout dataLayout;

    /**
     * 获取dataLayout，如果为null则new一个实例
     *
     * @return BaseDataLayout实例
     */
    public BaseDataLayout getDataLayout() {
        if (this.dataLayout == null) {
            this.dataLayout = new BaseDataLayout();
        }
        return this.dataLayout;
    }

    public void setDataLayout(BaseDataLayout dataLayout) {
        this.dataLayout = dataLayout;
    }

    public String getRequestId() {
        return requestId;
    }

    public void setRequestId(String requestId) {
        this.requestId = requestId;
    }

    public String getTaskId() {
        return taskId;
    }

    public void setTaskId(String taskId) {
        this.taskId = taskId;
    }

    public String getCopyId() {
        return copyId;
    }

    public void setCopyId(String copyId) {
        this.copyId = copyId;
    }

    public TaskEnvironment getTargetEnv() {
        return targetEnv;
    }

    public void setTargetEnv(TaskEnvironment targetEnv) {
        this.targetEnv = targetEnv;
    }

    public TaskResource getTargetObject() {
        return targetObject;
    }

    public void setTargetObject(TaskResource targetObject) {
        this.targetObject = targetObject;
    }

    public List<StorageRepository> getRepositories() {
        return repositories;
    }

    public void setRepositories(List<StorageRepository> repositories) {
        this.repositories = repositories;
    }

    public List<Endpoint> getAgents() {
        return agents;
    }

    public void setAgents(List<Endpoint> agents) {
        this.agents = agents;
    }

    /**
     * 清理base task中的敏感信息
     */
    public void cleanBaseTaskAuthPwd() {
        // 清理环境中auth字段下的敏感信息
        if (targetEnv != null && targetEnv.getAuth() != null) {
            StringUtil.clean(targetEnv.getAuth().getAuthPwd());
            // 清理OP服务化HCS_TOKEN
            if (targetEnv.getExtendInfo() != null) {
                StringUtil.clean(targetEnv.getExtendInfo().get(HCS_TOKEN));
            }
        }
        if (targetObject != null && targetObject.getAuth() != null) {
            StringUtil.clean(targetObject.getAuth().getAuthPwd());
            if (!VerifyUtil.isEmpty(targetObject.getAuth().getExtendInfo())) {
                // 清理掉恢复任务中，kerberos中的secret信息
                StringUtil.clean(targetObject.getAuth().getExtendInfo().get(SECRET));
            }
        }
        // 清理每一个存储仓中的auth信息中的敏感数据
        Optional.ofNullable(repositories).orElse(new ArrayList<>()).forEach(StorageRepository::cleanAuth);
        if (targetEnv == null) {
            return ;
        }
        // 清理targetEnv 中每一个node节点中的auth信息中的敏感数据
        Optional.ofNullable(targetEnv.getNodes()).orElse(new ArrayList<>()).forEach(node -> {
            if (node != null && node.getAuth() != null) {
                StringUtil.clean(node.getAuth().getAuthPwd());
            }
        });
    }
}
