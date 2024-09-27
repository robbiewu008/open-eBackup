package openbackup.system.base.common.utils.network;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import java.math.BigInteger;
import java.nio.ByteBuffer;
import java.nio.LongBuffer;
import java.util.Arrays;
import java.util.Locale;

/**
 * IPv6Address
 *
 * @author zKF66175
 * @version [V100R002C00, 2013-2-5]
 * @since 2019-10-25
 */
public final class IPv6Address implements Comparable<IPv6Address> {
    /**
     * MAX_ADDRESS
     */
    public static final String MAX_ADDRESS = "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff";

    /**
     * MAX
     */
    public static final IPv6Address MAX = IPv6Address.fromString(MAX_ADDRESS);

    private static final int N_SHORTS = 8;

    private static final int N_BYTES = 16;

    private final long highBits;

    private final long lowBits;

    /**
     * 构造函数
     *
     * @param highBits highBits
     * @param lowBits lowBits
     */
    IPv6Address(long highBits, long lowBits) {
        this.highBits = highBits;
        this.lowBits = lowBits;
    }

    /**
     * Create an IPv6 address from its String representation.
     * For example "1234:5678:abcd:0000:9876:3210:ffff:ffff" or "2001::ff" or even
     * "::". IPv4-Mapped IPv6 addresses such as "::ffff:123.456.123.456" are also supported.
     *
     * @param string string representation
     * @return IPv6 address
     */
    public static IPv6Address fromString(final String string) {
        if (string == null) {
            throw new IllegalArgumentException("can not parse [null]");
        }

        final String withoutIPv4MappedNotation = IPv6AddressHelpers.rewriteIPv4MappedNotation(string);
        final String longNotation = IPv6AddressHelpers.expandShortNotation(withoutIPv4MappedNotation);

        final long[] longs = tryParseStringArrayIntoLongArray(string, longNotation);

        IPv6AddressHelpers.validateLongs(longs);

        return IPv6AddressHelpers.mergeLongArrayIntoIPv6Address(longs);
    }

    private static long[] tryParseStringArrayIntoLongArray(String string, String longNotation) {
        try {
            return IPv6AddressHelpers.parseStringArrayIntoLongArray(longNotation.split(":"));
        } catch (NumberFormatException e) {
            throw new IllegalArgumentException("can not parse [" + string + "]");
        }
    }

    /**
     * Create an IPv6 address from a byte array.
     *
     * @param bytes byte array with 16 bytes (interpreted unsigned)
     * @return IPv6 address
     */
    public static IPv6Address fromByteArray(final byte[] bytes) {
        if (bytes == null) {
            throw new IllegalArgumentException("can not construct from [null]");
        }
        if (bytes.length != N_BYTES) {
            throw new IllegalArgumentException("the byte array to construct from should be 16 bytes long");
        }

        ByteBuffer buf = ByteBuffer.allocate(N_BYTES);
        for (byte tmp : bytes) {
            buf.put(tmp);
        }

        buf.rewind();
        LongBuffer longBuffer = buf.asLongBuffer();
        return new IPv6Address(longBuffer.get(), longBuffer.get());
    }

    /**
     * toByteArray
     *
     * @return byte[] representation
     */
    public byte[] toByteArray() {
        ByteBuffer byteBuffer = ByteBuffer.allocate(N_BYTES).putLong(highBits).putLong(lowBits);
        return byteBuffer.array();
    }

    /**
     * Create an IPv6 address from a (positive) {@link java.math.BigInteger}.
     * The magnitude of the {@link java.math.BigInteger} represents
     * the IPv6 address value. Or in other words, the {@link java.math.BigInteger}
     * with value N defines the Nth possible IPv6 address.
     *
     * @param bigInteger {@link java.math.BigInteger} value
     * @return IPv6 address
     */
    public static IPv6Address fromBigInteger(final BigInteger bigInteger) {
        if (bigInteger == null) {
            throw new IllegalArgumentException("can not construct from [null]");
        }
        if (bigInteger.compareTo(BigInteger.ZERO) < 0) {
            throw new IllegalArgumentException("can not construct from negative value");
        }
        if (bigInteger.compareTo(MAX.toBigInteger()) > 0) {
            throw new IllegalArgumentException("bigInteger represents a value bigger than 2^128 - 1");
        }

        byte[] bytes = bigInteger.toByteArray();

        if (bytes[0] == 0) {
            // a zero byte was added to represent the (always positive, hence zero) sign bit
            return fromByteArray(
                IPv6AddressHelpers.prefixWithZeroBytes(Arrays.copyOfRange(bytes, 1, bytes.length), N_BYTES));
        } else {
            return fromByteArray(IPv6AddressHelpers.prefixWithZeroBytes(bytes, N_BYTES));
        }
    }

