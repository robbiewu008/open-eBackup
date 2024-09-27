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
package openbackup.system.base.common.utils.network;

/**
 * Immutable representation of an IPv6 network mask. A network mask is
 * nothing more than an IPv6 address with a continuous range of 1 bits
 * starting from the most significant bit. A network mask can also be
 * represented as a prefix length, which is the count of these 1 bits.
 *
 * @author y00413474
 * @since 2020-07-01
 */
public final class IPv6NetworkMask {
    private final int prefixLength;

    /**
     * Construct an IPv6 network mask from a prefix length. The prefix length should be in the interval ]0, 128].
     *
     * @param prefixLength prefix length
     * @throws IllegalArgumentException if the prefix length is not in the interval ]0, 128]
     */
    IPv6NetworkMask(int prefixLength) {
        if (prefixLength < 0 || prefixLength > 128) {
            throw new IllegalArgumentException("prefix length should be in interval [0, 128]");
        }

        this.prefixLength = prefixLength;
    }

    /**
     * Construct an IPv6 network mask from a prefix length. The prefix length should be in the interval ]0, 128].
     *
     * @param prefixLength prefix length
     * @return ipv6 network mask
     * @throws IllegalArgumentException if the prefix length is not in the interval ]0, 128]
     */
    public static IPv6NetworkMask fromPrefixLength(int prefixLength) {
        return new IPv6NetworkMask(prefixLength);
    }

    /**
     * asPrefixLength
     *
     * @return int
     */
    public int asPrefixLength() {
        return prefixLength;
    }

    /**
     * equals method
     *
     * @param object object
     * @return compare result
     */
    @Override
    public boolean equals(Object object) {
        if (this == object) {
            return true;
        }
        if (!(object instanceof IPv6NetworkMask)) {
            return false;
        }

        IPv6NetworkMask that = (IPv6NetworkMask) object;

        return prefixLength == that.prefixLength;
    }

    /**
     * hash code
     *
     * @return hash code
     */
    @Override
    public int hashCode() {
        return prefixLength;
    }

    /**
     * case prefix length to string
     *
     * @return string
     */
    @Override
    public String toString() {
        return String.valueOf(prefixLength);
    }
}
