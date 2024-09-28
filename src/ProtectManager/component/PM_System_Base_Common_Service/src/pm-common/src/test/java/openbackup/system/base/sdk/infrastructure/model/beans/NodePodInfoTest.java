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
package openbackup.system.base.sdk.infrastructure.model.beans;

import openbackup.system.base.sdk.infrastructure.model.beans.NetPlaneInfo;
import openbackup.system.base.sdk.infrastructure.model.beans.NodePodInfo;

import org.junit.Assert;
import org.junit.Test;

import java.util.Arrays;
import java.util.List;

/**
 * NodePodInfo test
 *
 */
public class NodePodInfoTest {


    @Test
    public void get_all_ips_success() {
        NodePodInfo nodePodInfo = new NodePodInfo();
        List<NetPlaneInfo> netPlaneInfos = Arrays.asList(
                new NetPlaneInfo("197.0.0.1", "testName"),
                new NetPlaneInfo("199.0.0.1", ""));
        nodePodInfo.setNetPlaneInfos(netPlaneInfos);
        List<String> allIps = nodePodInfo.getAllIps();
        Assert.assertEquals(2, allIps.size());
    }
}
