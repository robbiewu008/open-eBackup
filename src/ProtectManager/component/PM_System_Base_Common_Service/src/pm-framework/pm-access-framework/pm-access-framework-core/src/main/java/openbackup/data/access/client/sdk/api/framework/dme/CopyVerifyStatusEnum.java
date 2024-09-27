package openbackup.data.access.client.sdk.api.framework.dme;

import lombok.AllArgsConstructor;
import lombok.Getter;

import java.util.Arrays;

/**
 * 副本校验状态枚举
 *
 * @author c30016231
 * @since 2022-08-03
 * @version [OceanProtect X8000 1.2.1]
 */
@Getter
@AllArgsConstructor
public enum CopyVerifyStatusEnum {
    /**
     * 未校验
     */
    NOT_VERIFY("0"),

    /**
     * 校验失败
     */
    VERIFY_FAILED("1"),

    /**
     * 校验成功
     */
    VERIFY_SUCCESS("2"),

    /**
     * 校验文件不存在
     */
    VERIFY_FILE_NOT_EXIST("3");

    private final String verifyStatus;

    /**
     * 根据副本校验状态值获取枚举类
     *
     * @param verifyStatus 副本校验状态值
     * @return 副本校验状态值枚举类 {@code CopyVerifyStatusEnum}
     */
    public static CopyVerifyStatusEnum getByStatus(String verifyStatus) {
        return Arrays.stream(CopyVerifyStatusEnum.values())
            .filter(statusEnum -> statusEnum.verifyStatus.equals(verifyStatus)).findFirst()
            .orElseThrow(IllegalArgumentException::new);
    }
}
