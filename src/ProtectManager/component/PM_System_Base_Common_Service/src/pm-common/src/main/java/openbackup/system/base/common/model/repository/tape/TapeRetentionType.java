package openbackup.system.base.common.model.repository.tape;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import lombok.AllArgsConstructor;
import lombok.Getter;

/**
 * 保留策略
 *
 * @author z30006621
 * @since 2021-08-24
 */
@Getter
@AllArgsConstructor
public enum TapeRetentionType {
    /**
     * 立刻过期
     */
    IMMEDIATE(1),

    /**
     * 永不过期
     */
    PERMANENT(2),

    /**
     * 临时
     */
    TEMPORARY(3);

    private final int value;

    /**
     * 通过value获取RetentionTypeEnum
     *
     * @param value 值
     * @return 保留类型
     */
    public static TapeRetentionType getRetentionType(int value) {
        for (TapeRetentionType tapeRetentionType : values()) {
            if (value == tapeRetentionType.getValue()) {
                return tapeRetentionType;
            }
        }
        throw new LegoCheckedException(CommonErrorCode.ERR_PARAM);
    }
}
