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
package openbackup.system.base.sdk.resource.enums;

import java.util.Arrays;
import java.util.List;

/**
 * 操作系统类型枚举
 *
 */
public enum LinuxOsTypeEnum {
    /**
     * linux版本RedHat
     */
    REDHAT("RedHat"),
    /**
     * linux版本SUSE
     */

    SUSE("SUSE"),
    /**
     * linux版本ROCKY
     */
    ROCKY("ROCKY"),
    /**
     * linux版本OEL
     */
    OEL("OEL"),
    /**
     * linux版本ISOFT
     */
    ISOFT("ISOFT"),
    /**
     * linux版本CentOS
     */
    CENTOS("CentOS"),
    /**
     * linux版本Kylin
     */
    KYLIN("Kylin"),
    /**
     * linux版本NeoKylin
     */
    NEO_KYLIN("NeoKylin"),
    /**
     * linux版本UnionTech OS
     */
    UNION_TECH_OS("UnionTech OS"),
    /**
     * linux版本openEuler
     */
    OPEN_EULER("openEuler"),
    /**
     * linux版本Debian
     */
    DEBIAN("Debian"),
    /**
     * linux版本SOLARIS
     */
    SOLARIS("SOLARIS"),
    /**
     * linux版本HPUXIA
     */
    HPUXIA("HPUXIA"),
    /**
     * linux版本Ubuntu
     */
    UBUNTU("Ubuntu"),

    /**
     * linux版本Asianux
     */
    ASIANUX("Asianux"),

    /**
     * linux版本NFSChina
     */
    NFS_CHINA("NFSChina"),

    /**
     * linux版本KylinSec
     */
    KYLINSED("KylinSec"),

    /**
     * linux版本ANOLIS
     */
    ANOLIS("ANOLIS"),

    /**
     * linux版本ANOLIS
     */
    CEOS("CEOS");

    private final String name;

    LinuxOsTypeEnum(String name) {
        this.name = name;
    }

    private static final List<LinuxOsTypeEnum> LINUX_OS_TYPES = Arrays.asList(values());

    /**
     * 判断传入的操作系统类型是否属于Linux大类
     *
     * @param type 传入的操作系统类型
     * @return 是否属于Linux大类
     */
    public static boolean isLinuxType(String type) {
        return LINUX_OS_TYPES.stream().anyMatch(osType -> osType.name.equals(type));
    }

    /**
     * 获取os类型名
     *
     * @return 类型名
     */
    public String getName() {
        return name;
    }
}
