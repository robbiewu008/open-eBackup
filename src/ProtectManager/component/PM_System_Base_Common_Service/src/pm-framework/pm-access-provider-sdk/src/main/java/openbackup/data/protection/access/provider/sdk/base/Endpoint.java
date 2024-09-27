/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.base;

import lombok.NoArgsConstructor;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 数据保护代理实体模型
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-11-01
 */
@NoArgsConstructor
public class Endpoint {
    // 代理主机id
    private String id;

    // 数据保护代理IP地址
    private String ip;

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
