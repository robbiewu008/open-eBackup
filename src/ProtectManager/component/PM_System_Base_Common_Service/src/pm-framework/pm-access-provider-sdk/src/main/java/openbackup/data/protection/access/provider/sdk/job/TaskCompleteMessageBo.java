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
package openbackup.data.protection.access.provider.sdk.job;

import com.fasterxml.jackson.annotation.JsonProperty;

import org.apache.commons.collections.MapUtils;

import java.util.HashMap;
import java.util.Map;

/**
 * dme task entity
 *
 */
public class TaskCompleteMessageBo extends TaskBaseMessage {
    private String taskId;
    @JsonProperty("job_status")
    private Integer jobStatus;

    @JsonProperty("job_progress")
    private Integer jobProgress;

    @JsonProperty("extend_info")
    private Map extendsInfo;

    @JsonProperty("additional_status")
    private String additionalStatus;

    // 存放任务完成的一些上下文信息
    private Map<String, String> context;

    private Long speed;

    public Integer getJobStatus() {
        return jobStatus;
    }

    public void setJobStatus(Integer jobStatus) {
        this.jobStatus = jobStatus;
    }

    public Integer getJobProgress() {
        return jobProgress;
    }

    public void setJobProgress(Integer jobProgress) {
        this.jobProgress = jobProgress;
    }

    public Map getExtendsInfo() {
        return extendsInfo;
    }

    public void setExtendsInfo(Map extendsInfo) {
        this.extendsInfo = extendsInfo;
    }

    public String getAdditionalStatus() {
        return additionalStatus;
    }

    public void setAdditionalStatus(String additionalStatus) {
        this.additionalStatus = additionalStatus;
    }

    /**
     * 获取上下文
     *
     * @return 上下文信息
     */
    public Map<String, String> getContext() {
        if (this.context == null) {
            this.context = new HashMap<>();
        }
        return context;
    }

    public void setContext(Map<String, String> context) {
        this.context = context;
    }

    /**
     * 从上下文中获取指定key对于的内容
     *
     * @param key 上下文中的key
     * @return 返回key对于的属性
     */
    public String getProperty(String key) {
        if (this.context == null) {
            return null;
        }
        return this.context.get(key);
    }

    /**
     * 从扩展参数中获取key对应的boolean值</br>
     *
     * <p>
     * 1.扩展参数为空时，返回false</br>
     * 2.扩展参数key对应的value不是Boolean时，返回false</br>
     * 3.扩展参数key对应的value是Boolean时，返回value</br>
     * </p>
     *
     * @param key 扩展参数中的key定义
     * @return key对应的boolean值
     */
    public boolean getBooleanFromExtendsInfo(String key) {
        if (MapUtils.isEmpty(extendsInfo)) {
            return false;
        }
        Object isDamaged = extendsInfo.getOrDefault(key, "false");
        if (isDamaged instanceof String) {
            return Boolean.parseBoolean((String) isDamaged);
        }
        return false;
    }

    public String getTaskId() {
        return taskId;
    }

    public void setTaskId(String taskId) {
        this.taskId = taskId;
    }

    /**
     * 任务完成扩展参数key定义
     *
     **/
    public static final class ExtendsInfoKeys {
        /**
         * 副本是否损坏
         */
        public static final String IS_DAMAGED = "isDamaged";

        private ExtendsInfoKeys() {
        }
    }

    public Long getSpeed() {
        return speed;
    }

    public void setSpeed(Long speed) {
        this.speed = speed;
    }
}
