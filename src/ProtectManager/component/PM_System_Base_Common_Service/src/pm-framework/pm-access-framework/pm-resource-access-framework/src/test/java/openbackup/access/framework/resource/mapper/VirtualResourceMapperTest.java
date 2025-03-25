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
package openbackup.access.framework.resource.mapper;

import com.baomidou.mybatisplus.autoconfigure.MybatisPlusAutoConfiguration;

import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.persistence.dao.VirtualResourceMapper;
import openbackup.access.framework.resource.persistence.model.VirtualResourceExtendPo;
import openbackup.access.framework.resource.persistence.model.VirtualResourceResponsePo;

import org.junit.Assert;
import org.junit.FixMethodOrder;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.MethodSorters;
import org.mybatis.spring.annotation.MapperScan;
import org.springframework.boot.SpringBootConfiguration;
import org.springframework.boot.autoconfigure.jdbc.DataSourceAutoConfiguration;
import org.springframework.boot.autoconfigure.sql.init.SqlInitializationAutoConfiguration;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.List;

import javax.annotation.Resource;

/**
 * 虚拟机mapper
 *
 */
@SpringBootConfiguration
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {
    VirtualResourceMapper.class, DataSourceAutoConfiguration.class, SqlInitializationAutoConfiguration.class,
    MybatisPlusAutoConfiguration.class
})
@MapperScan(basePackageClasses = VirtualResourceMapper.class)
@Slf4j
@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class VirtualResourceMapperTest {
    @Resource
    private VirtualResourceMapper virtualResourceMapper;

    /**
     * 用例场景：查询虚拟机资源
     * 前置条件：预置数据 init_data.sql
     * 检查点：sql正确
     */
    @Test
    public void pageListSlaIdWithResourceCounts() {
        List<VirtualResourceResponsePo> virtualResourceResponsePos = virtualResourceMapper.queryVirtualResource("1");
        List<VirtualResourceExtendPo> resourceExtendPos = virtualResourceMapper.listAll("", "root_uuid");
        Assert.assertEquals(virtualResourceResponsePos.size(), 0);
        Assert.assertEquals(resourceExtendPos.size(), 0);
    }
}
