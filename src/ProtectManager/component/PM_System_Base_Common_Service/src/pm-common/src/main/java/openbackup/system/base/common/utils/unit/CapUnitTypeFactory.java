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
package openbackup.system.base.common.utils.unit;

import java.util.HashMap;
import java.util.Map;

/**
 * 容量单位类型工厂类
 *
 */
public class CapUnitTypeFactory {
    private static final CapUnitTypeFactory INSTANCE = new CapUnitTypeFactory();

    private final Map<CapabilityUnitType, AbstractCapUnitType> capUnitTypeMap = new HashMap<>();

    private CapUnitTypeFactory() {
        capUnitTypeMap.put(CapabilityUnitType.GB, new GBCapUnitType());
        capUnitTypeMap.put(CapabilityUnitType.MB, new MBCapUnitType());
        capUnitTypeMap.put(CapabilityUnitType.TB, new TBCapUnitType());
        capUnitTypeMap.put(CapabilityUnitType.PB, new PBCapUnitType());
        capUnitTypeMap.put(CapabilityUnitType.KB, new KBCapUnitType());
        capUnitTypeMap.put(CapabilityUnitType.BYTE, new ByteCapUnitType());
    }

    /**
     * 获取容量单位类型工厂实例
     *
     * @return 返回容量单位类型工厂实例
     */
    public static synchronized CapUnitTypeFactory getInstance() {
        return INSTANCE;
    }

    /**
     * 创建指定容量单位的类型实例
     *
     * @param unitType 单位类型
     * @return 返回指定容量单位的类型实例
     */
    public AbstractCapUnitType createCapUnitType(CapabilityUnitType unitType) {
        return capUnitTypeMap.get(unitType);
    }
}
