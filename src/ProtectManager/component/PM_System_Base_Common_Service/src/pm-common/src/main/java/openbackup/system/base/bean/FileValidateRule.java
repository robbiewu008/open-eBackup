package openbackup.system.base.bean;

import lombok.Data;
import lombok.experimental.SuperBuilder;

/**
 * 通用文件校验规则
 *
 * @author hwx1144169
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022-03-31
 */
@Data
@SuperBuilder
public class FileValidateRule {
    // 文件名要求长度
    private int nameMaxLength;

    // 文件后缀
    private String suffix;

    // 文件大小
    private long maxSize;

    // 文件路径
    private String path;

    // 文件路径规则正则表达式
    private String pathRule;
}
