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
package openbackup.system.base.common.enums;

import lombok.Getter;

import org.apache.commons.lang3.StringUtils;
import org.apache.sshd.common.NamedFactory;
import org.apache.sshd.common.mac.BuiltinMacs;
import org.apache.sshd.common.mac.Mac;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;

/**
 * ssh mac 签名算法
 *
 */
public enum SshMacs {
    /**
     * 安全算法
     */
    SAFE("safe", "hmac-sha2-256", Collections.singletonList(BuiltinMacs.hmacsha256)),

    /**
     * 兼容性算法
     */
    COMPATIBLE("compatible", "hmac-sha2-256,hmac-sha1", Arrays.asList(BuiltinMacs.hmacsha1, BuiltinMacs.hmacsha256));

    @Getter
    private final String name;

    @Getter
    private final String macValue;

    @Getter
    private final List<NamedFactory<Mac>> macTypeList;

    SshMacs(String name, String macValue, List<NamedFactory<Mac>> macTypeList) {
        this.name = name;
        this.macValue = macValue;
        this.macTypeList = macTypeList;
    }

    /**
     * 根据签名算法名称获取具体的签名算法
     *
     * @param macName 签名算法名称
     * @return os枚举
     */
    public static String getOsTypeByOsName(String macName) {
        SshMacs sshMacs = Arrays.stream(SshMacs.values())
            .filter(os -> StringUtils.equals(os.getName(), macName))
            .findFirst()
            .orElseThrow(IllegalArgumentException::new);

        return sshMacs.getMacValue();
    }

    /**
     * 根据签名算法指定安全类型获取具体的签名算法列表
     * 注意此处假定传入的name是正确的
     * 如果发现无法匹配则会返回空值 需要在调用处前后进行校验
     *
     * @param name 签名算法安全类型
     * @return 签名算法列表
     */
    public static List<NamedFactory<Mac>> getMacTypeListByName(String name) {
        for (SshMacs macType : SshMacs.values()) {
            if (macType.getName().equalsIgnoreCase(name)) {
                return macType.getMacTypeList();
            }
        }
        // 如果找不到，返回空列表
        return Collections.emptyList();
    }
}
