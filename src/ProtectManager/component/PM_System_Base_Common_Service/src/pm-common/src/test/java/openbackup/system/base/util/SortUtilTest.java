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
package openbackup.system.base.util;

import openbackup.system.base.bean.SortRule;
import openbackup.system.base.sdk.cluster.enums.ClusterEnum;
import openbackup.system.base.sdk.cluster.model.MemberClusterInfo;
import openbackup.system.base.util.SortUtil;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.Arrays;
import java.util.List;

/**
 * SortUtilTest
 *
 */
@RunWith(PowerMockRunner.class)
@PowerMockIgnore( {"javax.management.*", "jdk.internal.reflect.*"})
public class SortUtilTest {
    /**
     * 用例场景：排序
     * 前置条件：NA
     * 检查点：排序正确
     */
    @Test
    public void test_sort1_success() throws Exception {
        // setup
        MemberClusterInfo memberClusterInfo1 = new MemberClusterInfo();
        memberClusterInfo1.setClusterName("node1");
        memberClusterInfo1.setStatus(26);
        MemberClusterInfo memberClusterInfo2 = new MemberClusterInfo();
        memberClusterInfo2.setClusterName("node2");
        memberClusterInfo2.setStatus(27);
        List<MemberClusterInfo> list = Arrays.asList(memberClusterInfo2, memberClusterInfo1);

        // run the test
        SortUtil.sort(list,
            new SortRule("clusterName"),
            new SortRule("status", ClusterEnum.StatusEnum.STATUS_ORDER_LIST));

        Assert.assertEquals("node1", list.get(0).getClusterName());
    }

    /**
     * 用例场景：排序
     * 前置条件：NA
     * 检查点：排序正确
     */
    @Test
    public void test_sort2_success() throws Exception {
        // setup
        MemberClusterInfo memberClusterInfo1 = new MemberClusterInfo();
        memberClusterInfo1.setClusterName("node2");
        memberClusterInfo1.setStatus(26);
        MemberClusterInfo memberClusterInfo2 = new MemberClusterInfo();
        memberClusterInfo2.setClusterName("node2");
        memberClusterInfo2.setStatus(27);
        List<MemberClusterInfo> list = Arrays.asList(memberClusterInfo2, memberClusterInfo1);

        // run the test
        SortUtil.sort(list,
            new SortRule("clusterName"),
            new SortRule("status", ClusterEnum.StatusEnum.STATUS_ORDER_LIST));

        Assert.assertEquals(27, list.get(0).getStatus().intValue());
    }
}