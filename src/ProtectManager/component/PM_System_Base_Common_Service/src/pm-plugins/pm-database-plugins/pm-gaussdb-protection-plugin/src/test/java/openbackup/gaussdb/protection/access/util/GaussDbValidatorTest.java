/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.gaussdb.protection.access.util;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;

import openbackup.system.base.common.exception.LegoCheckedException;

import org.junit.Assert;
import org.junit.Rule;
import org.junit.jupiter.api.Test;
import org.junit.rules.ExpectedException;

import java.util.ArrayList;
import java.util.List;

/**
 * 功能描述
 *
 * @author t30021437
 * @since 2023-02-07
 */
public class GaussDbValidatorTest {
    @Rule
    private ExpectedException expectedException = ExpectedException.none();

    @Test
    public void check_gauss_db_count() {
        List<ProtectedEnvironment> existingEnvironments = new ArrayList<>();
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        for (int i = 0; i < 9; i++) {
            existingEnvironments.add(protectedEnvironment);
        }
        Assert.assertThrows(LegoCheckedException.class, () -> GaussDBValidator.checkGaussDbCount(existingEnvironments));
    }
}