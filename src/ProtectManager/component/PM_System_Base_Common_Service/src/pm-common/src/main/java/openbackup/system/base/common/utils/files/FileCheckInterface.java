package openbackup.system.base.common.utils.files;

import openbackup.system.base.bean.FileCheckRule;

import org.springframework.web.multipart.MultipartFile;

/**
 * 文件校验接口
 *
 * @author w00607005
 * @since 2023-09-21
 */
public interface FileCheckInterface {
    /**
     * 检验文件
     *
     * @param multipartFile 文件
     * @param validateRule 校验规则
     *
     * @return 校验结果
     */
    boolean check(MultipartFile multipartFile, FileCheckRule validateRule);
}
