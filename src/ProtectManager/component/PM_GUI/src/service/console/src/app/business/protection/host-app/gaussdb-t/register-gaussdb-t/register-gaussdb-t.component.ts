import { FormGroup, FormBuilder, FormControl } from '@angular/forms';
import { Component, OnInit, ViewChild, TemplateRef } from '@angular/core';
import {
  DataMapService,
  I18NService,
  BaseUtilService,
  CommonConsts,
  DataMap,
  ProtectedResourceApiService,
  ResourceType,
  ProtectedEnvironmentApiService,
  MultiCluster,
  getMultiHostOps
} from 'app/shared';
import { ModalRef } from '@iux/live';
import {
  isNumber,
  each,
  map,
  first,
  set,
  get,
  toNumber,
  filter,
  isEmpty,
  includes
} from 'lodash';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  USER_GUIDE_CACHE_DATA,
  cacheGuideResource
} from 'app/shared/consts/guide-config';

@Component({
  selector: 'aui-register-gaussdb-t',
  templateUrl: './register-gaussdb-t.component.html',
  styleUrls: ['./register-gaussdb-t.component.less']
})
export class RegisterGaussdbTComponent implements OnInit {
  item;
  dataMap = DataMap;
  isTest = false;
  isDisabled = true;
  okLoading = false;
  testLoading = false;
  proxyOptions = [];
  formGroup: FormGroup;
  authOptions = this.dataMapService.toArray('Postgre_Auth_Method').map(item => {
    item['isLeaf'] = true;
    return item;
  });
  placeHolderTips = this.i18n.get(
    'protection_guassdb_single_username_desc_label'
  );
  nameErrorTip = this.baseUtilService.nameErrorTip;
  usernameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.lengthErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };
  passwordErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.lengthErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };

  deploymentTypeOptions = this.dataMapService
    .toArray('guassdbTDeploymentType')
    .filter(v => (v.isLeaf = true));

  @ViewChild('footerTpl', { static: true }) footerTpl: TemplateRef<any>;

  constructor(
    private appUtilsService: AppUtilsService,
    private modal: ModalRef,
    private i18n: I18NService,
    private fb: FormBuilder,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    public dataMapService: DataMapService,
    public baseUtilService: BaseUtilService
  ) {}

  ngOnInit() {
    this.getFooter();
    this.initForm();
    this.getProxyOptions();
    this.updateData();
  }

  showGuideNew(item): boolean {
    return (
      USER_GUIDE_CACHE_DATA.active &&
      includes(USER_GUIDE_CACHE_DATA.host, item.endpoint)
    );
  }

  updateData() {
    if (!this.item) {
      return;
    }
    this.protectedResourceApiService
      .ShowResource({ resourceId: this.item.uuid })
      .subscribe(res => {
        const data = {
          name: res.name,
          type: res.subType,
          username: res['auth']['authKey'],
          deployment_type: res['extendInfo']?.deployType
        };

        if (res.subType === DataMap.gaussDBTClusterType.single.value) {
          set(data, 'agent', get(res, 'dependencies.agents[0].uuid'));
        } else {
          set(data, 'agents', map(res['dependencies']?.agents, 'uuid'));
          set(data, 'deployment_type', get(res, 'extendInfo.deployType'));
        }
        this.formGroup.patchValue(data);
        this.formGroup
          .get('authMode')
          .setValue(
            toNumber(
              get(
                res,
                'extendInfo.sysType',
                DataMap.Postgre_Auth_Method.os.value
              )
            )
          );

        if (res.subType === DataMap.gaussDBTClusterType.single.value) {
          this.formGroup.get('username').disable();
        }
      });
  }

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [`${DataMap.Resource_Type.GaussDB_T.value}Plugin`]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        resource = filter(resource, item => !isEmpty(item.environment));
        const hostArray = [];
        resource = filter(
          resource,
          item =>
            item.environment.extendInfo.scenario ===
            DataMap.proxyHostType.external.value
        );
        if (MultiCluster.isMulti && isEmpty(this.item)) {
          resource = getMultiHostOps(resource);
        }
        each(resource, item => {
          const tmp = item.environment;
          hostArray.push({
            ...tmp,
            key: tmp.uuid,
            value: tmp.uuid,
            label: `${tmp.name}(${tmp.endpoint})`,
            isLeaf: true
          });
        });
        this.proxyOptions = hostArray;
      }
    );
  }

  getFooter() {
    this.modal.setProperty({ lvFooter: this.footerTpl });
  }

  initForm() {
    this.formGroup = this.fb.group({
      type: new FormControl(DataMap.gaussDBTClusterType.single.value),
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name()
        ]
      }),
      username: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ]
      }),
      deployment_type: new FormControl(
        DataMap.guassdbTDeploymentType.single.value
      ),
      agent: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      agents: new FormControl([]),
      authMode: new FormControl(DataMap.Postgre_Auth_Method.os.value, {
        validators: [this.baseUtilService.VALID.required()]
      }),
      password: new FormControl('')
    });

    this.formGroup.get('type').valueChanges.subscribe(res => {
      if (res === DataMap.gaussDBTClusterType.single.value) {
        this.placeHolderTips = this.i18n.get(
          'protection_guassdb_single_username_desc_label'
        );
        this.formGroup
          .get('agent')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup.get('agents').clearValidators();
        this.formGroup
          .get('deployment_type')
          .setValue(DataMap.guassdbTDeploymentType.single.value);
        this.formGroup.get('deployment_type').clearValidators();
        this.isDisabled = true;
      } else {
        this.placeHolderTips = this.i18n.get(
          'protection_guassdb_username_desc_label'
        );
        this.formGroup
          .get('agents')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup
          .get('deployment_type')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup.get('agent').clearValidators();
        this.formGroup.get('deployment_type').setValue('');
        this.isDisabled = false;
      }
      this.formGroup.get('agent').updateValueAndValidity();
      this.formGroup.get('agents').updateValueAndValidity();
      this.formGroup.get('deployment_type').updateValueAndValidity();
      this.formGroup
        .get('authMode')
        .setValue(DataMap.Postgre_Auth_Method.os.value);
    });

    this.formGroup.get('authMode').valueChanges.subscribe(res => {
      if (
        res === DataMap.Postgre_Auth_Method.db.value &&
        this.formGroup.value.type === DataMap.gaussDBTClusterType.single.value
      ) {
        this.formGroup
          .get('password')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(32)
          ]);
      } else {
        this.formGroup.get('password').clearValidators();
      }
      this.formGroup.get('password').updateValueAndValidity();
    });
  }

  getParams() {
    const params = {
      name: this.formGroup.value.name,
      type: ResourceType.DATABASE,
      subType: this.formGroup.value.type,
      auth: {
        authType:
          this.formGroup.value.type === DataMap.gaussDBTClusterType.single.value
            ? DataMap.Postgre_Auth_Method.os.value
            : DataMap.Postgre_Auth_Method.db.value,
        authKey: this.formGroup.get('username').value
      },
      dependencies: {
        agents: map(
          this.formGroup.value.type === DataMap.gaussDBTClusterType.single.value
            ? [this.formGroup.value.agent]
            : this.formGroup.value.agents,
          item => {
            return { uuid: item };
          }
        )
      }
    };

    if (
      this.formGroup.value.type === DataMap.gaussDBTClusterType.single.value
    ) {
      set(params, 'extendInfo.sysType', this.formGroup.value.authMode);
      set(
        params,
        'auth.extendInfo.sysAuthPwd',
        this.formGroup.value.authMode === DataMap.Postgre_Auth_Method.db.value
          ? this.formGroup.value.password
          : ''
      );
    } else {
      set(
        params,
        'extendInfo.deployType',
        this.formGroup.value.deployment_type
      );
    }

    return params;
  }

  test() {
    const params = this.getParams();
    this.protectedEnvironmentApiService
      .CheckEnvironment({
        checkEnvironmentRequestBody: params
      })
      .subscribe(res => {
        this.isTest = true;
      });
  }

  ok() {
    if (this.formGroup.invalid) {
      return;
    }
    const params = this.getParams();
    if (this.item) {
      this.protectedEnvironmentApiService
        .UpdateProtectedEnvironment({
          envId: this.item.uuid,
          UpdateProtectedEnvironmentRequestBody: params
        })
        .subscribe(res => {
          this.modal.close();
        });
      return;
    }
    this.protectedEnvironmentApiService
      .RegisterProtectedEnviroment({
        RegisterProtectedEnviromentRequestBody: params
      })
      .subscribe(res => {
        cacheGuideResource(res);
        this.modal.close();
      });
  }
}
