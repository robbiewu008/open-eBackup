package openbackup.system.base.sdk.cluster.model;

import lombok.Data;

import javax.validation.constraints.NotNull;
import javax.validation.constraints.Pattern;
import javax.validation.constraints.Size;

/**
 * 内部组件密码info类
 *
 * @author z00613137
 * @since 2023-05-22
 */
@Data
public class ClusterComponentPwdInfo {
    /**
     * 内部组件密码的Key值
     */
    @NotNull
    @Size(min = 1, max = 256)
    private String passwordField;

    /**
     * 内部组件密码的Value值
     * 根据秘钥生成规则，需包含字母、数字、特殊字符，长度在8-18位
     */
    @NotNull
    @Pattern(regexp = "^(?=.*[a-zA-Z])(?=.*\\d)(?=.*[~!@#$%^&*()_+`\\-={}|\\[\\]:;\"'<>,.?\\\\/ ]).{8,18}$")
    private String passwordValue;
}