    /**
     * The magnitude of the {@link java.math.BigInteger} represents the IPv6 address
     * value. Or in other words, the {@link java.math.BigInteger}
     * with value N defines the Nth possible IPv6 address.
     *
     * @return BigInteger
     */
    public BigInteger toBigInteger() {
        return new BigInteger(1, toByteArray());
    }

    /**
     * Addition. Will never overflow, but wraps around
     * when the highest ip address has been reached.
     *
     * @param value value to add
     * @return new IPv6 address
     */
    public IPv6Address add(int value) {
        final long newLowBits = lowBits + value;

        if (value >= 0) {
            if (IPv6AddressHelpers.isLessThanUnsigned(newLowBits, lowBits)) {
                // oops, we added something positive and the result is smaller
                // -> overflow detected (carry over one bit from low to high)
                return new IPv6Address(highBits + 1, newLowBits);
            } else {
                // no overflow
                return new IPv6Address(highBits, newLowBits);
            }
        } else {
            if (IPv6AddressHelpers.isLessThanUnsigned(lowBits, newLowBits)) {
                // oops, we added something negative and the result is bigger
                // -> overflow detected (carry over one bit from high to low)
                return new IPv6Address(highBits - 1, newLowBits);
            } else {
                // no overflow
                return new IPv6Address(highBits, newLowBits);
            }
        }
    }

    /**
     * Subtraction. Will never underflow, but wraps around
     * when the lowest ip address has been reached.
     *
     * @param value value to substract
     * @return new IPv6 address
     */
    public IPv6Address subtract(int value) {
        final long newLowBits = lowBits - value;

        if (value >= 0) {
            if (IPv6AddressHelpers.isLessThanUnsigned(lowBits, newLowBits)) {
                // oops, we subtracted something postive and the result is bigger
                // -> overflow detected (carry over one bit from high to low)
                return new IPv6Address(highBits - 1, newLowBits);
            } else {
                // no overflow
                return new IPv6Address(highBits, newLowBits);
            }
        } else {
            if (IPv6AddressHelpers.isLessThanUnsigned(newLowBits, lowBits)) {
                // oops, we subtracted something negative and the result is smaller
                // -> overflow detected (carry over one bit from low to high)
                return new IPv6Address(highBits + 1, newLowBits);
            } else {
                // no overflow
                return new IPv6Address(highBits, newLowBits);
            }
        }
    }

    /**
     * Mask the address with the given network mask.
     *
     * @param networkMask network mask
     * @return an address of which the last 128 - networkMask.asPrefixLength() bits are zero
     */
    public IPv6Address maskWithNetworkMask(final IPv6NetworkMask networkMask) {
        if (networkMask.asPrefixLength() == 128) {
            return this;
        } else if (networkMask.asPrefixLength() == 64) {
            return new IPv6Address(this.highBits, 0);
        } else if (networkMask.asPrefixLength() == 0) {
            return new IPv6Address(0, 0);
        } else if (networkMask.asPrefixLength() > 64) {
            // apply mask on low bits only
            final int remainingPrefixLength = networkMask.asPrefixLength() - 64;
            return new IPv6Address(this.highBits, this.lowBits & (0xFFFFFFFFFFFFFFFFL << (64 - remainingPrefixLength)));
        } else {
            // apply mask on high bits, low bits completely 0
            return new IPv6Address(this.highBits & (0xFFFFFFFFFFFFFFFFL << (64 - networkMask.asPrefixLength())), 0);
        }
    }

