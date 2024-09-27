package openbackup.data.access.framework.livemount.service;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.access.framework.livemount.common.enums.RetentionType;
import openbackup.data.access.framework.livemount.common.enums.RetentionUnit;
import openbackup.data.access.framework.livemount.common.enums.ScheduledType;
import openbackup.data.access.framework.livemount.common.enums.ScheduledUnit;
import openbackup.data.access.framework.livemount.controller.policy.request.UpdatePolicyRequest;
import openbackup.data.access.framework.livemount.controller.policy.response.LiveMountPolicyVo;
import openbackup.data.access.framework.livemount.dao.LiveMountEntityDao;
import openbackup.data.access.framework.livemount.dao.LiveMountPolicyEntityDao;
import openbackup.data.access.framework.livemount.data.PolicyServiceImplTestData;
import openbackup.data.access.framework.livemount.entity.LiveMountPolicyEntity;
import openbackup.data.access.framework.livemount.service.impl.PolicyServiceImpl;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.query.PageQueryService;
import openbackup.system.base.sdk.copy.model.BasePage;
import com.huawei.oceanprotect.system.base.user.service.ResourceSetApi;

import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.ContextConfiguration;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 * live mount policy test impl
 *
 * @author h30003246
 * @since 2021-03-02
 */
@RunWith(SpringRunner.class)
@SpringBootTest
@ContextConfiguration(classes = {PolicyServiceImpl.class})
public class PolicyServiceImplTest {
    /**
     * ExpectedException
     */
    @Rule
    public final ExpectedException expectedException = ExpectedException.none();

    @Autowired
    private PolicyServiceImpl policyService;

    @MockBean
    private LiveMountPolicyEntityDao liveMountPolicyEntityDao;

    @MockBean
    private LiveMountEntityDao liveMountEntityDao;

    @MockBean
    private PageQueryService pageQueryService;

    @MockBean
    private LiveMountService liveMountService;

    @MockBean
    private ResourceSetApi resourceSetApi;

