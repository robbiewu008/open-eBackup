package openbackup.system.base.common.utils;

import com.baomidou.mybatisplus.core.handlers.MetaObjectHandler;

import lombok.extern.slf4j.Slf4j;

import org.apache.ibatis.reflection.MetaObject;
import org.springframework.stereotype.Component;

import java.sql.Timestamp;

/**
 * MyBatis自动增加创建时间和更新时间货站
 *
 * @author h30003246
 * @since 2020-09-17
 */
@Slf4j
@Component
public class MybatisPlusAutoUpdateTimeUtil implements MetaObjectHandler {
    @Override
    public void insertFill(MetaObject metaObject) {
        log.debug("come to insert fill.");
        this.setFieldValByName("createdTime", new Timestamp(System.currentTimeMillis()), metaObject);
        this.setFieldValByName("updatedTime", new Timestamp(System.currentTimeMillis()), metaObject);
    }

    @Override
    public void updateFill(MetaObject metaObject) {
        log.debug("come to update fill.");
        this.setFieldValByName("updatedTime", new Timestamp(System.currentTimeMillis()), metaObject);
    }
}
