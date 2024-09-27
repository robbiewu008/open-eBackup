/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.framework.agent.dto;

import openbackup.data.access.client.sdk.api.framework.agent.dto.model.WwpnOrIqnInfo;

import lombok.Data;

import java.util.List;

/**
 * agent wwpn返回信息
 *
 * @author l30023229
 * @since 2023-02-21
 */
@Data
public class AgentWwpnInfo extends AgentBaseDto {
    private String uuid;

    /**
     * 查询WWPNS返回的响应体中代理类型
     */
    private String type;

    private List<String> wwpns;

    /**
     * 查询wwpns或校验IQN返回的响应体中k-v
     */
    private List<WwpnOrIqnInfo> wwpnInfoList;
}
