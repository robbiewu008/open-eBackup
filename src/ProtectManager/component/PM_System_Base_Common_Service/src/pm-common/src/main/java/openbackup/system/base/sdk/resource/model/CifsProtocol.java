package openbackup.system.base.sdk.resource.model;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.Set;

/**
 * 功能描述
 *
 * @author y30044273
 * @since 2023-11-17
 */
@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
public class CifsProtocol {
    private String shareName;

    private String userType;

    private Set<String> userNames;
}