    /**
     * test create policy count more than MAX_POLICY_NUM
     */
    @Test
    public void createPolicyCountMoreThanMaxPolicyNum() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("policy count reach limit 128.");
        PowerMockito.when(liveMountPolicyEntityDao.selectCount(any()))
                .thenReturn(Long.valueOf(IsmNumberConstant.HUNDRED_TWENTY_EIGHT));
        policyService.createPolicy(PolicyServiceImplTestData.getCreateRequest(),
                PolicyServiceImplTestData.getToken().getUser());
    }

    /**
     * test create policy entity is not null
     */
    @Test
    public void createPolicyEntityIsNotNull() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("policy name is duplicate");
        PowerMockito.when(liveMountPolicyEntityDao.selectOne(any()))
                .thenReturn(PolicyServiceImplTestData.getLiveMountPolicyEntity());
        policyService.createPolicy(PolicyServiceImplTestData.getCreateRequest(),
                PolicyServiceImplTestData.getToken().getUser());
    }

    /**
     * test create policy
     */
    @Test
    public void createPolicy() {
        PowerMockito.when(liveMountEntityDao.selectOne(any())).thenReturn(null);
        PowerMockito.when(liveMountEntityDao.selectCount(any())).thenReturn(0L);
        PowerMockito.when(liveMountEntityDao.insert(any())).thenReturn(1);
        policyService.createPolicy(PolicyServiceImplTestData.getCreateRequest(),
                PolicyServiceImplTestData.getToken().getUser());
        Assert.assertNotNull(policyService);
    }

    /**
     * test get policies
     */
    @Test
    public void getPolices() {
        LiveMountPolicyEntity liveMountPolicyEntityParam = PolicyServiceImplTestData.getLiveMountPolicyEntity();
        BasePage<Object> basePageParam = new BasePage<>();
        basePageParam.setItems(Arrays.asList(liveMountPolicyEntityParam));
        PowerMockito.when(pageQueryService.pageQuery(any(), any(), any(), any(), any()))
                .thenReturn(basePageParam);
        Object obj = policyService.getPolices(0, 20, "", Arrays.asList("-created_time"));
        if (obj instanceof BasePage) {
            BasePage<Object> basePage = (BasePage<Object>) obj;
            Object liveMountPolicyEntityObj = basePage.getItems().get(0);
            if (liveMountPolicyEntityObj instanceof LiveMountPolicyEntity) {
                assertLiveMountPolicyEntity(liveMountPolicyEntityParam,
                    (LiveMountPolicyEntity) liveMountPolicyEntityObj);
            }
        }
    }

    /**
     * test get policy entity is null
     */
    @Test
    public void getPolicyEntityIsNull() {
        expectedException.expect(LegoCheckedException.class);
        policyService.getPolicy("1");
    }

    /**
     * test get policy
     */
    @Test
    public void getPolicy() {
        LiveMountPolicyEntity liveMountPolicyEntity = PolicyServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectPolicy(liveMountPolicyEntity.getPolicyId()))
                .thenReturn(liveMountPolicyEntity);
        LiveMountPolicyVo liveMountPolicyVo = policyService.getPolicy(liveMountPolicyEntity.getPolicyId());
        assert liveMountPolicyVo.getName().equals(liveMountPolicyEntity.getName());
        assert liveMountPolicyVo.getSchedulePolicy().equals(liveMountPolicyEntity.getSchedulePolicy());
        assert liveMountPolicyVo.getRetentionPolicy().equals(liveMountPolicyEntity.getRetentionPolicy());
        assert liveMountPolicyVo.getPolicyId().equals(liveMountPolicyEntity.getPolicyId());
    }

    /**
     * test update policy policy is not exist
     */
    @Test
    public void updatePolicyPolicyIsNotExist() {
        LiveMountPolicyEntity liveMountPolicyEntity = PolicyServiceImplTestData.getLiveMountPolicyEntity();
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("policy is not exist. policy id is " +
                liveMountPolicyEntity.getPolicyId());
        policyService.updatePolicy(liveMountPolicyEntity.getPolicyId(),
                PolicyServiceImplTestData.getUpdatePolicyRequest());
    }

    /**
     * test update policy name is null
     */
    @Test
    public void updatePolicyNameIsNull() {
        LiveMountPolicyEntity liveMountPolicyEntity = PolicyServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectList(any())).thenReturn(null);
        PowerMockito.when(liveMountPolicyEntityDao.updateById(any())).thenReturn(1);
        PowerMockito.when(liveMountService.queryLiveMountEntitiesByPolicyId(any())).thenReturn(new ArrayList<>());
        PowerMockito.when(liveMountPolicyEntityDao.selectById(any())).thenReturn(liveMountPolicyEntity);
        UpdatePolicyRequest updatePolicyRequest = PolicyServiceImplTestData.getUpdatePolicyRequest();
        updatePolicyRequest.setName(null);
        policyService.updatePolicy(liveMountPolicyEntity.getPolicyId(), updatePolicyRequest);
        Mockito.verify(liveMountPolicyEntityDao, Mockito.times(1)).selectList(any());
    }

    /**
     * test update policy name is not null but validation fail
     */
    @Test
    public void updatePolicyNameValidateFail() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("The name validation failed, The correct format is: "
                + "The value is a string of 1 to 64 characters "
                + "and can contain only digits, letters, underscores (_), and hyphens (-). "
                + "It must start with a letter, Chinese character, or underscore (_).");
        LiveMountPolicyEntity liveMountPolicyEntity = PolicyServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectList(any())).thenReturn(null);
        PowerMockito.when(liveMountPolicyEntityDao.updateById(any())).thenReturn(1);
        PowerMockito.when(liveMountService.queryLiveMountEntitiesByPolicyId(any())).thenReturn(new ArrayList<>());
        PowerMockito.when(liveMountPolicyEntityDao.selectById(any())).thenReturn(liveMountPolicyEntity);
        UpdatePolicyRequest updatePolicyRequest = PolicyServiceImplTestData.getUpdatePolicyRequest();
        updatePolicyRequest.setName("!//&");
        policyService.updatePolicy(liveMountPolicyEntity.getPolicyId(), updatePolicyRequest);
    }

    /**
     * test update policy historyPolicy is not null
     */
    @Test
    public void updatePolicyHistoryPolicyIsNotNull() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("policy name is duplicate.");
        LiveMountPolicyEntity param = PolicyServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectList(any())).thenReturn(null);
        PowerMockito.when(liveMountPolicyEntityDao.updateById(any())).thenReturn(1);
        PowerMockito.when(liveMountService.queryLiveMountEntitiesByPolicyId(any())).thenReturn(new ArrayList<>());
        PowerMockito.when(liveMountPolicyEntityDao.selectById(any())).thenReturn(param);

        List<LiveMountPolicyEntity> historyPolicy = new ArrayList<>();
        LiveMountPolicyEntity liveMountPolicyEntity = new LiveMountPolicyEntity();
        liveMountPolicyEntity.setPolicyId(param.getPolicyId() + "!");
        historyPolicy.add(liveMountPolicyEntity);
        PowerMockito.when(liveMountPolicyEntityDao.selectList(any())).thenReturn(historyPolicy);
        policyService.updatePolicy(param.getPolicyId(),
                PolicyServiceImplTestData.getUpdatePolicyRequest());
    }

    /**
     * test update policy SchedulePolicy is after_backup_done
     */
    @Test
    public void updatePolicySchedulePolicyIsAfterBackupDone() {
        LiveMountPolicyEntity liveMountPolicyEntity = PolicyServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectList(any())).thenReturn(null);
        PowerMockito.when(liveMountPolicyEntityDao.updateById(any())).thenReturn(1);
        PowerMockito.when(liveMountService.queryLiveMountEntitiesByPolicyId(any())).thenReturn(new ArrayList<>());
        PowerMockito.when(liveMountPolicyEntityDao.selectById(any())).thenReturn(liveMountPolicyEntity);
        UpdatePolicyRequest updatePolicyRequest = PolicyServiceImplTestData.getUpdatePolicyRequest();
        updatePolicyRequest.setSchedulePolicy(ScheduledType.AFTER_BACKUP_DONE);
        policyService.updatePolicy(liveMountPolicyEntity.getPolicyId(), updatePolicyRequest);
        Mockito.verify(liveMountPolicyEntityDao, Mockito.times(1)).selectList(any());
    }

    /**
     * test update policy RetentionPolicy is latest_one
     */
    @Test
    public void updatePolicyRetentionPolicyIsLatestOne() {
        LiveMountPolicyEntity liveMountPolicyEntity = PolicyServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectList(any())).thenReturn(null);
        PowerMockito.when(liveMountPolicyEntityDao.updateById(any())).thenReturn(1);
        PowerMockito.when(liveMountService.queryLiveMountEntitiesByPolicyId(any())).thenReturn(new ArrayList<>());
        PowerMockito.when(liveMountPolicyEntityDao.selectById(any())).thenReturn(liveMountPolicyEntity);
        UpdatePolicyRequest updatePolicyRequest = PolicyServiceImplTestData.getUpdatePolicyRequest();
        updatePolicyRequest.setRetentionPolicy(RetentionType.LATEST_ONE);
        policyService.updatePolicy(liveMountPolicyEntity.getPolicyId(), updatePolicyRequest);
        Mockito.verify(liveMountPolicyEntityDao, Mockito.times(1)).selectList(any());
    }

    /**
     * test update policy ScheduleIntervalUnit is null
     */
    @Test
    public void updatePolicyScheduleIntervalUnitIsNull() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("");
        LiveMountPolicyEntity liveMountPolicyEntity = PolicyServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectList(any())).thenReturn(null);
        PowerMockito.when(liveMountPolicyEntityDao.updateById(any())).thenReturn(1);
        PowerMockito.when(liveMountService.queryLiveMountEntitiesByPolicyId(any())).thenReturn(new ArrayList<>());
        PowerMockito.when(liveMountPolicyEntityDao.selectById(any())).thenReturn(liveMountPolicyEntity);
        UpdatePolicyRequest updatePolicyRequest = PolicyServiceImplTestData.getUpdatePolicyRequest();
        updatePolicyRequest.setScheduleIntervalUnit(null);
        policyService.updatePolicy(liveMountPolicyEntity.getPolicyId(), updatePolicyRequest);
    }

    /**
     * test update policy ScheduleInterval is null
     */
    @Test
    public void updatePolicyScheduleIntervalIsNull() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("");
        LiveMountPolicyEntity liveMountPolicyEntity = PolicyServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectList(any())).thenReturn(null);
        PowerMockito.when(liveMountPolicyEntityDao.updateById(any())).thenReturn(1);
        PowerMockito.when(liveMountService.queryLiveMountEntitiesByPolicyId(any())).thenReturn(new ArrayList<>());
        PowerMockito.when(liveMountPolicyEntityDao.selectById(any())).thenReturn(liveMountPolicyEntity);
        UpdatePolicyRequest updatePolicyRequest = PolicyServiceImplTestData.getUpdatePolicyRequest();
        updatePolicyRequest.setScheduleInterval(null);
        policyService.updatePolicy(liveMountPolicyEntity.getPolicyId(), updatePolicyRequest);
    }

    /**
     * test update policy ScheduleInterval is 0
     */
    @Test
    public void updatePolicyScheduleIntervalIsZero() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("");
        LiveMountPolicyEntity liveMountPolicyEntity = PolicyServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectList(any())).thenReturn(null);
        PowerMockito.when(liveMountPolicyEntityDao.updateById(any())).thenReturn(1);
        PowerMockito.when(liveMountService.queryLiveMountEntitiesByPolicyId(any())).thenReturn(new ArrayList<>());
        PowerMockito.when(liveMountPolicyEntityDao.selectById(any())).thenReturn(liveMountPolicyEntity);
        UpdatePolicyRequest updatePolicyRequest = PolicyServiceImplTestData.getUpdatePolicyRequest();
        updatePolicyRequest.setScheduleInterval(0);
        policyService.updatePolicy(liveMountPolicyEntity.getPolicyId(), updatePolicyRequest);
    }

    /**
     * test update policy ScheduleIntervalUnit is hour and value is in the appropriate range
     */
    @Test
    public void updatePolicyScheduleIntervalUnitIsHourAndValueInAppropriateRange() {
        LiveMountPolicyEntity liveMountPolicyEntity = PolicyServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectList(any())).thenReturn(null);
        PowerMockito.when(liveMountPolicyEntityDao.updateById(any())).thenReturn(1);
        PowerMockito.when(liveMountService.queryLiveMountEntitiesByPolicyId(any())).thenReturn(new ArrayList<>());
        PowerMockito.when(liveMountPolicyEntityDao.selectById(any())).thenReturn(liveMountPolicyEntity);
        UpdatePolicyRequest updatePolicyRequest = PolicyServiceImplTestData.getUpdatePolicyRequest();
        updatePolicyRequest.setScheduleIntervalUnit(ScheduledUnit.HOUR);
        updatePolicyRequest.setScheduleInterval(22);
        policyService.updatePolicy(liveMountPolicyEntity.getPolicyId(), updatePolicyRequest);
        Mockito.verify(liveMountPolicyEntityDao, Mockito.times(1)).selectList(any());
    }

    /**
     * test update policy ScheduleIntervalUnit is hour and value is in the appropriate range
     */
    @Test
    public void updatePolicyScheInteUnitIsHourAndValueOutAppropriateRange() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("");
        LiveMountPolicyEntity liveMountPolicyEntity = PolicyServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectList(any())).thenReturn(null);
        PowerMockito.when(liveMountPolicyEntityDao.updateById(any())).thenReturn(1);
        PowerMockito.when(liveMountService.queryLiveMountEntitiesByPolicyId(any())).thenReturn(new ArrayList<>());
        PowerMockito.when(liveMountPolicyEntityDao.selectById(any())).thenReturn(liveMountPolicyEntity);
        UpdatePolicyRequest updatePolicyRequest = PolicyServiceImplTestData.getUpdatePolicyRequest();
        updatePolicyRequest.setScheduleIntervalUnit(ScheduledUnit.HOUR);
        updatePolicyRequest.setScheduleInterval(25);
        policyService.updatePolicy(liveMountPolicyEntity.getPolicyId(), updatePolicyRequest);
    }

    /**
     * test update policy ScheduleIntervalUnit is day and value is in the appropriate range
     */
    @Test
    public void updatePolicyScheduleIntervalUnitIsDayAndValueInAppropriateRange() {
        LiveMountPolicyEntity liveMountPolicyEntity = PolicyServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectList(any())).thenReturn(null);
        PowerMockito.when(liveMountPolicyEntityDao.updateById(any())).thenReturn(1);
        PowerMockito.when(liveMountService.queryLiveMountEntitiesByPolicyId(any())).thenReturn(new ArrayList<>());
        PowerMockito.when(liveMountPolicyEntityDao.selectById(any())).thenReturn(liveMountPolicyEntity);
        UpdatePolicyRequest updatePolicyRequest = PolicyServiceImplTestData.getUpdatePolicyRequest();
        updatePolicyRequest.setScheduleIntervalUnit(ScheduledUnit.DAY);
        updatePolicyRequest.setScheduleInterval(14);
        policyService.updatePolicy(liveMountPolicyEntity.getPolicyId(), updatePolicyRequest);
        Mockito.verify(liveMountPolicyEntityDao, Mockito.times(1)).selectList(any());
    }

    /**
     * test update policy ScheduleIntervalUnit is day and value is in the appropriate range
     */
    @Test
    public void updatePolicyScheduleIntervalUnitIsDayAndValueOutAppropriateRange() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("");
        LiveMountPolicyEntity liveMountPolicyEntity = PolicyServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectList(any())).thenReturn(null);
        PowerMockito.when(liveMountPolicyEntityDao.updateById(any())).thenReturn(1);
        PowerMockito.when(liveMountService.queryLiveMountEntitiesByPolicyId(any())).thenReturn(new ArrayList<>());
        PowerMockito.when(liveMountPolicyEntityDao.selectById(any())).thenReturn(liveMountPolicyEntity);
        UpdatePolicyRequest updatePolicyRequest = PolicyServiceImplTestData.getUpdatePolicyRequest();
        updatePolicyRequest.setScheduleIntervalUnit(ScheduledUnit.DAY);
        updatePolicyRequest.setScheduleInterval(31);
        policyService.updatePolicy(liveMountPolicyEntity.getPolicyId(), updatePolicyRequest);
    }

    /**
     * test update policy ScheduleIntervalUnit is week and value is in the appropriate range
     */
    @Test
    public void updatePolicyScheduleIntervalUnitIsWeekAndValueInAppropriateRange() {
        LiveMountPolicyEntity liveMountPolicyEntity = PolicyServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectList(any())).thenReturn(null);
        PowerMockito.when(liveMountPolicyEntityDao.updateById(any())).thenReturn(1);
        PowerMockito.when(liveMountService.queryLiveMountEntitiesByPolicyId(any())).thenReturn(new ArrayList<>());
        PowerMockito.when(liveMountPolicyEntityDao.selectById(any())).thenReturn(liveMountPolicyEntity);
        UpdatePolicyRequest updatePolicyRequest = PolicyServiceImplTestData.getUpdatePolicyRequest();
        updatePolicyRequest.setScheduleIntervalUnit(ScheduledUnit.WEEK);
        updatePolicyRequest.setScheduleInterval(2);
        policyService.updatePolicy(liveMountPolicyEntity.getPolicyId(), updatePolicyRequest);
        Mockito.verify(liveMountPolicyEntityDao, Mockito.times(1)).selectList(any());
    }

    /**
     * test update policy ScheduleIntervalUnit is week and value is in the appropriate range
     */
    @Test
    public void updatePolicyScheInteUnitIsWeekAndValueOutAppropriateRange() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("");
        LiveMountPolicyEntity liveMountPolicyEntity = PolicyServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectList(any())).thenReturn(null);
        PowerMockito.when(liveMountPolicyEntityDao.updateById(any())).thenReturn(1);
        PowerMockito.when(liveMountService.queryLiveMountEntitiesByPolicyId(any())).thenReturn(new ArrayList<>());
        PowerMockito.when(liveMountPolicyEntityDao.selectById(any())).thenReturn(liveMountPolicyEntity);
        UpdatePolicyRequest updatePolicyRequest = PolicyServiceImplTestData.getUpdatePolicyRequest();
        updatePolicyRequest.setScheduleIntervalUnit(ScheduledUnit.WEEK);
        updatePolicyRequest.setScheduleInterval(5);
        policyService.updatePolicy(liveMountPolicyEntity.getPolicyId(), updatePolicyRequest);
    }

    /**
     * test update policy Timestamp is null
     */
    @Test
    public void updatePolicyTimestampIsNull() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("");
        LiveMountPolicyEntity liveMountPolicyEntity = PolicyServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectList(any())).thenReturn(null);
        PowerMockito.when(liveMountPolicyEntityDao.updateById(any())).thenReturn(1);
        PowerMockito.when(liveMountService.queryLiveMountEntitiesByPolicyId(any())).thenReturn(new ArrayList<>());
        PowerMockito.when(liveMountPolicyEntityDao.selectById(any())).thenReturn(liveMountPolicyEntity);
        UpdatePolicyRequest updatePolicyRequest = PolicyServiceImplTestData.getUpdatePolicyRequest();
        updatePolicyRequest.setScheduleStartTime(null);
        policyService.updatePolicy(liveMountPolicyEntity.getPolicyId(), updatePolicyRequest);
    }

    /**
     * test update policy Timestamp is out appropriate range
     */
    @Test
    public void updatePolicyTimestampIsOutaAppropriateRange() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("");
        LiveMountPolicyEntity liveMountPolicyEntity = PolicyServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectList(any())).thenReturn(null);
        PowerMockito.when(liveMountPolicyEntityDao.updateById(any())).thenReturn(1);
        PowerMockito.when(liveMountService.queryLiveMountEntitiesByPolicyId(any())).thenReturn(new ArrayList<>());
        PowerMockito.when(liveMountPolicyEntityDao.selectById(any())).thenReturn(liveMountPolicyEntity);
        UpdatePolicyRequest updatePolicyRequest = PolicyServiceImplTestData.getUpdatePolicyRequest();
        updatePolicyRequest.setScheduleStartTime("a123456");
        policyService.updatePolicy(liveMountPolicyEntity.getPolicyId(), updatePolicyRequest);
    }

    /**
     * test update policy RetentionUnit is null
     */
    @Test
    public void updatePolicyRetentionUnitIsNull() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("");
        LiveMountPolicyEntity liveMountPolicyEntity = PolicyServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectList(any())).thenReturn(null);
        PowerMockito.when(liveMountPolicyEntityDao.updateById(any())).thenReturn(1);
        PowerMockito.when(liveMountService.queryLiveMountEntitiesByPolicyId(any())).thenReturn(new ArrayList<>());
        PowerMockito.when(liveMountPolicyEntityDao.selectById(any())).thenReturn(liveMountPolicyEntity);
        UpdatePolicyRequest updatePolicyRequest = PolicyServiceImplTestData.getUpdatePolicyRequest();
        updatePolicyRequest.setRetentionUnit(null);
        policyService.updatePolicy(liveMountPolicyEntity.getPolicyId(), updatePolicyRequest);
    }

    /**
     * test update policy RetentionValue is null
     */
    @Test
    public void updatePolicyRetentionValueIsNull() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("");
        LiveMountPolicyEntity liveMountPolicyEntity = PolicyServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectList(any())).thenReturn(null);
        PowerMockito.when(liveMountPolicyEntityDao.updateById(any())).thenReturn(1);
        PowerMockito.when(liveMountService.queryLiveMountEntitiesByPolicyId(any())).thenReturn(new ArrayList<>());
        PowerMockito.when(liveMountPolicyEntityDao.selectById(any())).thenReturn(liveMountPolicyEntity);
        UpdatePolicyRequest updatePolicyRequest = PolicyServiceImplTestData.getUpdatePolicyRequest();
        updatePolicyRequest.setRetentionValue(null);
        policyService.updatePolicy(liveMountPolicyEntity.getPolicyId(), updatePolicyRequest);
    }

    /**
     * test update policy RetentionValue is 0
     */
    @Test
    public void updatePolicyRetentionValueIsZero() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("");
        LiveMountPolicyEntity liveMountPolicyEntity = PolicyServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectList(any())).thenReturn(null);
        PowerMockito.when(liveMountPolicyEntityDao.updateById(any())).thenReturn(1);
        PowerMockito.when(liveMountService.queryLiveMountEntitiesByPolicyId(any())).thenReturn(new ArrayList<>());
        PowerMockito.when(liveMountPolicyEntityDao.selectById(any())).thenReturn(liveMountPolicyEntity);
        UpdatePolicyRequest updatePolicyRequest = PolicyServiceImplTestData.getUpdatePolicyRequest();
        updatePolicyRequest.setRetentionValue(0);
        policyService.updatePolicy(liveMountPolicyEntity.getPolicyId(), updatePolicyRequest);
    }

    /**
     * test update policy RetentionUnit is day and value is in the appropriate range
     */
    @Test
    public void updatePolicyRetentionUnitIsDayAndValueInAppropriateRange() {
        LiveMountPolicyEntity liveMountPolicyEntity = PolicyServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectList(any())).thenReturn(null);
        PowerMockito.when(liveMountPolicyEntityDao.updateById(any())).thenReturn(1);
        PowerMockito.when(liveMountService.queryLiveMountEntitiesByPolicyId(any())).thenReturn(new ArrayList<>());
        PowerMockito.when(liveMountPolicyEntityDao.selectById(any())).thenReturn(liveMountPolicyEntity);
        UpdatePolicyRequest updatePolicyRequest = PolicyServiceImplTestData.getUpdatePolicyRequest();
        updatePolicyRequest.setRetentionUnit(RetentionUnit.DAY);
        updatePolicyRequest.setRetentionValue(300);
        policyService.updatePolicy(liveMountPolicyEntity.getPolicyId(), updatePolicyRequest);
        Mockito.verify(liveMountPolicyEntityDao, Mockito.times(1)).selectList(any());
    }

    /**
     * test update policy RetentionUnit is day and value is in the appropriate range
     */
    @Test
    public void updatePolicyRetentionUnitIsDayAndValueOutAppropriateRange() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("");
        LiveMountPolicyEntity liveMountPolicyEntity = PolicyServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectList(any())).thenReturn(null);
        PowerMockito.when(liveMountPolicyEntityDao.updateById(any())).thenReturn(1);
        PowerMockito.when(liveMountService.queryLiveMountEntitiesByPolicyId(any())).thenReturn(new ArrayList<>());
        PowerMockito.when(liveMountPolicyEntityDao.selectById(any())).thenReturn(liveMountPolicyEntity);
        UpdatePolicyRequest updatePolicyRequest = PolicyServiceImplTestData.getUpdatePolicyRequest();
        updatePolicyRequest.setRetentionUnit(RetentionUnit.DAY);
        updatePolicyRequest.setRetentionValue(367);
        policyService.updatePolicy(liveMountPolicyEntity.getPolicyId(), updatePolicyRequest);
    }

    /**
     * test update policy RetentionUnit is week and value is in the appropriate range
     */
    @Test
    public void updatePolicyRetentionUnitIsWeekAndValueInAppropriateRange() {
        LiveMountPolicyEntity liveMountPolicyEntity = PolicyServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectList(any())).thenReturn(null);
        PowerMockito.when(liveMountPolicyEntityDao.updateById(any())).thenReturn(1);
        PowerMockito.when(liveMountService.queryLiveMountEntitiesByPolicyId(any())).thenReturn(new ArrayList<>());
        PowerMockito.when(liveMountPolicyEntityDao.selectById(any())).thenReturn(liveMountPolicyEntity);
        UpdatePolicyRequest updatePolicyRequest = PolicyServiceImplTestData.getUpdatePolicyRequest();
        updatePolicyRequest.setRetentionUnit(RetentionUnit.WEEK);
        updatePolicyRequest.setRetentionValue(33);
        policyService.updatePolicy(liveMountPolicyEntity.getPolicyId(), updatePolicyRequest);
        Mockito.verify(liveMountPolicyEntityDao, Mockito.times(1)).selectList(any());
    }

    /**
     * test update policy RetentionUnit is week and value is in the appropriate range
     */
    @Test
    public void updatePolicyRetentionUnitIsWeekAndValueOutAppropriateRange() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("");
        LiveMountPolicyEntity liveMountPolicyEntity = PolicyServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectList(any())).thenReturn(null);
        PowerMockito.when(liveMountPolicyEntityDao.updateById(any())).thenReturn(1);
        PowerMockito.when(liveMountService.queryLiveMountEntitiesByPolicyId(any())).thenReturn(new ArrayList<>());
        PowerMockito.when(liveMountPolicyEntityDao.selectById(any())).thenReturn(liveMountPolicyEntity);
        UpdatePolicyRequest updatePolicyRequest = PolicyServiceImplTestData.getUpdatePolicyRequest();
        updatePolicyRequest.setRetentionUnit(RetentionUnit.WEEK);
        updatePolicyRequest.setRetentionValue(66);
        policyService.updatePolicy(liveMountPolicyEntity.getPolicyId(), updatePolicyRequest);
    }

    /**
     * test update policy RetentionUnit is month and value is in the appropriate range
     */
    @Test
    public void updatePolicyRetentionUnitIsMonthAndValueInAppropriateRange() {
        LiveMountPolicyEntity liveMountPolicyEntity = PolicyServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectList(any())).thenReturn(null);
        PowerMockito.when(liveMountPolicyEntityDao.updateById(any())).thenReturn(1);
        PowerMockito.when(liveMountService.queryLiveMountEntitiesByPolicyId(any())).thenReturn(new ArrayList<>());
        PowerMockito.when(liveMountPolicyEntityDao.selectById(any())).thenReturn(liveMountPolicyEntity);
        UpdatePolicyRequest updatePolicyRequest = PolicyServiceImplTestData.getUpdatePolicyRequest();
        updatePolicyRequest.setRetentionUnit(RetentionUnit.MONTH);
        updatePolicyRequest.setRetentionValue(2);
        policyService.updatePolicy(liveMountPolicyEntity.getPolicyId(), updatePolicyRequest);
        Mockito.verify(liveMountPolicyEntityDao, Mockito.times(1)).selectList(any());
    }

    /**
     * test update policy RetentionUnit is month and value is in the appropriate range
     */
    @Test
    public void updatePolicyRetentionUnitIsMonthAndValueOutAppropriateRange() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("");
        LiveMountPolicyEntity liveMountPolicyEntity = PolicyServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectList(any())).thenReturn(null);
        PowerMockito.when(liveMountPolicyEntityDao.updateById(any())).thenReturn(1);
        PowerMockito.when(liveMountService.queryLiveMountEntitiesByPolicyId(any())).thenReturn(new ArrayList<>());
        PowerMockito.when(liveMountPolicyEntityDao.selectById(any())).thenReturn(liveMountPolicyEntity);
        UpdatePolicyRequest updatePolicyRequest = PolicyServiceImplTestData.getUpdatePolicyRequest();
        updatePolicyRequest.setRetentionUnit(RetentionUnit.MONTH);
        updatePolicyRequest.setRetentionValue(35);
        policyService.updatePolicy(liveMountPolicyEntity.getPolicyId(), updatePolicyRequest);
    }

    /**
     * test update policy RetentionUnit is year and value is in the appropriate range
     */
    @Test
    public void updatePolicyRetentionUnitIsYearAndValueInAppropriateRange() {
        LiveMountPolicyEntity liveMountPolicyEntity = PolicyServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectList(any())).thenReturn(null);
        PowerMockito.when(liveMountPolicyEntityDao.updateById(any())).thenReturn(1);
        PowerMockito.when(liveMountService.queryLiveMountEntitiesByPolicyId(any())).thenReturn(new ArrayList<>());
        PowerMockito.when(liveMountPolicyEntityDao.selectById(any())).thenReturn(liveMountPolicyEntity);
        UpdatePolicyRequest updatePolicyRequest = PolicyServiceImplTestData.getUpdatePolicyRequest();
        updatePolicyRequest.setRetentionUnit(RetentionUnit.YEAR);
        updatePolicyRequest.setRetentionValue(2);
        policyService.updatePolicy(liveMountPolicyEntity.getPolicyId(), updatePolicyRequest);
        Mockito.verify(liveMountPolicyEntityDao, Mockito.times(1)).selectList(any());
    }

    /**
     * test update policy RetentionUnit is year and value is in the appropriate range
     */
    @Test
    public void updatePolicyRetentionUnitIsYearAndValueOutAppropriateRange() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("");
        LiveMountPolicyEntity liveMountPolicyEntity = PolicyServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectList(any())).thenReturn(null);
        PowerMockito.when(liveMountPolicyEntityDao.updateById(any())).thenReturn(1);
        PowerMockito.when(liveMountService.queryLiveMountEntitiesByPolicyId(any())).thenReturn(new ArrayList<>());
        PowerMockito.when(liveMountPolicyEntityDao.selectById(any())).thenReturn(liveMountPolicyEntity);
        UpdatePolicyRequest updatePolicyRequest = PolicyServiceImplTestData.getUpdatePolicyRequest();
        updatePolicyRequest.setRetentionUnit(RetentionUnit.YEAR);
        updatePolicyRequest.setRetentionValue(12);
        policyService.updatePolicy(liveMountPolicyEntity.getPolicyId(), updatePolicyRequest);
    }

    /**
     * test update policy SchedulePolicy is after_backup_done
     */
    @Test
    public void updatePolicySchedulePolicyIsAfterBackUpDone() {
        LiveMountPolicyEntity liveMountPolicyEntity = PolicyServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectList(any())).thenReturn(null);
        PowerMockito.when(liveMountPolicyEntityDao.updateById(any())).thenReturn(1);
        PowerMockito.when(liveMountService.queryLiveMountEntitiesByPolicyId(any())).thenReturn(new ArrayList<>());
        PowerMockito.when(liveMountPolicyEntityDao.selectById(any())).thenReturn(liveMountPolicyEntity);
        UpdatePolicyRequest updatePolicyRequest = PolicyServiceImplTestData.getUpdatePolicyRequest();
        updatePolicyRequest.setSchedulePolicy(ScheduledType.AFTER_BACKUP_DONE);
        policyService.updatePolicy(liveMountPolicyEntity.getPolicyId(), updatePolicyRequest);
        Mockito.verify(liveMountPolicyEntityDao, Mockito.times(1)).selectList(any());
    }

    /**
     * test update policy LiveMountPolicyEntity and UpdatePolicyRequest ScheduleStartTime is not same
     */
    @Test
    public void updatePolicyScheduleStartTimeIsNotSame() {
        LiveMountPolicyEntity liveMountPolicyEntity = PolicyServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectList(any())).thenReturn(null);
        PowerMockito.when(liveMountPolicyEntityDao.updateById(any())).thenReturn(1);
        PowerMockito.when(liveMountService.queryLiveMountEntitiesByPolicyId(any())).thenReturn(new ArrayList<>());
        PowerMockito.when(liveMountPolicyEntityDao.selectById(any())).thenReturn(liveMountPolicyEntity);
        UpdatePolicyRequest updatePolicyRequest = PolicyServiceImplTestData.getUpdatePolicyRequest();
        updatePolicyRequest.setScheduleStartTime(liveMountPolicyEntity.getScheduleStartTime().toString() + "1");
        updatePolicyRequest.setScheduleInterval(liveMountPolicyEntity.getScheduleInterval());
        updatePolicyRequest.setScheduleIntervalUnit(ScheduledUnit.HOUR);
        liveMountPolicyEntity.setScheduleIntervalUnit("h");
        policyService.updatePolicy(liveMountPolicyEntity.getPolicyId(), updatePolicyRequest);
        Mockito.verify(liveMountPolicyEntityDao, Mockito.times(1)).updateById(any());
    }

    /**
     * test update policy LiveMountPolicyEntity and UpdatePolicyRequest ScheduleInterval is not same
     */
    @Test
    public void updatePolicyScheduleIntervalIsNotSame() {
        LiveMountPolicyEntity liveMountPolicyEntity = PolicyServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectList(any())).thenReturn(null);
        PowerMockito.when(liveMountPolicyEntityDao.updateById(any())).thenReturn(1);
        PowerMockito.when(liveMountService.queryLiveMountEntitiesByPolicyId(any())).thenReturn(new ArrayList<>());
        PowerMockito.when(liveMountPolicyEntityDao.selectById(any())).thenReturn(liveMountPolicyEntity);
        UpdatePolicyRequest updatePolicyRequest = PolicyServiceImplTestData.getUpdatePolicyRequest();
        updatePolicyRequest.setScheduleStartTime(liveMountPolicyEntity.getScheduleStartTime().toString());
        updatePolicyRequest.setScheduleInterval(liveMountPolicyEntity.getScheduleInterval() + 1);
        updatePolicyRequest.setScheduleIntervalUnit(ScheduledUnit.HOUR);
        liveMountPolicyEntity.setScheduleIntervalUnit("h");
        policyService.updatePolicy(liveMountPolicyEntity.getPolicyId(), updatePolicyRequest);
        Mockito.verify(liveMountPolicyEntityDao, Mockito.times(1)).updateById(any());
    }

    /**
     * test update policy LiveMountPolicyEntity and UpdatePolicyRequest ScheduleIntervalUnit is not same
     */
    @Test
    public void updatePolicyScheduleIntervalUnitIsNotSame() {
        LiveMountPolicyEntity liveMountPolicyEntity = PolicyServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectList(any())).thenReturn(null);
        PowerMockito.when(liveMountPolicyEntityDao.updateById(any())).thenReturn(1);
        PowerMockito.when(liveMountService.queryLiveMountEntitiesByPolicyId(any())).thenReturn(new ArrayList<>());
        PowerMockito.when(liveMountPolicyEntityDao.selectById(any())).thenReturn(liveMountPolicyEntity);
        UpdatePolicyRequest updatePolicyRequest = PolicyServiceImplTestData.getUpdatePolicyRequest();
        updatePolicyRequest.setScheduleStartTime(liveMountPolicyEntity.getScheduleStartTime().toString());
        updatePolicyRequest.setScheduleInterval(liveMountPolicyEntity.getScheduleInterval());
        updatePolicyRequest.setScheduleIntervalUnit(ScheduledUnit.HOUR);
        liveMountPolicyEntity.setScheduleIntervalUnit("d");
        policyService.updatePolicy(liveMountPolicyEntity.getPolicyId(), updatePolicyRequest);
        Mockito.verify(liveMountPolicyEntityDao, Mockito.times(1)).updateById(any());
    }

    /**
     * test update policy
     */
    @Test
    public void updatePolicy() {
        LiveMountPolicyEntity liveMountPolicyEntity = PolicyServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectList(any())).thenReturn(null);
        PowerMockito.when(liveMountPolicyEntityDao.updateById(any())).thenReturn(1);
        PowerMockito.when(liveMountService.queryLiveMountEntitiesByPolicyId(any())).thenReturn(new ArrayList<>());
        PowerMockito.when(liveMountPolicyEntityDao.selectById(any())).thenReturn(liveMountPolicyEntity);
        UpdatePolicyRequest updatePolicyRequest = PolicyServiceImplTestData.getUpdatePolicyRequest();
        policyService.updatePolicy(liveMountPolicyEntity.getPolicyId(), updatePolicyRequest);
        Mockito.verify(liveMountPolicyEntityDao, Mockito.times(1)).updateById(any());
    }

    /**
     * test delete policy liveMountEntities is not null
     */
    @Test
    public void deletePolicyLiveMountEntitiesIsNotNull() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("the policy has association live mount");
        PowerMockito.when(liveMountService.queryLiveMountEntitiesByPolicyId(any()))
                .thenReturn(Arrays.asList(new LiveMountEntity()));
        policyService.deletePolicy("1");
    }

    /**
     * test delete policy
     */
    @Test
    public void deletePolicy() {
        PowerMockito.when(liveMountService.queryLiveMountEntitiesByPolicyId(any())).thenReturn(null);
        PowerMockito.when(liveMountPolicyEntityDao.deleteById(anyString())).thenReturn(1);
        policyService.deletePolicy("1");
        Mockito.verify(liveMountPolicyEntityDao, Mockito.times(1)).deleteById(anyString());
    }

    /**
     * test select policy by id LiveMountPolicyEntity is null
     */
    @Test
    public void selectPolicyByIdLiveMountPolicyEntityIsNull() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("policy is not exist. policy id is " + "1");
        PowerMockito.when(liveMountPolicyEntityDao.selectById(any())).thenReturn(null);
        policyService.selectPolicyById("1");
    }

    /**
     * test select policy by id
     */
    @Test
    public void selectPolicyById() {
        LiveMountPolicyEntity liveMountPolicyEntityParam = PolicyServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectById(liveMountPolicyEntityParam.getPolicyId()))
                .thenReturn(liveMountPolicyEntityParam);
        LiveMountPolicyEntity liveMountPolicyEntity = policyService.selectPolicyById(
                liveMountPolicyEntityParam.getPolicyId());
        assertLiveMountPolicyEntity(liveMountPolicyEntityParam, liveMountPolicyEntity);
    }

    /**
     * assert liveMountPolicyEntity
     *
     * @param param liveMountPolicyEntity param
     * @param entity liveMountPolicyEntity return
     */
    private void assertLiveMountPolicyEntity(LiveMountPolicyEntity param, LiveMountPolicyEntity entity) {
        assert entity.getName().equals(param.getName());
        assert entity.getSchedulePolicy().equals(param.getSchedulePolicy());
        assert entity.getRetentionPolicy().equals(param.getRetentionPolicy());
        assert entity.getPolicyId().equals(param.getPolicyId());
    }
}
