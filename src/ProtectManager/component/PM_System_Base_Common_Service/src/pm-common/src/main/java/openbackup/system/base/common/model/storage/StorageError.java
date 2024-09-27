package openbackup.system.base.common.model.storage;

import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.utils.VerifyUtil;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 功能描述
 *
 * @author y00413474
 * @since 2020-07-01
 */
@Data
public class StorageError {
    /**
     * 错误编码
     */
    @JsonProperty("code")
    private String code;

    /**
     * 错误描述
     */
    @JsonProperty("description")
    private String description;

    /**
     * 错误建议
     */
    @JsonProperty("suggestion")
    private String suggestion;

    /**
     * 错误参数
     */
    @JsonProperty("errorParam")
    private String errorParam;

    /**
     * 从device获取的值是否成功
     *
     * @return 接口是否返回成功
     */
    public boolean isSuccess() {
        return !VerifyUtil.isEmpty(code) && String.valueOf(LegoNumberConstant.ZERO).equals(code);
    }
}
