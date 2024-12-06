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
package com.huawei.emeistor.console.service.impl;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.doNothing;

import lombok.SneakyThrows;

import org.bouncycastle.openssl.jcajce.JcaPEMWriter;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.io.BufferedReader;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.nio.file.Files;

/**
 * rsa test
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({RsaServiceImpl.class, Files.class, FileWriter.class, JcaPEMWriter.class})
public class RsaServiceImplTest {
    @Mock
    FileOutputStream fileOutputStream;

    @Mock
    JcaPEMWriter jcaPEMWriter;

    @Mock
    FileWriter fileWriter;

    @Mock
    FileReader fileReader;

    @Mock
    BufferedReader reader;

    @InjectMocks
    RsaServiceImpl rsaService;

    @Before
    @SneakyThrows
    public void initial() {

    }

    @Test
    @SneakyThrows
    public void test_run_rsa_generate_success()  {
    }
}
