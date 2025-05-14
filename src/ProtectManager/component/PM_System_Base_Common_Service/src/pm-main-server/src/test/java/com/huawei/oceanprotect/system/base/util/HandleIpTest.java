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
package com.huawei.oceanprotect.system.base.util;

import com.huawei.oceanprotect.system.base.initialize.network.beans.InitResource;
import com.huawei.oceanprotect.system.base.initialize.network.common.ExpansionIpSegment;
import com.huawei.oceanprotect.system.base.initialize.network.common.Ipv4Resource;
import com.huawei.oceanprotect.system.base.initialize.network.common.Ipv6Resource;
import com.huawei.oceanprotect.system.base.initialize.network.util.HandleIpUtils;

import org.junit.Assert;
import org.junit.Test;

import java.util.ArrayList;
import java.util.List;

/**
 * IP段处理合并公共测试类
 *
 */
public class HandleIpTest {

    /**
     * 测试 old Ipv4分离
     */
    @Test
    public void test_deleteIpv4LogicPortIp_success() {
        Ipv4Resource ipv4Resource = new Ipv4Resource();
        ipv4Resource.setStartIp("192.168.50.10");
        ipv4Resource.setEndIp("192.168.50.19");
        Ipv4Resource newIpv4Resource = HandleIpUtils.deleteIpv4LogicPortIp(ipv4Resource, 2);
        Assert.assertTrue("192.168.50.14".equals(newIpv4Resource.getStartIp()));
        Assert.assertTrue("192.168.50.19".equals(newIpv4Resource.getEndIp()));
    }

    /**
     * 测试 old Ipv6分离
     */
    @Test
    public void test_deleteIpv6LogicPortIp_success() {
        Ipv6Resource ipv6Resource = new Ipv6Resource();
        ipv6Resource.setStartIp("2016:8:40:96:c11::90");
        ipv6Resource.setEndIp("2016:8:40:96:c11::99");
        Ipv6Resource newIpv6Resource = HandleIpUtils.deleteIpv6LogicPortIp(ipv6Resource, 2);
        Assert.assertTrue("2016:8:40:96:c11::94".equals(newIpv6Resource.getStartIp()));
        Assert.assertTrue("2016:8:40:96:c11::99".equals(newIpv6Resource.getEndIp()));
    }

    /**
     * 测试 old Ipv4分离
     */
    @Test
    public void test_no_ipv4List_getIpv4NetPlaneList_success() {
        Ipv4Resource ipv4Resource = new Ipv4Resource();
        ipv4Resource.setStartIp("192.168.50.10");
        ipv4Resource.setEndIp("192.168.50.19");
        List<Ipv4Resource> ipv4NetPlaneList = HandleIpUtils.getIpv4NetPlaneList(new ArrayList<>(), ipv4Resource, 2);
        Assert.assertTrue("192.168.50.14".equals(ipv4NetPlaneList.get(0).getStartIp()));
        Assert.assertTrue("192.168.50.19".equals(ipv4NetPlaneList.get(0).getEndIp()));
    }

    /**
     * 测试 old Ipv6分离
     */
    @Test
    public void test_no_ipv4List_getIpv6NetPlaneList_success() {
        Ipv6Resource ipv6Resource = new Ipv6Resource();
        ipv6Resource.setStartIp("2016:8:40:96:c11::90");
        ipv6Resource.setEndIp("2016:8:40:96:c11::99");
        List<Ipv6Resource> ipv6NetPlaneList = HandleIpUtils.getIpv6NetPlaneList(new ArrayList<>(), ipv6Resource, 2);
        Assert.assertTrue("2016:8:40:96:c11::94".equals(ipv6NetPlaneList.get(0).getStartIp()));
        Assert.assertTrue("2016:8:40:96:c11::99".equals(ipv6NetPlaneList.get(0).getEndIp()));
    }


