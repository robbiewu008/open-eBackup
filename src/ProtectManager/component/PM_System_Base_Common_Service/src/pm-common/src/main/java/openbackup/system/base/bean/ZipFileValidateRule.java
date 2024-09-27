package openbackup.system.base.bean;

import lombok.Data;
import lombok.experimental.SuperBuilder;

import java.util.List;

/**
 * 压缩文件校验规则
 *
 * @author hwx1144169
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022-04-07
 */
@Data
@SuperBuilder
public class ZipFileValidateRule extends FileValidateRule {
    // 压缩文件中子文件最大允许数量
    private long fileMaxNum;

    // 压缩文件临时存放目录
    private String tempPath;

    // 压缩炸弹校验时，指定的解压文件临时存放路径
    private String unzipTempPath;

    // 解压文件后的文件大小最大允许值
    private long maxDecompressSize;

    // 解压文件包含的子文件列表
    private List<String> subFileList;
}
