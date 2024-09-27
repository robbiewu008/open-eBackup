/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.common.utils.network;

import static openbackup.system.base.common.utils.network.BitSetHelpers.bitSetOf;

import java.util.BitSet;

/**
 * 功能描述
 *
 * @author y00413474
 * @since 2020-07-01
 */
public class IPv6NetworkHelpers {
    static int longestPrefixLength(IPv6Address first, IPv6Address last) {
        final BitSet firstBits = bitSetOf(first.getLowBits(), first.getHighBits());
        final BitSet lastBits = bitSetOf(last.getLowBits(), last.getHighBits());

        return countLeadingSimilarBits(firstBits, lastBits);
    }

    private static int countLeadingSimilarBits(BitSet first, BitSet last) {
        int result = 0;
        for (int i = 127; i >= 0 && (first.get(i) == last.get(i)); i--) {
            result++;
        }

        return result;
    }
}