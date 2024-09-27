/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.ssh;

/**
 * SSHService
 *
 * @author l30057246
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/6/19
 */
public interface AgentRegisterService {
    /**
     * 检查当前agent是否在线
     *
     * @param ip endpoint信息
     * @return agent是否已经在线
     */
    boolean isAgentOnline(String ip);
}
