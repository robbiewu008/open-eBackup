/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 */

package openbackup.system.base.service.email;

import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.utils.NumberUtil;

import java.util.Comparator;

/**
 * 安全协议排序比较器
 *
 * @author l90005176
 * @version [OceanStor BCManager V200R001C00, 2016年2月16日]
 * @since 2018-01-01
 */
public class ProtocolComparator implements Comparator<String> {
    private static final String PROTOCOL_TLS = "TLS";

    private static final String PROTOCOL_SSL = "SSL";

    private static final String PROTOCOL_SPLITTER = "v";

    /**
     * 对协议按照类型和版本进行比较
     *
     * @param protocol1 协议1
     * @param protocol2 协议2
     * @return 根据第一个参数小于、等于或大于第二个参数分别返回负整数、零或正整数。
     */
    @Override
    public int compare(String protocol1, String protocol2) {
        if (!protocol1.startsWith(PROTOCOL_TLS) && !protocol1.startsWith(PROTOCOL_SSL)) {
            return IsmNumberConstant.NEGATIVE_ONE;
        }

        String[] protocol1Array = protocol1.split(PROTOCOL_SPLITTER);
        String[] protocol2Array = protocol2.split(PROTOCOL_SPLITTER);

        String protocol1Type = protocol1Array[LegoNumberConstant.ZERO];
        double protocol1Version = (protocol1Array.length == LegoNumberConstant.ONE)
            ? 0.0
            : NumberUtil.parseDouble(protocol1Array[1]);

        String protocol2Type = protocol2Array[LegoNumberConstant.ZERO];
        double protocol2Version = (protocol2Array.length == LegoNumberConstant.ONE)
            ? 0.0
            : NumberUtil.parseDouble(protocol2Array[1]);

        int result = compareByProtocolType(protocol1Type, protocol2Type);
        if (result != LegoNumberConstant.ZERO) {
            return result;
        }

        return compareByProtocolVersion(protocol1Version, protocol2Version);
    }

    /**
     * 按协议类型进行比较
     *
     * @param protocol1Type 协议1的类型
     * @param protocol2Type 协议2的类型
     * @return int 根据第一个参数小于、等于或大于第二个参数分别返回负整数、零或正整数。
     */
    private int compareByProtocolType(String protocol1Type, String protocol2Type) {
        if (protocol1Type.equalsIgnoreCase(PROTOCOL_TLS) && protocol2Type.equalsIgnoreCase(PROTOCOL_SSL)) {
            return LegoNumberConstant.ONE;
        }

        if (protocol1Type.equalsIgnoreCase(PROTOCOL_SSL) && protocol2Type.equalsIgnoreCase(PROTOCOL_TLS)) {
            return IsmNumberConstant.NEGATIVE_ONE;
        }

        return LegoNumberConstant.ZERO;
    }

    /**
     * 按协议版本进行比较
     *
     * @param protocol1Version 协议1的版本
     * @param protocol2Version 协议2的版本
     * @return int 根据第一个参数小于、等于或大于第二个参数分别返回负整数、零或正整数。
     */
    private int compareByProtocolVersion(double protocol1Version, double protocol2Version) {
        return Double.compare(protocol2Version, protocol1Version);
    }
}