    /**
     * 测试 归档 old Ipv4分离
     */
    @Test
    public void test_no_ipv4List_get_archive_ipv4NetPlaneList_success() {
        Ipv4Resource ipv4Resource = new Ipv4Resource();
        ipv4Resource.setStartIp("192.168.50.10");
        ipv4Resource.setEndIp("192.168.50.19");
        List<Ipv4Resource> ipv4NetPlaneList = HandleIpUtils.getArchiveIpv4NetPlaneList(new ArrayList<>(), ipv4Resource);
        Assert.assertTrue("192.168.50.10".equals(ipv4NetPlaneList.get(0).getStartIp()));
        Assert.assertTrue("192.168.50.19".equals(ipv4NetPlaneList.get(0).getEndIp()));
    }

    /**
     * 测试 归档 old Ipv6分离
     */
    @Test
    public void test_no_ipv4List_get_archive_ipv6NetPlaneList_success() {
        Ipv6Resource ipv6Resource = new Ipv6Resource();
        ipv6Resource.setStartIp("2016:8:40:96:c11::90");
        ipv6Resource.setEndIp("2016:8:40:96:c11::99");
        List<Ipv6Resource> ipv6NetPlaneList = HandleIpUtils.getArchiveIpv6NetPlaneList(new ArrayList<>(), ipv6Resource);
        Assert.assertTrue("2016:8:40:96:c11::90".equals(ipv6NetPlaneList.get(0).getStartIp()));
        Assert.assertTrue("2016:8:40:96:c11::99".equals(ipv6NetPlaneList.get(0).getEndIp()));
    }

    /**
     * 用例场景：对于HandleIpUtils.compareIpv4 方法 进行测试
     * 前置条件：新的Ipv4 与 老的Ipv4 Mask参数相同
     * 检查点：新的Ipv4 与 老的Ipv4 个数不同返回False
     */
    @Test
    public void test_compare_ipv4_count_fail() {
        List<Ipv4Resource> oldIpv4List = new ArrayList<>();
        Ipv4Resource ipv4Resource1 = new Ipv4Resource();
        ipv4Resource1.setStartIp("192.168.50.10");
        ipv4Resource1.setEndIp("192.168.50.20");
        ipv4Resource1.setMask("255.255.0.0");

        oldIpv4List.add(ipv4Resource1);
        List<Ipv4Resource> newIpv4List = new ArrayList<>();
        Ipv4Resource ipv4Resource2 = new Ipv4Resource();
        ipv4Resource2.setStartIp("192.168.50.21");
        ipv4Resource2.setEndIp("192.168.50.30");
        ipv4Resource2.setMask("255.255.0.0");
        newIpv4List.add(ipv4Resource1);
        newIpv4List.add(ipv4Resource2);

        Assert.assertFalse(HandleIpUtils.compareIpv4(newIpv4List, oldIpv4List));
    }

    /**
     * 用例场景：对于HandleIpUtils.compareIpv4 方法 进行测试
     * 前置条件：新的Ipv4 与 老的Ipv4 IP个数相同
     * 检查点：新的Ipv4 与 老的Ipv4 mask不同 返回False
     */
    @Test
    public void test_compare_ipv4_fail() {
        List<Ipv4Resource> oldIpv4List = new ArrayList<>();
        Ipv4Resource ipv4Resource1 = new Ipv4Resource();
        ipv4Resource1.setStartIp("192.168.50.10");
        ipv4Resource1.setEndIp("192.168.50.20");
        ipv4Resource1.setMask("255.255.0.0");

        oldIpv4List.add(ipv4Resource1);
        List<Ipv4Resource> newIpv4List = new ArrayList<>();
        Ipv4Resource ipv4Resource2 = new Ipv4Resource();
        ipv4Resource2.setStartIp("192.168.50.10");
        ipv4Resource2.setEndIp("192.168.50.20");
        ipv4Resource2.setMask("255.255.255.0");
        newIpv4List.add(ipv4Resource2);

        Assert.assertFalse(HandleIpUtils.compareIpv4(newIpv4List, oldIpv4List));
    }

