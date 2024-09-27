package openbackup.system.base.common.factory;

import openbackup.system.base.common.factory.YamlPropertiesSourceFactory;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.core.io.ClassPathResource;
import org.springframework.core.io.support.EncodedResource;

import java.io.IOException;

/**
 * 功能描述
 *
 * @author l30023229
 * @since 2023-08-10
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {YamlPropertiesSourceFactory.class})
public class YamlPropertiesSourceFactoryTest {
    @InjectMocks
    private YamlPropertiesSourceFactory yamlPropertiesSourceFactory;

    @Test
    public void createPropertySourceTest() throws IOException {
        ClassPathResource resource = new ClassPathResource("/conf/alarmI18nE/AlarmCommonEn.json");
        EncodedResource encodedResource = new EncodedResource(resource);
        yamlPropertiesSourceFactory.createPropertySource("AlarmCommonEn.json", encodedResource);
    }
}
