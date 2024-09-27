package openbackup.data.protection.access.provider.sdk.resource;

import lombok.Getter;
import lombok.Setter;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * The CheckContext
 *
 * @author g30003063
 * @since 2022/5/30
 */
@Getter
@Setter
public class ResourceCheckContext {
    private List<ActionResult> actionResults;

    private Map<ProtectedResource, List<ProtectedEnvironment>> resourceConnectableMap;

    private Map<String, Object> context = new HashMap<>();
}