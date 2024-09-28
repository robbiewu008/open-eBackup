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

import java.util.List;

/**
 * 功能描述
 *
 */
public class ProviderJobLogMessage {
    private String id;

    @JsonProperty("log_timestamp")
    private Long startTime;

    @JsonProperty("log_info")
    private String logInfo;

    @JsonProperty("log_info_param")
    private List<String> logInfoParam;

    @JsonProperty("log_detail")
    private String logDetail;

    @JsonProperty("log_detail_param")
    private List<String> logDetailParam;

    @JsonProperty("log_level")
    private Integer logLevel;

    @JsonProperty("log_detail_info")
    private List<String> logDetailInfo;

    public Long getStartTime() {
        return startTime;
    }

    public void setStartTime(Long startTime) {
        this.startTime = startTime;
    }

    public String getLogInfo() {
        return logInfo;
    }

    public void setLogInfo(String logInfo) {
        this.logInfo = logInfo;
    }

    public List<String> getLogInfoParam() {
        return logInfoParam;
    }

    public void setLogInfoParam(List<String> logInfoParam) {
        this.logInfoParam = logInfoParam;
    }

    public String getLogDetail() {
        return logDetail;
    }

    public void setLogDetail(String logDetail) {
        this.logDetail = logDetail;
    }

    public List<String> getLogDetailParam() {
        return logDetailParam;
    }

    public void setLogDetailParam(List<String> logDetailParam) {
        this.logDetailParam = logDetailParam;
    }

    public Integer getLogLevel() {
        return logLevel;
    }

    public void setLogLevel(Integer logLevel) {
        this.logLevel = logLevel;
    }

    public List<String> getLogDetailInfo() {
        return logDetailInfo;
    }

    public void setLogDetailInfo(List<String> logDetailInfo) {
        this.logDetailInfo = logDetailInfo;
    }

    public String getId() {
        return id;
    }

    public void setId(String id) {
        this.id = id;
    }
}
