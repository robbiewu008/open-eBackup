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

import java.math.BigInteger;
import java.util.Arrays;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.NoSuchElementException;

/**
 * IPv6AddressRange
 *
 */
public class IPv6AddressRange implements Comparable<IPv6AddressRange>, Iterable<IPv6Address> {
    private final IPv6Address first;

    private final IPv6Address last;

    IPv6AddressRange(IPv6Address first, IPv6Address last) {
        if (first.compareTo(last) > 0) {
            throw new IllegalArgumentException("Cannot create ip address range with last address < first address");
        }

        this.first = first;
        this.last = last;
    }

    /**
     * fromFirstAndLast
     *
     * @param first first
     * @param last last
     * @return IPv6AddressRange
     */
    public static IPv6AddressRange fromFirstAndLast(IPv6Address first, IPv6Address last) {
        return new IPv6AddressRange(first, last);
    }

    /**
     * contains
     *
     * @param address address
     * @return boolean
     */
    public boolean contains(IPv6Address address) {
        return first.compareTo(address) <= 0 && last.compareTo(address) >= 0;
    }

    /**
     * contains
     *
     * @param range range
     * @return boolean
     */
    public boolean contains(IPv6AddressRange range) {
        return contains(range.first) && contains(range.last);
    }

    /**
     * overlaps
     *
     * @param range range
     * @return boolean
     */
    public boolean overlaps(IPv6AddressRange range) {
        return contains(range.first) || contains(range.last) || range.contains(first) || range.contains(last);
    }

    /**
     * return an iterator which iterates all addresses in this range, in order.
     *
     * @return Iterator<IPv6Address>
     */
    @Override
    public Iterator<IPv6Address> iterator() {
        return new IPv6AddressRangeIterator();
    }

    /**
     * return number of addresses in the range
     *
     * @return BigInteger
     */
    public BigInteger size() {
        BigInteger firstAsBigInteger = new BigInteger(1, first.toByteArray());
        BigInteger lastAsBigInteger = new BigInteger(1, last.toByteArray());

        // note that first and last are included in the range.
        return lastAsBigInteger.subtract(firstAsBigInteger).add(BigInteger.ONE);
    }

    /**
     * Deaggregate a range of IPv6 addresses (which is not necessarily aligned
     * with a single IPv6 network) into a minimal set of non
     * overlapping consecutive subnets.
     *
     * @return iterator of IPv6 networks that all together define the minimal
     * set of subnets by which the range can be represented.
     */
    public Iterator<IPv6Network> toSubnets() {
        return new IPv6AddressRangeAsSubnetsIterator();
    }

    /**
     * Remove an address from the range, resulting in one, none or two new ranges.
     * If an address outside the range is removed, this has no
     * effect. If the first or last address is removed, a single new range is returned
     * (potentially empty if the range only contained a
     * single address). If an address somewhere else in the range is removed,
     * two new ranges are returned.
     *
     * @param address adddress to remove from the range
     * @return list of resulting ranges
     */
    public List<IPv6AddressRange> remove(IPv6Address address) {
        if (address == null) {
            throw new IllegalArgumentException("invalid address [null]");
        }

        if (!contains(address)) {
            return Collections.singletonList(this);
        } else if (address.equals(first) && address.equals(last)) {
            return Collections.emptyList();
        } else if (address.equals(first)) {
            return Collections.singletonList(fromFirstAndLast(first.add(1), last));
        } else if (address.equals(last)) {
            return Collections.singletonList(fromFirstAndLast(first, last.subtract(1)));
        } else {
            return Arrays.asList(fromFirstAndLast(first, address.subtract(1)), fromFirstAndLast(address.add(1), last));
        }
    }

    /**
     * Extend the range just enough at its head or tail such that the given address is included.
     *
     * @param address address to extend the range to
     * @return new (bigger) range
     */
    public IPv6AddressRange extend(IPv6Address address) {
        if (address.compareTo(first) < 0) {
            return fromFirstAndLast(address, last);
        } else if (address.compareTo(last) > 0) {
            return fromFirstAndLast(first, address);
        } else {
            return this;
        }
    }

