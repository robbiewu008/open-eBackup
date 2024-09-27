package openbackup.system.base.config;

import com.baomidou.mybatisplus.annotation.DbType;
import com.baomidou.mybatisplus.extension.plugins.MybatisPlusInterceptor;
import com.baomidou.mybatisplus.extension.plugins.inner.PaginationInnerInterceptor;

import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

/**
 * MyBatis Plus相关的配置
 *
 * @author y00280557
 * @since 2020-09-01
 */
@Configuration
class MyBatisPlusConfig {
    /**
     * mybatis-plus分页插件
     *
     * @return PaginationInterceptor
     */
    @Bean
    public MybatisPlusInterceptor paginationInterceptor() {
        MybatisPlusInterceptor mybatisPlusInterceptor = new MybatisPlusInterceptor();
        mybatisPlusInterceptor.addInnerInterceptor(new PaginationInnerInterceptor(DbType.OPENGAUSS));
        return mybatisPlusInterceptor;
    }
}
