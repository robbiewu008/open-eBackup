package openbackup.system.base.sdk.resource.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;
import lombok.EqualsAndHashCode;

/**
 * 数据库实体类
 *
 * @author t00482481
 * @since 2020-07-05
 */
@EqualsAndHashCode(callSuper = true)
@Data
public class DatasourceEntity extends ResourceEntity {
    @JsonProperty("link_status")
    private String linkStatus;

    @JsonProperty("verify_status")
    private String verifyStatus;

    @JsonProperty("db_username")
    private String dbUsername;

    @JsonProperty("db_password")
    private String dbPassword;

    @JsonProperty("db_role")
    private int dbRole;

    @JsonProperty("auth_type")
    private int authType;

    @JsonProperty("inst_name")
    private String instName;

    @JsonProperty("is_asminst")
    private int isAsmInst;

    @JsonProperty("version")
    private String version;

    @JsonProperty("asm")
    private String asm;

    @JsonProperty("asm_auth")
    private String asmAuth;
}
