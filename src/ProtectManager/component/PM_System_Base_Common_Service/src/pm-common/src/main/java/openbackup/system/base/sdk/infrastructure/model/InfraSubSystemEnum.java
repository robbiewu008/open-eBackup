package openbackup.system.base.sdk.infrastructure.model;

import lombok.Getter;

/**
 * sub system type for infrastructure
 *
 * @author t00508428
 * @since 2020-12-17
 */
@Getter
public enum InfraSubSystemEnum {
    AISHU("AISHU"),
    INFRA("INFRA");

    private final String value;

    InfraSubSystemEnum(String value) {
        this.value = value;
    }
}
