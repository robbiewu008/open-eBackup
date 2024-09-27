/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.common.utils;

import openbackup.system.base.common.constants.LegoNumberConstant;

import java.util.UUID;

/**
 * UUIDGenerator
 *
 * @author p00171530
 * @version [Lego V100R002C10, 2014-12-18]
 * @since 2019-11-01
 */
public class UUIDGenerator {
    private UUIDGenerator() {
    }

    /**
     * 获得一个UUID
     *
     * @return String UUID
     */
    public static synchronized String getUUID() {
        String vars = UUID.randomUUID().toString();

        // 去掉“-”符号
        StringBuffer buf = new StringBuffer();
        buf.append(vars, 0, LegoNumberConstant.EIGHT);
        buf.append(vars, LegoNumberConstant.NINE, LegoNumberConstant.THIRTEEN);
        buf.append(vars, LegoNumberConstant.FOURTEEN, LegoNumberConstant.EIGHTEEN);
        buf.append(vars, LegoNumberConstant.NINETEEN, LegoNumberConstant.TWENTY_THREE);
        buf.append(vars.substring(LegoNumberConstant.TWENTY_FOUR));
        return buf.toString();
    }

    /**
     * 获得指定数目的UUID
     *
     * @param number int 需要获得的UUID数量
     * @return String[] UUID数组
     */
    public static String[] getUUID(int number) {
        if (number < 1) {
            return new String[0];
        }
        String[] ss = new String[number];
        for (int i = 0; i < number; i++) {
            ss[i] = getUUID();
        }
        return ss;
    }
}
