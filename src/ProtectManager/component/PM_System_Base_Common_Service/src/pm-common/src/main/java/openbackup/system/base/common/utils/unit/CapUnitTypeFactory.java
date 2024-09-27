/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.common.utils.unit;

import java.util.HashMap;
import java.util.Map;

/**
 * 容量单位类型工厂类
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-12-30
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