    /**
     * Calculate the maximum address with the given network mask.
     *
     * @param networkMask network mask
     * @return an address of which the last 128 - networkMask.asPrefixLength() bits are one
     */
    public IPv6Address maximumAddressWithNetworkMask(final IPv6NetworkMask networkMask) {
        if (networkMask.asPrefixLength() == 128) {
            return this;
        } else if (networkMask.asPrefixLength() == 64) {
            return new IPv6Address(this.highBits, 0xFFFFFFFFFFFFFFFFL);
        } else if (networkMask.asPrefixLength() > 64) {
            // apply mask on low bits only
            final int remainingPrefixLength = networkMask.asPrefixLength() - 64;
            return new IPv6Address(this.highBits, this.lowBits | (0xFFFFFFFFFFFFFFFFL >>> remainingPrefixLength));
        } else {
            // apply mask on high bits, low bits completely 1
            return new IPv6Address(this.highBits | (0xFFFFFFFFFFFFFFFFL >>> networkMask.asPrefixLength()),
                0xFFFFFFFFFFFFFFFFL);
        }
    }

    /**
     * Set a bit in the address.
     *
     * @param bit bit
     * @return IPv6Address
     */
    public IPv6Address setBit(final int bit) {
        if (bit < 0 || bit > 127) {
            throw new IllegalArgumentException("can only set bits in the interval [0, 127]");
        }

        if (bit < 64) {
            return new IPv6Address(this.highBits, this.lowBits | (1L << bit));
        } else {
            return new IPv6Address(this.highBits | (1L << (bit - 64)), this.lowBits);
        }
    }

    /**
     * Returns true if the address is an IPv4-mapped IPv6 address.
     * In these addresses, the first 80 bits are zero, the next 16 bits are one,
     * and the remaining 32 bits are the IPv4 address.
     *
     * @return boolean
     */
    public boolean isIPv4Mapped() {
        boolean isHighBitsZero = this.highBits == 0;
        boolean isLowBitsZero = (this.lowBits & 0xFFFF000000000000L) == 0;
        boolean isLowBitsNonZero = (this.lowBits & 0x0000FFFF00000000L) == 0x0000FFFF00000000L;
        return (isHighBitsZero && isLowBitsZero) && isLowBitsNonZero;
    }

    /**
     * Returns a string representation of the IPv6 address. It will use shorthand notation
     * and special notation for IPv4-mapped IPv6
     * addresses whenever possible.
     *
     * @return String
     */
    @Override
    public String toString() {
        if (isIPv4Mapped()) {
            return toIPv4MappedAddressString();
        } else {
            return toShortHandNotationString();
        }
    }

    private String toIPv4MappedAddressString() {
        int byteZero = (int) ((this.lowBits & 0x00000000FF000000L) >> 24);
        int byteOne = (int) ((this.lowBits & 0x0000000000FF0000L) >> 16);
        int byteTwo = (int) ((this.lowBits & 0x000000000000FF00L) >> 8);
        int byteThree = (int) ((this.lowBits & 0x00000000000000FFL));

        final StringBuilder result = new StringBuilder("::ffff:");
        result.append(byteZero).append(".").append(byteOne).append(".").append(byteTwo).append(".").append(byteThree);

        return result.toString();
    }

    private String toShortHandNotationString() {
        final String[] strings = toArrayOfShortStrings();

        final StringBuilder result = new StringBuilder();

        int[] shortHandNotationPositionAndLength = startAndLengthOfLongestRunOfZeroes();
        int shortHandNotationPosition = shortHandNotationPositionAndLength[0];
        int shortHandNotationLength = shortHandNotationPositionAndLength[1];

        // RFC5952 recommends not to use shorthand notation for a single zero
        boolean isUseShortHandNotation = shortHandNotationLength > 1;

        for (int i = 0; i < strings.length; i++) {
            if (isUseShortHandNotation && i == shortHandNotationPosition) {
                if (i == 0) {
                    result.append("::");
                } else {
                    result.append(":");
                }
            } else if (!(i > shortHandNotationPosition && i < shortHandNotationPosition + shortHandNotationLength)) {
                result.append(strings[i]);
                if (i < N_SHORTS - 1) {
                    result.append(":");
                }
            } else {
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM);
            }
        }

