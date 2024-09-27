package openbackup.system.base.common.factory;

import org.springframework.boot.env.YamlPropertySourceLoader;
import org.springframework.core.env.PropertySource;
import org.springframework.core.io.support.EncodedResource;
import org.springframework.core.io.support.PropertySourceFactory;

import java.io.IOException;
import java.util.List;
import java.util.Optional;

/**
 * 非默认yaml文件解析
 *
 * @author l30023229
 * @since 2023-08-09
 */
public class YamlPropertiesSourceFactory implements PropertySourceFactory {
    @Override
    public PropertySource<?> createPropertySource(String name, EncodedResource resource) throws IOException {
        String resourceName = Optional.ofNullable(name).orElse(resource.getResource().getFilename());
        List<PropertySource<?>> yamlSources = new YamlPropertySourceLoader().load(resourceName, resource.getResource());
        return yamlSources.get(0);
    }
}
