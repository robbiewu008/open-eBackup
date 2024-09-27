package openbackup.data.access.framework.protection.plugin;

import openbackup.data.protection.access.provider.sdk.plugin.PluginConfig;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.plugin.PluginExtensionInvokeContext;
import openbackup.data.protection.access.provider.sdk.plugin.ResourceExtensionHandler;
import openbackup.data.protection.access.provider.sdk.plugin.ResourceExtensionManager;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;

import com.fasterxml.jackson.databind.JsonNode;

import org.springframework.beans.factory.InitializingBean;
import org.springframework.context.ApplicationContext;
import org.springframework.stereotype.Component;

import java.lang.reflect.ParameterizedType;
import java.lang.reflect.Type;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * 统一资源扩展管理
 * ResourceExtensionHandler与PluginExtensionInvokeContext泛型信息需要相同，并且必须在该父类指定具体类型
 *
 * @author h30027154
 * @since 2022-05-30
 */
@Component
public class UnifiedResourceExtensionManager implements ResourceExtensionManager, InitializingBean {
    private List<ResourceExtensionHandler> resourceExtensionHandlers;

    private ApplicationContext applicationContext;

    private PluginConfigManager pluginConfigManager;

    public UnifiedResourceExtensionManager(ApplicationContext applicationContext,
        PluginConfigManager pluginConfigManager) {
        this.applicationContext = applicationContext;
        this.pluginConfigManager = pluginConfigManager;
    }

    @Override
    public <T, R> R invoke(String subType, String namePath, PluginExtensionInvokeContext<T, R> context) {
        if (context == null) {
            return null;
        }
        Optional<ResourceExtensionHandler<T, R>> handlerOptional = findResourceExtensionHandler(namePath, context);
        if (!handlerOptional.isPresent()) {
            throw new LegoCheckedException("can not find ResourceExtensionHandler for namePath: " + namePath);
        }
        Optional<PluginConfig> pluginConfig = pluginConfigManager.getPluginConfig(subType);
        Optional<Object> configObject = resolvePluginConfig(pluginConfig.orElse(null), namePath);
        R handleRes = handlerOptional.get().handle(configObject.orElse(null), context.getParams());
        context.setResult(handleRes);
        return handleRes;
    }

    @Override
    public void afterPropertiesSet() {
        if (resourceExtensionHandlers == null) {
            resourceExtensionHandlers = new ArrayList<>();
        }
        resourceExtensionHandlers.addAll(applicationContext.getBeansOfType(ResourceExtensionHandler.class).values());
        // check duplicate name
        Map<String, Long> namePathMap = resourceExtensionHandlers.stream()
            .collect(Collectors.groupingBy(ResourceExtensionHandler::getNamePath, Collectors.counting()));
        namePathMap.forEach((namePath, count) -> {
            if (count > 1) {
                throw new LegoUncheckedException(
                    "resource extend handler name path(" + namePath + ") exists more than 1");
            }
        });
    }

    private <T, R> Optional<ResourceExtensionHandler<T, R>> findResourceExtensionHandler(String namePath,
        PluginExtensionInvokeContext<T, R> context) {
        Optional<ResourceExtensionHandler> extensionHandlerOptional = resourceExtensionHandlers.stream()
            .filter(handler -> Objects.equals(handler.getNamePath(), namePath))
            .findFirst();
        if (!extensionHandlerOptional.isPresent()) {
            return Optional.empty();
        }
        ResourceExtensionHandler resourceExtensionHandler = extensionHandlerOptional.get();
        // check ParameterizedType
        if (!checkParameterizedType(resourceExtensionHandler, context)) {
            throw new LegoCheckedException(
                resourceExtensionHandler.getClass() + " parameterized type can not match " + context.getClass());
        }
        return Optional.of((ResourceExtensionHandler<T, R>) resourceExtensionHandler);
    }

    private boolean checkParameterizedType(ResourceExtensionHandler resourceExtensionHandler,
        PluginExtensionInvokeContext context) {
        Type[] handlerParameterizeTypes = getParameterizedActualType(resourceExtensionHandler.getClass());
        Type[] contextParameterizeTypes = getParameterizedActualType(context.getClass());
        return Arrays.equals(handlerParameterizeTypes, contextParameterizeTypes);
    }

    private Type[] getParameterizedActualType(Class tClass) {
        Type genericSuperclass = tClass.getGenericSuperclass();
        if (genericSuperclass instanceof Class) {
            if (Objects.equals(Object.class, genericSuperclass)) {
                return new Type[0];
            } else {
                return getParameterizedActualType((Class) genericSuperclass);
            }
        } else if (genericSuperclass instanceof ParameterizedType) {
            ParameterizedType parameterizedType = (ParameterizedType) genericSuperclass;
            return parameterizedType.getActualTypeArguments();
        } else {
            return new Type[0];
        }
    }

    private Optional<Object> resolvePluginConfig(PluginConfig config, String namePath) {
        if (config == null) {
            return Optional.empty();
        }
        String[] namePathSplits = namePath.split("\\.");
        if (namePathSplits.length == 0) {
            return Optional.empty();
        }
        JsonNode jsonNode = config.getConfigMap().get(namePathSplits[0]);
        if (jsonNode == null) {
            return Optional.empty();
        }
        JsonNode tmpNode = jsonNode;
        for (int i = 1; i < namePathSplits.length; i++) {
            tmpNode = tmpNode.path(namePathSplits[i]);
        }
        return Optional.ofNullable(JsonUtil.read(tmpNode.toString(), Object.class));
    }
}
