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
package openbackup.data.access.framework.core.common.enums;

import openbackup.data.access.framework.core.common.enums.AgentHostOsTypeEnum;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.DataMoverCheckedException;

import org.junit.Assert;
import org.junit.Test;

/**
 * 功能描述
 *
 */
public class AgentHostOsTypeEnumTest {
    @Test
    public void get_osTypeThin_success() {
        Assert.assertEquals(AgentHostOsTypeEnum.OEL.getOsTypeThin(), "OEL");
        Assert.assertEquals(AgentHostOsTypeEnum.KYLIN.getOsTypeThin(), "Kylin");
        Assert.assertEquals(AgentHostOsTypeEnum.NEOKYLIN.getOsTypeThin(), "NeoKylin");
        Assert.assertEquals(AgentHostOsTypeEnum.CentOS.getOsTypeThin(), "CentOS");
        Assert.assertEquals(AgentHostOsTypeEnum.SUSE.getOsTypeThin(), "SUSE");
        Assert.assertEquals(AgentHostOsTypeEnum.ISOFT.getOsTypeThin(), "ISOFT");
        Assert.assertEquals(AgentHostOsTypeEnum.REDHAT.getOsTypeThin(), "RedHat");
        Assert.assertEquals(AgentHostOsTypeEnum.OPENEULER.getOsTypeThin(), "openEuler");
        Assert.assertEquals(AgentHostOsTypeEnum.ROCKY.getOsTypeThin(), "ROCKY");
        Assert.assertEquals(AgentHostOsTypeEnum.UNIONTECH.getOsTypeThin(), "UnionTech OS Server");
        Assert.assertEquals(AgentHostOsTypeEnum.UBUNTU.getOsTypeThin(), "Ubuntu");
        Assert.assertEquals(AgentHostOsTypeEnum.DEBIAN.getOsTypeThin(), "Debian");
        Assert.assertEquals(AgentHostOsTypeEnum.HPUX.getOsTypeThin(), "HPUX IA");
        Assert.assertEquals(AgentHostOsTypeEnum.AIX.getOsTypeThin(), "AIX");
        Assert.assertEquals(AgentHostOsTypeEnum.SOLARIS.getOsTypeThin(), "SOLARIS");
    }

    @Test
    public void get_osType_success() {
        Assert.assertEquals(AgentHostOsTypeEnum.OEL.getOsType(), "Linux");
        Assert.assertEquals(AgentHostOsTypeEnum.KYLIN.getOsType(), "Linux");
        Assert.assertEquals(AgentHostOsTypeEnum.NEOKYLIN.getOsType(), "Linux");
        Assert.assertEquals(AgentHostOsTypeEnum.CentOS.getOsType(), "Linux");
        Assert.assertEquals(AgentHostOsTypeEnum.SUSE.getOsType(), "Linux");
        Assert.assertEquals(AgentHostOsTypeEnum.ISOFT.getOsType(), "Linux");
        Assert.assertEquals(AgentHostOsTypeEnum.REDHAT.getOsType(), "Linux");
        Assert.assertEquals(AgentHostOsTypeEnum.OPENEULER.getOsType(), "Linux");
        Assert.assertEquals(AgentHostOsTypeEnum.ROCKY.getOsType(), "Linux");
        Assert.assertEquals(AgentHostOsTypeEnum.UNIONTECH.getOsType(), "Linux");
        Assert.assertEquals(AgentHostOsTypeEnum.UBUNTU.getOsType(), "Linux");
        Assert.assertEquals(AgentHostOsTypeEnum.DEBIAN.getOsType(), "Linux");
        Assert.assertEquals(AgentHostOsTypeEnum.HPUX.getOsType(), "HP-UX");
        Assert.assertEquals(AgentHostOsTypeEnum.AIX.getOsType(), "AIX");
        Assert.assertEquals(AgentHostOsTypeEnum.SOLARIS.getOsType(), "Solaris");
    }

    @Test
    public void test_getOsTypeByThin_succes() {
        String osTypeByThin = AgentHostOsTypeEnum.getOsTypeByThin("OEL");
        Assert.assertEquals("Linux", osTypeByThin);

        try {
            AgentHostOsTypeEnum.getOsTypeByThin("errType");
            Assert.fail();
        } catch (DataMoverCheckedException e) {
            Assert.assertEquals(CommonErrorCode.ERR_PARAM, e.getErrorCode());
        }
    }
}
