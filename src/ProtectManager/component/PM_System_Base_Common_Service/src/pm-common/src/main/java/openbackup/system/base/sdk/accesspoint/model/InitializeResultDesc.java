package openbackup.system.base.sdk.accesspoint.model;

import openbackup.system.base.sdk.accesspoint.model.enums.InitializeResultCode;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;
import lombok.ToString;

/**
 * 初始化动作结果描述
 *
 * @author w00493811
 * @since 2020-12-26
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
@ToString
public class InitializeResultDesc {
    /**
     * 初始化错误编码
     */
    private InitializeResultCode code;

    /**
     * 初始化错误描述
     */
    private String desc;
}
