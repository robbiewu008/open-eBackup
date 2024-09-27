/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.livemount.convert;

import openbackup.data.access.framework.livemount.converter.PolicyDataConverter;
import openbackup.data.access.framework.livemount.dao.LiveMountPolicyEntityDao;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.ContextConfiguration;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.ArrayList;
import java.util.Collection;

/**
 * test PolicyData Converter
 *
 * @author fwx1022842
 * @since 2021-3-8
 */
@RunWith(SpringRunner.class)
@SpringBootTest
@ContextConfiguration(classes = {PolicyDataConverter.class})
public class PolicyDataConverterTest {
    @MockBean
    private LiveMountPolicyEntityDao liveMountPolicyEntityDao;

    @Autowired
    private PolicyDataConverter policyDataConverter;

    /**
     * get name
     */
    @Test
    public void getName() {
        String name = policyDataConverter.getName();
        assert "live_mount_policy".equals(name);
    }

    /**
     * convert
     */
    @Test
    public void convert() {
        Collection<String> data = new ArrayList<>();
        data.add("1");
        data.add("2");
        Collection<?> convert = policyDataConverter.convert(data);
        Assert.assertEquals(convert.size(), 0);
    }
}
