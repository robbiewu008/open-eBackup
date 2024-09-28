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
package openbackup.goldendb.protection.access.interceptor;

import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.when;

import openbackup.data.protection.access.provider.sdk.resource.ResourceService;

import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnitRunner;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Optional;

/**
 * 功能描述
 *
 */
@RunWith(MockitoJUnitRunner.class)
public class GoldenDBCopyDeleteInterceptorTest {
    @Mock
    private ResourceService resourceService;

    @Mock
    private CopyRestApi copyRestApi;

    private GoldenDBCopyDeleteInterceptor interceptor;

    @Before
    public void setUp() {
        interceptor = new GoldenDBCopyDeleteInterceptor(copyRestApi, resourceService);
    }

    @Test
    public void test_applicable_success() {
        // Run the test
        boolean result = interceptor.applicable(ResourceSubTypeEnum.GOLDENDB_CLUSETER_INSTANCE.getType());

        // Verify the results
        Assert.assertTrue(result);
    }

    @Test
    public void test_should_supply_agent_success() {
        boolean result = interceptor.shouldSupplyAgent(null, null);
        Assert.assertFalse(result);
    }

    @Test
    public void test_get_copies_copy_type_is_full_success() {
        Copy copy = new Copy();
        copy.setUuid("uuid");
        when(copyRestApi.queryLatestFullBackupCopies(anyString(), anyInt(), anyInt())).thenReturn(Optional.of(copy));

        Copy thisCopy = new Copy();
        thisCopy.setGn(1);
        thisCopy.setResourceId("resourceId");
        List<String> result = interceptor.getCopiesCopyTypeIsFull(null, thisCopy, null);
        Assert.assertEquals(0, result.size());
    }

    @Test
    public void test_get_copies_copy_type_is_full_success2() {
        when(copyRestApi.queryLatestFullBackupCopies(anyString(), anyInt(), anyInt())).thenReturn(Optional.empty());

        Copy thisCopy = new Copy();
        thisCopy.setGn(1);
        thisCopy.setResourceId("resourceId");
        thisCopy.setProperties("{\"format\":1}");

        Copy nexCopy = new Copy();
        nexCopy.setGn(10);

        Copy copy1 = new Copy();
        copy1.setUuid("1");
        copy1.setGn(0);

        Copy copy2 = new Copy();
        copy2.setUuid("2");
        copy2.setGn(2);

        Copy copy3 = new Copy();
        copy3.setUuid("3");
        copy3.setBackupType(2);
        copy3.setGn(3);

        List<String> result = interceptor.getCopiesCopyTypeIsFull(Arrays.asList(copy1, copy2, copy3), thisCopy,
            nexCopy);
        Assert.assertEquals(Arrays.asList("2", "3"), result);
    }

    @Test
    public void test_get_copies_copy_type_is_full_success3() {
        when(copyRestApi.queryLatestFullBackupCopies(anyString(), anyInt(), anyInt())).thenReturn(Optional.empty());

        Copy thisCopy = new Copy();
        thisCopy.setGn(1);
        thisCopy.setResourceId("resourceId");
        thisCopy.setProperties("{\"format\":0}");

        Copy nexCopy = new Copy();
        nexCopy.setGn(10);

        Copy copy1 = new Copy();
        copy1.setUuid("1");
        copy1.setBackupType(4);
        copy1.setGn(0);

        Copy copy2 = new Copy();
        copy2.setUuid("2");
        copy2.setBackupType(4);
        copy2.setGn(2);

        Copy copy3 = new Copy();
        copy3.setUuid("3");
        copy3.setBackupType(2);
        copy3.setGn(3);

        List<String> result = interceptor.getCopiesCopyTypeIsFull(Arrays.asList(copy1, copy2, copy3), thisCopy,
            nexCopy);
        Assert.assertEquals(Collections.singletonList("2"), result);
    }
}
