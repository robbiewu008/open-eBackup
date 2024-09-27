package openbackup.system.base.sdk.auth.model.request;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * HSC token 认证字段
 *
 * @author l30044826
 * @since 2023-07-07
 */
@Getter
@Setter
public class Identity {
    private List<String> methods;

    private Password password;
}
