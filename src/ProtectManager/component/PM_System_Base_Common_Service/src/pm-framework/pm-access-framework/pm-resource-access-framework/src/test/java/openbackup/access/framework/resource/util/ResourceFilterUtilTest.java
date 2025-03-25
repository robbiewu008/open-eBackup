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
package openbackup.access.framework.resource.util;

import com.alibaba.fastjson.JSON;

import openbackup.access.framework.resource.persistence.model.VirtualResourceExtendPo;
import openbackup.data.protection.access.provider.sdk.resourcegroup.dto.ResourceGroupExtendParam;

import org.junit.Assert;
import org.junit.Test;

import java.util.ArrayList;
import java.util.List;

/**
 * 资源过滤工具类
 *
 */
public class ResourceFilterUtilTest {
    /**
     * 用例名称：按tag过滤。
     * 前置条件：无
     * check点：过滤满足的条件数据
     */
    @Test
    public void test_filter_resources_by_tag() {
        String filterStr = "{\"resource_tag_filters\":[{\"filter_by\":\"NAME\",\"type\":"
            + "\"VM\",\"rule\":\"START_WITH\",\"mode\":\"INCLUDE\",\"values\":[\"aaa\",\"bbb\"]},"
            + "{\"filter_by\":\"NAME\",\"type\":\"VM\",\"rule\":\"ALL\",\"mode\":\"INCLUDE\",\"values\":[\"eeee\"]},"
            + "{\"filter_by\":\"NAME\",\"type\":\"VM\",\"rule\":\"ALL\",\"mode\":\"EXCLUDE\",\"values\":[\"dddd\"]}]}";
        ResourceGroupExtendParam extendParam = JSON.parseObject(filterStr,
            ResourceGroupExtendParam.class);
        List<VirtualResourceExtendPo> virtualResources = ResourceFilterUtil.filterResources(mockVirtualResources(),
            extendParam);

        Assert.assertEquals(virtualResources.size(), 3);
    }

    /**
     * 用例名称：按name过滤。
     * 前置条件：无
     * check点：过滤满足的条件数据
     */
    @Test
    public void test_filter_resources_by_name() {
        String filterStr = "{\"resource_filters\":[{\"filter_by\":\"NAME\",\"type\":\"VM\",\"rule\":\"START_WITH\","
            + "\"mode\":\"INCLUDE\",\"values\":[\"lixin\",\"50G\"]},"
            + "{\"filter_by\":\"NAME\",\"type\":\"VM\",\"rule\":\"ALL\","
            + "\"mode\":\"INCLUDE\",\"values\":[\"del_win20222\"]},"
            + "{\"filter_by\":\"NAME\",\"type\":\"VM\",\"rule\":\"ALL\","
            + "\"mode\":\"EXCLUDE\",\"values\":[\"lixin2\"]}]}";
        ResourceGroupExtendParam extendParam = JSON.parseObject(filterStr,
            ResourceGroupExtendParam.class);
        List<VirtualResourceExtendPo> virtualResources = ResourceFilterUtil.filterResources(mockVirtualResources(),
            extendParam);

        Assert.assertEquals(virtualResources.size(), 3);
    }

    /**
     * 用例名称：按name,tag过滤。
     * 前置条件：无
     * check点：过滤满足的条件数据
     */
    @Test
    public void test_filter_resources_by_name_and_tag() {
        String filterStr = "{\"resource_tag_filters\":[{\"filter_by\":\"NAME\",\"type\":\"VM\",\"rule\":"
            + "\"FUZZY\",\"mode\":\"INCLUDE\",\"values\":[\"aaa\",\"bbb\"]},{\"filter_by\":"
            + "\"NAME\",\"type\":\"VM\",\"rule\":\"ALL\",\"mode\":\"INCLUDE\",\"values\":[\"eeee\"]},{\"filter_by\":"
            + "\"NAME\",\"type\":\"VM\",\"rule\":\"ALL\",\"mode\":\"EXCLUDE\",\"values\":"
            + "[\"dddd\"]}],\"resource_filters\":"
            + "[{\"filter_by\":\"NAME\",\"type\":\"VM\",\"rule\":\"FUZZY\",\"mode\":\"INCLUDE\",\"values\":"
            + "[\"lixin\",\"win\"]},{\"filter_by\":\"NAME\",\"type\":\"VM\","
            + "\"rule\":\"END_WITH\",\"mode\":\"EXCLUDE\",\"values\":[\"in1\"]}]}";
        ResourceGroupExtendParam extendParam = JSON.parseObject(filterStr,
            ResourceGroupExtendParam.class);
        List<VirtualResourceExtendPo> virtualResources = ResourceFilterUtil.filterResources(mockVirtualResources(),
            extendParam);

        Assert.assertEquals(virtualResources.size(), 2);
    }

    private List<VirtualResourceExtendPo> mockVirtualResources() {
        List<VirtualResourceExtendPo> virtualResources = new ArrayList<>();
        virtualResources.add(mockVirtualResource("aaa", "lixin1"));
        virtualResources.add(mockVirtualResource("bbbb", "lixin2"));
        virtualResources.add(mockVirtualResource("cccc", "lixin3"));
        virtualResources.add(mockVirtualResource("dddd,eeee", "del_win20222"));
        virtualResources.add(mockVirtualResource("eeee", "del_win202222"));
        return virtualResources;
    }

    private VirtualResourceExtendPo mockVirtualResource(String tag, String name) {
        VirtualResourceExtendPo po = new VirtualResourceExtendPo();
        po.setName(name);
        po.setTags(tag);
        return po;
    }
}