    /**
     * Remove a network from the range, resulting in one, none or two new ranges.
     * If a network outside (or partially outside) the range is
     * removed, this has no effect. If the network which is removed is aligned with
     * the beginning or end of the range, a single new ranges
     * is returned (potentially empty if the range was equal to the network which is
     * removed from it). If a network somewhere else in the
     * range is removed, two new ranges are returned.
     *
     * @param network network to remove from the range
     * @return list of resulting ranges
     */
    public List<IPv6AddressRange> remove(IPv6Network network) {
        if (network == null) {
            throw new IllegalArgumentException("invalid network [null]");
        }
        if (!contains(network)) {
            return Collections.singletonList(this);
        } else if (this.equals(network)) {
            return Collections.emptyList();
        } else if (first.equals(network.getFirst())) {
            return Collections.singletonList(fromFirstAndLast(network.getLast().add(1), last));
        } else if (last.equals(network.getLast())) {
            return Collections.singletonList(fromFirstAndLast(first, network.getFirst().subtract(1)));
        } else {
            return Arrays.asList(fromFirstAndLast(first, network.getFirst().subtract(1)),
                fromFirstAndLast(network.getLast().add(1), last));
        }
    }

    /**
     * toString
     *
     * @return String
     */
    @Override
    public String toString() {
        return first.toString() + " - " + last.toString();
    }

    /**
     * return like <code>toString</code> but without using shorthand notations for addresses
     *
     * @return String
     */
    public String toLongString() {
        return first.toLongString() + " - " + last.toLongString();
    }

    /**
     * The natural  orders them on increasing first addresses, and on increasing last
     * address if the first address would be equal.
     * Note that the natural order does thus not compare sizes of ranges.
     *
     * @param that that
     * @return int
     */
    @Override
    public int compareTo(IPv6AddressRange that) {
        if (!this.first.equals(that.first)) {
            return this.first.compareTo(that.first);
        } else {
            return this.last.compareTo(that.last);
        }
    }

    /**
     * equals
     *
     * @param object object
     * @return boolean
     */
    @Override
    public boolean equals(Object object) {
        if (this == object) {
            return true;
        }
        if (!(object instanceof IPv6AddressRange)) {
            return false;
        }

        IPv6AddressRange that = (IPv6AddressRange) object;

        if (first != null ? !first.equals(that.first) : that.first != null) {
            return false;
        }
        return last != null ? last.equals(that.last) : that.last == null;
    }

    @Override
    public int hashCode() {
        int result = first != null ? first.hashCode() : 0;
        return 31 * result + (last != null ? last.hashCode() : 0);
    }

    public IPv6Address getFirst() {
        return first;
    }

    public IPv6Address getLast() {
        return last;
    }

    /**
     * @see IPv6AddressRange#iterator()
     */
    private final class IPv6AddressRangeIterator implements Iterator<IPv6Address> {
        private IPv6Address current = first;

        @Override
        public boolean hasNext() {
            return current.compareTo(last) <= 0;
        }

        @Override
        public IPv6Address next() {
            if (hasNext()) {
                IPv6Address result = current;
                current = current.add(1);
                return result;
            } else {
                throw new NoSuchElementException();
            }
        }

        @Override
        public void remove() {
            throw new UnsupportedOperationException("This iterator provides read only access");
        }
    }

    private class IPv6AddressRangeAsSubnetsIterator implements Iterator<IPv6Network> {
        private IPv6Address base = first;

        @Override
        public IPv6Network next() {
            int step;

            IPv6Network next;
            if (hasNext()) {
                step = 0;

                // try setting the step-th bit until we reach a bit that is already set
                while (step < 128 && !(base.setBit(step)).equals(base)) {
                    // if the max address in this subnet is beyond the end of the range, we went too far
                    if ((
                        base.maximumAddressWithNetworkMask(IPv6NetworkMask.fromPrefixLength(127 - step)).compareTo(last)
                            > 0)) {
                        break;
                    }
                    step++;
                }

                // the next subnet is found
                next = IPv6Network.fromAddressAndMask(base, IPv6NetworkMask.fromPrefixLength(128 - step));

                // start the next loop after the end of the subnet just found
                if (next.getLast().compareTo(last) < 0) {
                    base = next.getLast().add(1);
                } else {
                    base = null; // to signal we reached the end
                }
            } else {
                throw new NoSuchElementException();
            }

            return next;
        }

        @Override
        public boolean hasNext() {
            // there is a next subnet as long as we didn't reach the end of the range
            return base != null && (base.compareTo(last) <= 0);
        }

        @Override
        public void remove() {
            throw new UnsupportedOperationException("This iterator provides read only access");
        }
    }
}
