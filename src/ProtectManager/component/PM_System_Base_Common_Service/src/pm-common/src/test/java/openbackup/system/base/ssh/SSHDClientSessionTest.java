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
package openbackup.system.base.ssh;

import com.google.common.collect.Sets;

import openbackup.system.base.ssh.SSHDClientSession;
import openbackup.system.base.ssh.SshUser;

import org.apache.sshd.sftp.common.SftpException;
import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.io.IOException;
import java.nio.file.attribute.PosixFilePermission;
import java.util.HashSet;

import javax.naming.AuthenticationException;

/**
 * 功能描述
 *
 * @author w30042425
 * @since 2023-10-20
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest( {SSHDClientSession.class})
public class SSHDClientSessionTest {

    private SSHDClientSession instance;

    @Before
    public void test_get_instance_success() throws AuthenticationException, IOException {
        SshUser sshUser = SshUser.builder()
            .port(22)
            .sftpPort(22)
            .username("root")
            .password("Admin@storage1")
            .isSuperUser(true)
            .ip("8.40.147.80")
            .build();
        instance = SSHDClientSession.getInstance(sshUser);
    }

    @Test
    public void test_exec_success() throws IOException {
        instance.exec("ifconfig");
    }

    @Test
    public void test_upload_file_error() throws SftpException {
        HashSet<PosixFilePermission> permissions = Sets.newHashSet(PosixFilePermission.OWNER_EXECUTE,
            PosixFilePermission.OWNER_EXECUTE);
        Assert.assertThrows(IOException.class, () -> {
            instance.uploadFile("/test/test.txt", "/opt/test.txt", permissions, permissions);
        });
    }

    @Test
    public void test_mkdir_success() throws SftpException {
        HashSet<PosixFilePermission> permissions = Sets.newHashSet(PosixFilePermission.OWNER_EXECUTE,
            PosixFilePermission.OWNER_WRITE,PosixFilePermission.OWNER_READ);
        instance.mkdir("/opt/backup3/127_1", permissions);
    }

    @After
    public void test_close() {
        instance.close();
    }
}