        return result.toString().toLowerCase(Locale.ROOT);
    }

    private String[] toArrayOfShortStrings() {
        final short[] shorts = toShortArray();
        final String[] strings = new String[shorts.length];
        for (int i = 0; i < shorts.length; i++) {
            strings[i] = String.format(Locale.ROOT, "%x", shorts[i]);
        }
        return strings;
    }

    /**
     * representation of the IPv6 address, never using shorthand notation.
     *
     * @return String
     */
    public String toLongString() {
        final String[] strings = toArrayOfZeroPaddedstrings();
        final StringBuilder result = new StringBuilder();
        for (int i = 0; i < strings.length - 1; i++) {
            result.append(strings[i]).append(":");
        }

        result.append(strings[strings.length - 1]);

        return result.toString();
    }

    private String[] toArrayOfZeroPaddedstrings() {
        final short[] shorts = toShortArray();
        final String[] strings = new String[shorts.length];
        for (int i = 0; i < shorts.length; i++) {
            strings[i] = String.format(Locale.ROOT, "%04x", shorts[i]);
        }
        return strings;
    }

    private short[] toShortArray() {
        final short[] shorts = new short[N_SHORTS];

        for (int i = 0; i < N_SHORTS; i++) {
            if (IPv6AddressHelpers.inHighRange(i)) {
                shorts[i] = (short) (((highBits << i * 16) >>> 16 * (N_SHORTS - 1)) & 0xFFFF);
            } else {
                shorts[i] = (short) (((lowBits << i * 16) >>> 16 * (N_SHORTS - 1)) & 0xFFFF);
            }
        }

        return shorts;
    }

    int[] startAndLengthOfLongestRunOfZeroes() {
        int longestConsecutiveZeroes = 0;
        int longestConsecutiveZeroesPos = -1;
        short[] shorts = toShortArray();
        for (int pos = 0; pos < shorts.length; pos++) {
            int consecutiveZeroesAtCurrentPos = countConsecutiveZeroes(shorts, pos);
            if (consecutiveZeroesAtCurrentPos > longestConsecutiveZeroes) {
                longestConsecutiveZeroes = consecutiveZeroesAtCurrentPos;
                longestConsecutiveZeroesPos = pos;
            }
        }

        return new int[] {longestConsecutiveZeroesPos, longestConsecutiveZeroes};
    }

    private int countConsecutiveZeroes(short[] shorts, int offset) {
        int count = 0;
        for (int i = offset; i < shorts.length && shorts[i] == 0; i++) {
            count++;
        }

        return count;
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
        if (object == null) {
            return false;
        }

        if (!(object instanceof IPv6Address)) {
            return false;
        }

        IPv6Address that = (IPv6Address) object;

        if (highBits != that.highBits) {
            return false;
        }

        return lowBits == that.lowBits;
    }

    /**
     * hashCode
     *
     * @return int
     */
    @Override
    public int hashCode() {
        int result = (int) (lowBits ^ (lowBits >>> 32));
        return 31 * result + (int) (highBits ^ (highBits >>> 32));
    }

    /**
     * compareTo
     *
     * @param that that
     * @return int
     */
    public int compareTo(IPv6Address that) {
        if (this.highBits == that.highBits) {
            if (this.lowBits == that.lowBits) {
                return 0;
            } else {
                return IPv6AddressHelpers.isLessThanUnsigned(this.lowBits, that.lowBits) ? -1 : 1;
            }
        } else {
            return IPv6AddressHelpers.isLessThanUnsigned(this.highBits, that.highBits) ? -1 : 1;
        }
    }

    /**
     * getHighBits
     *
     * @return long
     */
    public long getHighBits() {
        return highBits;
    }

    /**
     * getLowBits
     *
     * @return long
     */
    public long getLowBits() {
        return lowBits;
    }

    /**
     * numberOfTrailingZeroes
     *
     * @return int
     */
    public int numberOfTrailingZeroes() {
        return lowBits == 0 ?
            Long.numberOfTrailingZeros(highBits) + 64 :
            Long.numberOfTrailingZeros(lowBits);
    }

    /**
     * numberOfLeadingZeroes
     *
     * @return int
     */
    public int numberOfLeadingZeroes() {
        return highBits == 0 ?
            Long.numberOfLeadingZeros(lowBits) + 64 :
            Long.numberOfLeadingZeros(highBits);
    }

    /**
     * numberOfLeadingOnes
     *
     * @return int
     */
    public int numberOfLeadingOnes() {
        // count leading ones in "value" by counting leading zeroes in "~ value"
        final IPv6Address flipped = new IPv6Address(~this.highBits, ~this.lowBits);
        return flipped.numberOfLeadingZeroes();
    }
}
