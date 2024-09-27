package openbackup.system.base.config.business.initialize.beans;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * 存储卷配置
 *
 * @author w00493811
 * @since 2021-02-02
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
public class StorageVolumeBean {
    /**
     * 类型
     */
    private int type;

    /**
     * 数量
     */
    private int number;

    /**
     * 名称
     */
    private String name;

    /**
     * 容量
     */
    private long size;
}
