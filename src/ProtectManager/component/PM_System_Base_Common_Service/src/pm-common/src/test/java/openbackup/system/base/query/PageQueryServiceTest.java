package openbackup.system.base.query;

import static org.mockito.ArgumentMatchers.any;

import openbackup.system.base.common.annotation.DbMangerMapper;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.sdk.copy.model.BasePage;

import com.baomidou.mybatisplus.annotation.FieldFill;
import com.baomidou.mybatisplus.annotation.IdType;
import com.baomidou.mybatisplus.annotation.TableField;
import com.baomidou.mybatisplus.annotation.TableId;
import com.baomidou.mybatisplus.annotation.TableName;
import com.baomidou.mybatisplus.core.conditions.Wrapper;
import com.baomidou.mybatisplus.core.mapper.BaseMapper;
import com.baomidou.mybatisplus.core.metadata.IPage;

import lombok.Data;

import org.apache.ibatis.annotations.Param;
import org.apache.ibatis.annotations.Select;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnitRunner;
import org.powermock.api.mockito.PowerMockito;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.autoconfigure.sql.init.SqlInitializationAutoConfiguration;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;

import java.sql.Timestamp;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Map;

/**
 * Page Query Service Test
 *
 * @author twx1009756
 * @since 2021-03-17
 */
@RunWith(MockitoJUnitRunner.class)
@SpringBootTest(classes = {PageQueryService.class, PageQueryServiceTest.LMDao.class, PageQueryServiceTest.LM.class, SqlInitializationAutoConfiguration.class})
@MockBean(SessionService.class)
public class PageQueryServiceTest {
    @Autowired
    @InjectMocks
    PageQueryService pageQueryService;

    @Mock
    @Autowired
    LMDao lMDao;

    @Mock
    @Autowired
    SessionService sessionService;

    @Test
    public void testSplitConditions() {
        List<Map.Entry<String, List<?>>> result0 =
                PageQueryService.splitConditions(
                        new JSONArray(
                                Arrays.asList(Collections.singletonList(">"), 20, Collections.singletonList("<"), 30)));
        Assert.assertEquals(2, result0.size());
        List<Map.Entry<String, List<?>>> result1 =
                PageQueryService.splitConditions(
                        new JSONArray(
                                Arrays.asList(
                                        Collections.singletonList(">"),
                                        20,
                                        Collections.singletonList("<"),
                                        30,
                                        Collections.singletonList("in"),
                                        1,
                                        2,
                                        3)));
        Assert.assertEquals(3, result1.size());
        List<Map.Entry<String, List<?>>> result2 =
                PageQueryService.splitConditions(
                        new JSONArray(
                                Arrays.asList(
                                        1,
                                        2,
                                        3,
                                        Collections.singletonList(">"),
                                        20,
                                        Collections.singletonList("<"),
                                        30)));
        Assert.assertEquals(3, result2.size());
    }

    /**
     * 测试PageQuery方法
     */
    @Test
    public void testPageQueries() {
        @SuppressWarnings("unchecked")
        IPage<LM> pageObject = PowerMockito.mock(IPage.class);
        PowerMockito.when(lMDao.page(any(), any())).thenReturn(pageObject);
        int page = 3;
        int size = 10;
        String conditions = "{}";
        List<String> orders = new ArrayList<>();
        PageQueryParam pageQueryParam = new PageQueryParam(page, size, conditions, orders);

        BasePage<LM> data0 =
                pageQueryService.pageQuery(LM.class, lMDao::page, pageQueryParam, "-created_time", "user_id");
        Assert.assertNotNull(data0);

        BasePage<LM> data1 =
                pageQueryService.pageQuery(
                        LM.class, lMDao::page, pageQueryParam, "-created_time,-live_mount_count", "user_id");
        Assert.assertNotNull(data1);
    }

    @DbMangerMapper
    interface LMDao extends BaseMapper<LM> {
        /**
         * JOIN_SQL
         */
        String JOIN_SQL =
                "select a.*, case when c.total is null then 0 else c.total end as live_mount_count "
                        + "from live_mount_policy as a"
                        + " left join (select policy_id, count(id) as total "
                        + " from LIVE_MOUNT group by policy_id) c on c.policy_id = a.policy_id";

        /**
         * WRAP_SQL
         */
        String WRAP_SQL = "SELECT * from ( " + JOIN_SQL + " ) AS q ${ew.customSqlSegment}";

        /**
         * page query
         *
         * @param page page
         * @param queryWrapper query wrapper
         * @return page
         */
        @Select(WRAP_SQL)
        IPage<LM> page(IPage<LM> page, @Param("ew") Wrapper<LM> queryWrapper);
    }

    @Data
    @PageQueryConfig(
            conditions = {"%name%"},
            orders = {"live_mount_count", "created_time"})
    @TableName(value = "live_mount_policy")
    static class LM {
        @TableId(type = IdType.INPUT)
        private String policyId;

        private String name;

        private String copyDataSelectionPolicy;

        private String retentionPolicy;

        private String retentionUnit;

        private Integer retentionValue;

        private String schedulePolicy;

        private Integer scheduleInterval;

        private String scheduleIntervalUnit;

        private Timestamp scheduleStartTime;

        @TableField(fill = FieldFill.INSERT)
        private Timestamp createdTime;

        @TableField(fill = FieldFill.INSERT_UPDATE)
        private Timestamp updatedTime;

        private String latestCopyFor;

        private String afterCopyGenerated;

        @TableField(exist = false)
        private int liveMountCount;

        private String userId;
    }
}
