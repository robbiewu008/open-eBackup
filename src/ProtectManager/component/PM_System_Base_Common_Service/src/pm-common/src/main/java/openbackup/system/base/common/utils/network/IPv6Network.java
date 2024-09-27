/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.common.utils.network;

import java.math.BigInteger;
import java.util.Iterator;
import java.util.Locale;
import java.util.NoSuchElementException;

/**
 * Immutable representation of an IPv6 network based on an address and a prefix length.
 * An IPv6 network is also an IPv6 address range (but
 * not all ranges are valid networks).
 *
 * @author y00413474
 * @since 2020-07-01
 */
public final class IPv6Network extends IPv6AddressRange {
    /**
     * MULTICAST_NETWORK
     */
    public static final IPv6Network MULTICAST_NETWORK = fromString("ff00::/8");

    /**
     * SITE_LOCAL_NETWORK
     */
    public static final IPv6Network SITE_LOCAL_NETWORK = fromString("fec0::/48");

    /**
     * LINK_LOCAL_NETWORK
     */
    public static final IPv6Network LINK_LOCAL_NETWORK = fromString("fe80::/64");

    private final IPv6Address address;

    private final IPv6NetworkMask networkMask;

    /**
     * Construct from address and network mask.
     *
     * @param address     address
     * @param networkMask network mask
     */
    private IPv6Network(IPv6Address address, IPv6NetworkMask networkMask) {
        super(address.maskWithNetworkMask(networkMask), address.maximumAddressWithNetworkMask(networkMask));

        this.address = address.maskWithNetworkMask(networkMask);
        this.networkMask = networkMask;
    }

    /**
     * Create an IPv6 network from an IPv6Address and an IPv6NetworkMask
     *
     * @param address     IPv6 address (the network address or any other address within the network)
     * @param networkMask IPv6 network mask
     * @return IPv6 network
     */
    public static IPv6Network fromAddressAndMask(IPv6Address address, IPv6NetworkMask networkMask) {
        return new IPv6Network(address, networkMask);
    }

    /**
     * Create an IPv6 network from the two addresses within the network.
     * This will construct the smallest possible network ("longest prefix
     * length") which contains both addresses.
     *
     * @param one address one
     * @param two address two, should be bigger than address one
     * @return ipv6 network
     */
    public static IPv6Network fromTwoAddresses(IPv6Address one, IPv6Address two) {
        final IPv6NetworkMask longestPrefixLength = IPv6NetworkMask.fromPrefixLength(
            IPv6NetworkHelpers.longestPrefixLength(one, two));
        return new IPv6Network(one.maskWithNetworkMask(longestPrefixLength), longestPrefixLength);
    }

    /**
     * Create an IPv6 network from its String representation.
     * For example "1234:5678:abcd:0:0:0:0:0/64" or "2001::ff/128".
     *
     * @param string string representation
     * @return ipv6 network
     */
    public static IPv6Network fromString(String string) {
        if (string.indexOf('/') == -1) {
            throw new IllegalArgumentException("Expected format is network-address/prefix-length");
        }

        final String networkAddressString = parseNetworkAddress(string);
        int prefixLength = parsePrefixLength(string);

        final IPv6Address networkAddress = IPv6Address.fromString(networkAddressString);

        return fromAddressAndMask(networkAddress, new IPv6NetworkMask(prefixLength));
    }

    private static String parseNetworkAddress(String string) {
        return string.substring(0, string.indexOf('/'));
    }

    private static int parsePrefixLength(String string) {
        try {
            return Integer.parseInt(string.substring(string.indexOf('/') + 1));
        } catch (NumberFormatException e) {
            throw new IllegalArgumentException("Prefix length should be a positive integer");
        }
    }

    /**
     * Split a network in smaller subnets of a given size.
     *
     * @param size size
     * @return iterator of the split subnets.
     * @throws IllegalArgumentException if the requested size is bigger than the original size
     */
    public Iterator<IPv6Network> split(IPv6NetworkMask size) {
        if (size.asPrefixLength() < this.getNetmask().asPrefixLength()) {
            throw new IllegalArgumentException(
                String.format(Locale.ROOT, "Can not split a network of size %s in subnets of larger size %s",
                    this.getNetmask().asPrefixLength(), size.asPrefixLength()));
        }

        return new IPv6NetworkSplitsIterator(size);
    }

    @Override
    public String toString() {
        return address.toString() + "/" + networkMask.asPrefixLength();
    }

    /**
     * toLongString
     *
     * @return String
     */
    @Override
    public String toLongString() {
        return address.toLongString() + "/" + networkMask.asPrefixLength();
    }

    @Override
    public boolean equals(Object object) {
        if (this == object) {
            return true;
        }
        if (object == null) {
            return false;
        }
        if (!super.equals(object)) {
            return false;
        }

        if (!(object instanceof IPv6Network)) {
            return false;
        }

        IPv6Network that = (IPv6Network) object;

        if (address != null ? !address.equals(that.address) : that.address != null) {
            return false;
        }
        return networkMask != null ? networkMask.equals(that.networkMask) : that.networkMask == null;
    }

    @Override
    public int hashCode() {
        int result = super.hashCode();
        result = 31 * result + (address != null ? address.hashCode() : 0);
        return 31 * result + (networkMask != null ? networkMask.hashCode() : 0);
    }

    /**
     * getNetmask
     *
     * @return IPv6NetworkMask
     */
    public IPv6NetworkMask getNetmask() {
        return networkMask;
    }

    private final class IPv6NetworkSplitsIterator implements Iterator<IPv6Network> {
        private final IPv6NetworkMask size;

        private IPv6Network current;

        private final BigInteger nbrAddressesPerSplit;

        public IPv6NetworkSplitsIterator(IPv6NetworkMask size) {
            this.size = size;
            this.nbrAddressesPerSplit = BigInteger.ONE.shiftLeft(128 - size.asPrefixLength());
            this.current = IPv6Network.fromAddressAndMask(IPv6Network.this.address, size);
        }

        @Override
        public boolean hasNext() {
            return current.getLast().compareTo(IPv6Network.this.getLast()) <= 0;
        }

        @Override
        public IPv6Network next() {
            if (hasNext()) {
                IPv6Network result = current;
                current = calculateNext(current);
                return result;
            } else {
                throw new NoSuchElementException();
            }
        }

        private IPv6Network calculateNext(IPv6Network current) {
            BigInteger next = current.address.toBigInteger().add(nbrAddressesPerSplit);
            return IPv6Network.fromAddressAndMask(IPv6Address.fromBigInteger(next), size);
        }

        @Override
        public void remove() {
            throw new UnsupportedOperationException("This iterator provides read only access");
        }
    }
}