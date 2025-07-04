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

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.DataMoverCheckedException;

/**
 * 功能描述
 *
 */
public enum AgentHostOsTypeEnum {
    /**
     * Linux_OEL
     */
    OEL("OEL", Constants.LINUX),

    /**
     * Linux_Kylin
     */
    KYLIN("Kylin", Constants.LINUX),

    /**
     * Linux_NeoKylin
     */
    NEOKYLIN("NeoKylin", Constants.LINUX),

    /**
     * Linux_CentOS
     */
    CentOS("CentOS", Constants.LINUX),

    /**
     * Linux_SUSE
     */
    SUSE("SUSE", Constants.LINUX),

    /**
     * Linux_ISOFT
     */
    ISOFT("ISOFT", Constants.LINUX),

    /**
     * Linux_RedHat
     */
    REDHAT("RedHat", Constants.LINUX),

    /**
     * Linux_openEuler
     */
    OPENEULER("openEuler", Constants.LINUX),

    /**
     * Linux_ROCKY
     */
    ROCKY("ROCKY", Constants.LINUX),

    /**
     * Linux_UnionTech
     */
    UNIONTECH("UnionTech OS Server", Constants.LINUX),

    /**
     * Linux_Ubuntu
     */
    UBUNTU("Ubuntu", Constants.LINUX),

    /**
     * Linux_Debian
     */
    DEBIAN("Debian", Constants.LINUX),

    /**
     * Linux_HPUX
     */
    HPUX("HPUX IA", "HP-UX"),

    /**
     * Linux_AIX
     */
    AIX("AIX", "AIX"),

    /**
     * Linux_SOLARIS
     */
    SOLARIS("SOLARIS", "Solaris");

    /**
     * 细粒度操作系统
     */
    private final String osTypeThin;

    private final String osType;

    AgentHostOsTypeEnum(String osTypeThin, String osType) {
        this.osTypeThin = osTypeThin;
        this.osType = osType;
    }

    /**
     * agent 操作系统 细分
     *
     * @return code
     */
    public String getOsTypeThin() {
        return osTypeThin;
    }

    /**
     * 操作系统
     *
     * @return message
     */
    public String getOsType() {
        return osType;
    }

    /**
     * 通过string获取 宽泛操作系统
     *
     * @param osTypeThin 操作系统
     * @return String 宽泛操作系统
     */
    public static String getOsTypeByThin(String osTypeThin) {
        for (AgentHostOsTypeEnum agentHostOsTypeEnum : AgentHostOsTypeEnum.values()) {
            if (agentHostOsTypeEnum.osTypeThin.equals(osTypeThin)) {
                return agentHostOsTypeEnum.osType;
            }
        }
        throw new DataMoverCheckedException("no matched AgentHostOsType found", CommonErrorCode.ERR_PARAM);
    }

    /**
     * 内部常量类
     */
    private static class Constants {
        private static final String LINUX = "Linux";
    }
}
