package openbackup.access.framework.resource.dto;

import lombok.EqualsAndHashCode;
import lombok.Getter;
import lombok.Setter;

/**
 * The ResourceDependencyRelation
 *
 * @author g30003063
 * @since 2022/5/28
 */
@Getter
@Setter
@EqualsAndHashCode
public class ResourceDependencyRelation {
    private String uuid;

    private String type;

    private String parentUuid;
}