    /**
     * 用例场景：对于HandleIpUtils.getExpansionIpv4List 方法 进行测试
     * 前置条件：NA
     * 检查点：校验新的IPV4对象的值和原先需要配置的值是否一致
     */
    @Test
    public void get_expansion_ipv4_list_success() {
        List<ExpansionIpSegment> netPlane = new ArrayList<>();
        ExpansionIpSegment expansion1 = new ExpansionIpSegment();
        expansion1.setStartIp("192.168.100.10");
        expansion1.setEndIp("192.168.100.13");
        ExpansionIpSegment expansion2 = new ExpansionIpSegment();
        expansion2.setStartIp("192.168.100.18");
        expansion2.setEndIp("192.168.100.20");
        netPlane.add(expansion1);
        netPlane.add(expansion2);
        List<Ipv4Resource> expansionIpv4List = HandleIpUtils.getExpansionIpv4List("255.255.0.0", netPlane);
        Assert.assertEquals(2, expansionIpv4List.size());
        Assert.assertEquals(expansion1.getStartIp(),expansionIpv4List.get(0).getStartIp());
        Assert.assertEquals(expansion1.getEndIp(),expansionIpv4List.get(0).getEndIp());
        Assert.assertEquals("255.255.0.0",expansionIpv4List.get(0).getMask());
        Assert.assertEquals(expansion2.getStartIp(),expansionIpv4List.get(1).getStartIp());
        Assert.assertEquals(expansion2.getEndIp(),expansionIpv4List.get(1).getEndIp());
        Assert.assertEquals("255.255.0.0",expansionIpv4List.get(1).getMask());
    }

    /**
     * 用例场景：对于HandleIpUtils.getExpansionIpv6List 方法 进行测试
     * 前置条件：NA
     * 检查点：校验新的IPV6对象的值和原先需要配置的值是否一致
     */
    @Test
    public void get_expansion_ipv6_list_success() {
        List<ExpansionIpSegment> netPlane = new ArrayList<>();
        ExpansionIpSegment expansion1 = new ExpansionIpSegment();
        expansion1.setStartIp("2016:8:40:96:c11::90");
        expansion1.setEndIp("2016:8:40:96:c11::92");
        ExpansionIpSegment expansion2 = new ExpansionIpSegment();
        expansion2.setStartIp("2016:8:40:96:c11::95");
        expansion2.setEndIp("2016:8:40:96:c11::99");
        netPlane.add(expansion1);
        netPlane.add(expansion2);
        List<Ipv6Resource> expansionIpv6List = HandleIpUtils.getExpansionIpv6List("64", netPlane);
        Assert.assertEquals(2, expansionIpv6List.size());
        Assert.assertEquals(expansion1.getStartIp(),expansionIpv6List.get(0).getStartIp());
        Assert.assertEquals(expansion1.getEndIp(),expansionIpv6List.get(0).getEndIp());
        Assert.assertEquals("64",expansionIpv6List.get(0).getPrefix());
        Assert.assertEquals(expansion2.getStartIp(),expansionIpv6List.get(1).getStartIp());
        Assert.assertEquals(expansion2.getEndIp(),expansionIpv6List.get(1).getEndIp());
        Assert.assertEquals("64",expansionIpv6List.get(1).getPrefix());
    }

    /**
     * 用例场景：对于HandleIpUtils.isIpv4OrIpv6 方法 进行测试
     * 前置条件：NA
     * 检查点：校验传入参数是ipv4或者ipv6
     */
    @Test
    public void Is_ipv4_or_ipv6_success() {
        Assert.assertTrue(HandleIpUtils.isIpv4OrIpv6("192.168.100.10"));
        Assert.assertTrue(HandleIpUtils.isIpv4OrIpv6("2016:8:40:96:c11::90"));
    }

    @Test
    public void test_get_empty_ipv4_netplane_list() {
        HandleIpUtils.getIpv4NetPlaneList(new ArrayList<>(),new Ipv4Resource(),0);
        Assert.assertTrue(true);
    }

    @Test
    public void test_get_empty_ipv6_netplane_list() {
        HandleIpUtils.getIpv6NetPlaneList(new ArrayList<>(),new Ipv6Resource(),0);
        Assert.assertTrue(true);
    }

    @Test
    public void test_get_empty_ipv4_archive_netplane_list() {
        HandleIpUtils.getArchiveIpv4NetPlaneList(new ArrayList<>(),new Ipv4Resource());
        Assert.assertTrue(true);
    }

    @Test
    public void test_get_empty_ipv6_archive_netplane_list() {
        HandleIpUtils.getArchiveIpv6NetPlaneList(new ArrayList<>(),new Ipv6Resource());
        Assert.assertTrue(true);
    }

