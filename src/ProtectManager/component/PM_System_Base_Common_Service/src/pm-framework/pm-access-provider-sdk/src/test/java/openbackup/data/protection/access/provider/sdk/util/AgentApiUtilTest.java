/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.util;

import openbackup.data.protection.access.provider.sdk.util.AgentApiUtil;
import openbackup.system.base.common.model.host.AgentManagementDomain;
import junit.framework.TestCase;
import org.junit.Assert;
import org.junit.Test;

import java.util.ArrayList;
import java.util.List;

/**
 * 功能描述
 *
 * @author t30028453
 * @version [DataBackup 1.3.0]
 * @since 2023-05-31
 */
public class AgentApiUtilTest extends TestCase {
    /**
     * 用例场景：选择一个pod，出现频率最高的
     * 前置条件：无
     * 检查点：选择一个pod，出现频率最高的
     */
    @Test
    public void test_selectDomain() {
        List<AgentManagementDomain> allDomain = getAgentManagementDomains();
        AgentManagementDomain selectDomain = AgentApiUtil.selectDomain(allDomain);
        Assert.assertTrue(selectDomain.getDomain().equals("protectengine-0"));
    }

    private List<AgentManagementDomain> getAgentManagementDomains() {
        List<AgentManagementDomain> allDomain = new ArrayList<>();
        for (int i = 0; i < 1; i++) {
            AgentManagementDomain domain = new AgentManagementDomain();
            if (i % 2 == 0) {
                domain.setDomain("protectengine-0");
            } else{
                domain.setDomain("protectengine-1");
            }
            domain.setPort(8090);
            allDomain.add(domain);
        }
        return allDomain;
    }
}