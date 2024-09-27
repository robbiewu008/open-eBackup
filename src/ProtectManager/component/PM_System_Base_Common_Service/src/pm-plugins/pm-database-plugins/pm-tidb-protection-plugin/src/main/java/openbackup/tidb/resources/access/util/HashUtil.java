package openbackup.tidb.resources.access.util;

import lombok.extern.slf4j.Slf4j;

import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

/**
 * HashUtil 工具类
 *
 * @author w00426202
 * @since 2023-10-12
 */
@Slf4j
public final class HashUtil {
    private static final char[] SIXTEEN = new char[] {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };

    /**
     * hash 散列
     *
     * @param plain 字符串
     * @return 散列后的值
     */
    public static String digest(String plain) {
        try {
            byte[] plainBytes = plain.getBytes(StandardCharsets.UTF_8);
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            byte[] digestBytes = md.digest(plainBytes);
            return new String(byteToHex(digestBytes));
        } catch (NoSuchAlgorithmException var4) {
            log.error("no algorithm {}", "SHA-256");
            throw new IllegalArgumentException(var4);
        }
    }

    private static char[] byteToHex(byte[] encryptedBytes) {
        char[] result = new char[encryptedBytes.length * 2];

        for (int i = 0; i < encryptedBytes.length; ++i) {
            byte encryptedByte = encryptedBytes[i];
            int high = (240 & encryptedByte) >> 4;
            int low = 15 & encryptedByte;
            result[i * 2] = SIXTEEN[high];
            result[i * 2 + 1] = SIXTEEN[low];
        }
        return result;
    }
}