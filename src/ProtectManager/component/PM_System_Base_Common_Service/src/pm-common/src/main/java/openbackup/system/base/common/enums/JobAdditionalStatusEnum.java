package openbackup.system.base.common.enums;

import openbackup.system.base.util.EnumUtil;

/**
 * 任务附加状态枚举
 *
 * @author w00616953
 * @since 2022-03-07
 */
public enum JobAdditionalStatusEnum {
    /**
     * 数据库可用
     */
    DATABASE_AVAILABLE("Database Available"),

    /**
     * 虚拟机可用
     */
    VIRTUAL_MACHINE_AVAILABLE("Virtual Machine Available");

    private final String value;

    JobAdditionalStatusEnum(String value) {
        this.value = value;
    }

    /**
     * 获取枚举对应的值
     *
     * @return 枚举对应的值
     */
    public String getValue() {
        return value;
    }

    /**
     * 根据值获取枚举实例
     *
     * @param additionalStatus 附加状态值
     * @return 附加状态枚举实例
     */
    public static JobAdditionalStatusEnum getEnum(String additionalStatus) {
        return EnumUtil.get(
                JobAdditionalStatusEnum.class, JobAdditionalStatusEnum::getValue, additionalStatus, false, true);
    }
}