    @Test
    public void test_get_ipv4_netplane_range() {
        HandleIpUtils.getIpv4netPlaneRange(new InitResource("8.40.102.105", "8.40.102.106", "255.255.0.0"));
        Assert.assertTrue(true);
    }

    @Test
    public void test_get_ipv6_netplane_range() {
        HandleIpUtils.getIpv6netPlaneRange(new InitResource("2016:8:40:96:c11::90", "2016:8:40:96:c11::92", "255.255.0.0"));
        Assert.assertTrue(true);
    }
    @Test
    public void test_get_ipv4_ip_segment_list() {
        HandleIpUtils.getIpv4ExpansionIpSegmentList(new ArrayList<>(),new Ipv4Resource(),2,true);
        HandleIpUtils.getIpv4ExpansionIpSegmentList(new ArrayList<>(),new Ipv4Resource(),2,false);
        Assert.assertTrue(true);
    }
    @Test
    public void test_get_ipv6_ip_segment_list() {
        HandleIpUtils.getIpv6ExpansionIpSegmentList(new ArrayList<>(),new Ipv6Resource(),2,true);
        HandleIpUtils.getIpv6ExpansionIpSegmentList(new ArrayList<>(),new Ipv6Resource(),2,false);
        Assert.assertTrue(true);
    }

    @Test
    public void test_is_net_plane_ipv4_exist() {
        HandleIpUtils.isNetPlaneIpv4Exist(new ArrayList<>(),new Ipv4Resource());
        HandleIpUtils.isNetPlaneIpv4Exist(new ArrayList<Ipv4Resource>(){{
            add(new Ipv4Resource());
        }},new Ipv4Resource());
        Assert.assertTrue(true);
    }
    @Test
    public void test_is_net_plane_ipv6_exist() {
        HandleIpUtils.isNetPlaneIpv6Exist(new ArrayList<>(),new Ipv6Resource());
        HandleIpUtils.isNetPlaneIpv6Exist(new ArrayList<Ipv6Resource>(){{
            add(new Ipv6Resource());
        }},new Ipv6Resource());
        Assert.assertTrue(true);
    }

    @Test
    public void test_compare_ipv6() {
        HandleIpUtils.compareIpv6(new ArrayList<>(),new ArrayList<Ipv6Resource>(){{
            add(new Ipv6Resource());
        }});

        HandleIpUtils.compareIpv6(new ArrayList<Ipv6Resource>(){{
            add(new Ipv6Resource(){{
                setStartIp("2016:8:40:96:c11::90");
                setEndIp("2016:8:40:96:c11::94");
            }});
        }},new ArrayList<Ipv6Resource>(){{
            add(new Ipv6Resource(){{
                setStartIp("2016:8:40:96:c11::92");
                setEndIp("2016:8:40:96:c11::96");
            }});
        }});
        Assert.assertTrue(true);
    }

    @Test
    public void test_is_exist_same_ipv4() {
        HandleIpUtils.isExistSameIpv4Segment(new ArrayList<Ipv4Resource>() {
            {
                add(new Ipv4Resource(){{
                    setStartIp("8.40.102.105");
                    setEndIp("8.40.102.108");
                }});
            }
        }, new ArrayList<Ipv4Resource>() {
            {
                add(new Ipv4Resource(){{
                    setStartIp("8.40.102.105");
                    setEndIp("8.40.102.108");
                }});
            }
        });
        Assert.assertTrue(true);
    }

    @Test
    public void test_is_exist_same_ipv6() {
        HandleIpUtils.isExistSameIpv6Segment(new ArrayList<Ipv6Resource>() {
            {
                add(new Ipv6Resource(){{
                    setStartIp("2016:8:40:96:c11::90");
                    setEndIp("2016:8:40:96:c11::94");
                }});
            }
        }, new ArrayList<Ipv6Resource>() {
            {
                add(new Ipv6Resource(){{
                    setStartIp("2016:8:40:96:c11::90");
                    setEndIp("2016:8:40:96:c11::94");
                }});
            }
        });

        Assert.assertTrue(true);
    }
    }

