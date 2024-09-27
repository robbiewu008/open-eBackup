package openbackup.system.base.sdk.kmc.model;

import lombok.Data;

import javax.validation.constraints.NotEmpty;

/**
 * 功能描述
 *
 * @author y00413474
 * @version [BCManager 8.0.0]
 * @since 2020-05-30
 */
@Data
public class PlaintextVo {
    @NotEmpty(message = "plaintext cannot be empty.")
    private String plaintext;
}
