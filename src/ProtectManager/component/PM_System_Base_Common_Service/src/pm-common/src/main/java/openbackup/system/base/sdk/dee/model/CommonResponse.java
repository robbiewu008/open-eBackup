package openbackup.system.base.sdk.dee.model;

import lombok.Data;

/**
 * 功能描述
 *
 * @author c30047317
 * @since 2023-08-14
 */
@Data
public class CommonResponse {
    // 错误码，0为成功
    private long errorCode;

    // 错误信息
    private String errorMessage;

    // 参数
    private String parameters;
}