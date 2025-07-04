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
package openbackup.data.protection.access.provider.sdk.base;

import lombok.NoArgsConstructor;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 数据保护代理实体模型
 *
 */
@NoArgsConstructor
public class Endpoint {
    // 代理主机id
    private String id;

    // 数据保护代理IP地址
    private String ip;

    // 数据保护代理IP地址列表
    private String ipList;

    // 数据保护代理端口
    private int port;

    private String agentOS;

    // 数据保护代理wwpn配置
    private String[] wwpns;

    // 数据保护代理iqns配置
    private List<String> iqns;

    // 数据保护代理关联的SanClient主机列表
    private List<SanClientInfo> sanClients;

    // 高级备份参数，key/value键值对存放
    private Map<String, String> advanceParams = new HashMap<>();

    /**
     * 构造方法
     *
     * @param ip   port
     * @param port port
     */
    public Endpoint(String ip, int port) {
        this(null, ip, port);
    }

    /**
     * 构造方法
     *
     * @param id id
     * @param ip ip
     * @param port port
     */
    public Endpoint(String id, String ip, int port) {
        this.ip = ip;
        this.port = port;
        this.id = id;
    }

    /**
     * 构造方法
     *
     * @param id id
     * @param ip ip
     * @param port port
     * @param agentOS agentOS
     */
    public Endpoint(String id, String ip, int port, String agentOS) {
        this(id, ip, port);
        this.agentOS = agentOS;
    }

    public String getId() {
        return id;
    }

    public void setId(String id) {
        this.id = id;
    }

    public String getIp() {
        return ip;
    }

    public void setIp(String ip) {
        this.ip = ip;
    }

    public int getPort() {
        return port;
    }

    public void setPort(int port) {
        this.port = port;
    }

    public String getIpList() {
        return ipList;
    }

    public void setIpList(String ipList) {
        this.ipList = ipList;
    }

    public void setAgentOS(String agentOS) {
        this.agentOS = agentOS;
    }

    public String getAgentOS() {
        return agentOS;
    }

    public String[] getWwpns() {
        return wwpns;
    }

    public void setWwpns(String[] wwpns) {
        this.wwpns = wwpns;
    }

    public List<String> getIqns() {
        return iqns;
    }

    public void setIqns(List<String> iqns) {
        this.iqns = iqns;
    }

    public List<SanClientInfo> getSanClients() {
        return sanClients;
    }

    public void setSanClients(List<SanClientInfo> sanClients) {
        this.sanClients = sanClients;
    }

    public Map<String, String> getAdvanceParams() {
        if (advanceParams == null) {
            advanceParams = new HashMap<>();
        }
        return advanceParams;
    }

    public void setAdvanceParams(Map<String, String> advanceParams) {
        this.advanceParams = advanceParams;
    }

    /**
     * 设置高级参数
     *
     * @param key key
     * @param value value
     */
    public void setAdvanceParamsByKey(String key, String value) {
        if (advanceParams == null) {
            advanceParams = new HashMap<>();
        }
        advanceParams.put(key, value);
    }
}
