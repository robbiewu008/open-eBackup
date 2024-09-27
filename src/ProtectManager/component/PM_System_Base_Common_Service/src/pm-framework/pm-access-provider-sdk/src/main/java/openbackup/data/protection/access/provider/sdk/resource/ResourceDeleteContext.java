package openbackup.data.protection.access.provider.sdk.resource;

import lombok.Data;

import java.util.ArrayList;
import java.util.List;

/**
 * 删除资源的上下文
 *
 * @author h30027154
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-05
 */
@Data
public class ResourceDeleteContext {
    private List<ResourceDeleteDependency> resourceDeleteDependencyList;

    /**
     * 默认值
     *
     * @return ResourceDeleteContext
     */
    public static ResourceDeleteContext defaultValue() {
        ResourceDeleteContext resourceDeleteContext = new ResourceDeleteContext();
        resourceDeleteContext.setResourceDeleteDependencyList(new ArrayList<>());
        return resourceDeleteContext;
    }

    /**
     * 资源删除时的依赖检查
     */
    @Data
    public static class ResourceDeleteDependency {
        // 是否进行依赖判断
        private boolean shouldCheckIfBeDependency;

        // 要删除的资源ID, 返回parent-child关系的顶层资源
        private List<String> deleteIds;

        /**
         * 默认值
         *
         * @return ResourceDeleteDependency
         */
        public static ResourceDeleteDependency defaultValue() {
            ResourceDeleteDependency resourceDeleteDependency = new ResourceDeleteDependency();
            resourceDeleteDependency.setShouldCheckIfBeDependency(true);
            resourceDeleteDependency.setDeleteIds(new ArrayList<>());
            return resourceDeleteDependency;
        }
    }
}
