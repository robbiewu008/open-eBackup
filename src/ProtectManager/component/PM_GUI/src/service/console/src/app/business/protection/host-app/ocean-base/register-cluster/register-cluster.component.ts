import { Component, Input, OnInit } from '@angular/core';
import {
  AbstractControl,
  FormArray,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import {
  BaseUtilService,
  DataMap,
  getMultiHostOps,
  I18NService,
  MultiCluster,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType,
  RouterUrl
} from 'app/shared';
import {
  USER_GUIDE_CACHE_DATA,
  cacheGuideResource
} from 'app/shared/consts/guide-config';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  cloneDeep,
  each,
  filter,
  get,
  includes,
  isEmpty,
  isUndefined,
  map
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-register-cluster',
  templateUrl: './register-cluster.component.html',
  styleUrls: ['./register-cluster.component.less']
})
export class RegisterClusterComponent implements OnInit {
  data;
  formGroup: FormGroup;
  dataMap = DataMap;
  proxyOptions = [];
  clientProxyOptions = [];
  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  agentsErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMinLength: this.i18n.get('common_host_min_number_label', [1])
  };
  portErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.integerErrorTip,
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  };
  ipErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.ipErrorTip
  };
  passwordErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_length_rang_label', [8, 32]),
    invalidMinLength: this.i18n.get('common_valid_length_rang_label', [8, 32]),
    invalidPwd: this.i18n.get('common_invalid_inputtext_label')
  };
  obclientTips = this.i18n.get('protection_agents_link_tips_label', [
    'OBClient'
  ]);
  observerTips = this.i18n.get('protection_agents_link_tips_label', [
    'OBServer'
  ]);
  rangeErrorTip = {
    ...this.baseUtilService.integerErrorTip,
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [60, 120])
  };
  @Input() rowData;
  constructor(
    private appUtilsService: AppUtilsService,
    private fb: FormBuilder,
    public i18n: I18NService,
    public baseUtilService: BaseUtilService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getProxyOptions();
  }

  showGuideNew(item): boolean {
    return (
      USER_GUIDE_CACHE_DATA.active &&
      includes(USER_GUIDE_CACHE_DATA.host, item.endpoint)
    );
  }

  helpHover() {
    this.appUtilsService.openRouter(RouterUrl.ProtectionHostAppHost);
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(),
          this.baseUtilService.VALID.maxLength(64)
        ]
      }),
      username: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(),
          this.baseUtilService.VALID.maxLength(64)
        ]
      }),
      password: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.minLength(8),
          this.baseUtilService.VALID.maxLength(32),
          this.validPwd()
        ]
      }),
      interval: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.rangeValue(60, 120)
        ]
      }),
      obClientAgents: new FormControl([], {
        validators: [this.baseUtilService.VALID.minLength(1)]
      }),
      obServerNodes: this.fb.array([this.getServerNodesFormGroup()])
    });
  }
  get obServerNodes() {
    return (this.formGroup.get('obServerNodes') as FormArray).controls;
  }
  getServerNodesFormGroup(data?) {
    return this.fb.group({
      proxy: new FormControl(data ? data.parentUuid : '', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      businessIp: new FormControl(data ? data.ip : '', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.ipv4()
        ]
      }),
      port: new FormControl(data ? data.port : '', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 65535)
        ]
      })
    });
  }

  validPwd(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup) || !control.value) {
        return null;
      }
      // 定义支持的特殊字符范围
      const specialChars = '~!@#%^&*_-+=|(){}[]:;,.?/';
      // 需要将字符中的特殊字符集做转译，比如正则自带的[]
      const escapedSpecialChars = specialChars.replace(
        /[-\/\\^$*+?.()|[\]{}]/g,
        '\\$&'
      );
      // 构建正则表达式，确保至少包含2个字母、2个数字和2个特殊字符
      const regex = new RegExp(
        '^(?=(?:.*[A-Za-z]){2})(?=(?:.*\\d){2})(?=(?:.*[' +
          escapedSpecialChars +
          ']){2}).{8,32}$'
      );
      if (!regex.test(control.value)) {
        return { invalidPwd: { value: control.value } };
      }

      return null;
    };
  }

  addServerRow(data?) {
    this.proxyOptions = [...this.proxyOptions];
    const formArr = this.formGroup.get('obServerNodes') as FormArray;
    formArr.push(this.getServerNodesFormGroup(data));
  }

  deleteServerRow(index: number) {
    const obServerNodes = this.formGroup.get('obServerNodes') as FormArray;
    obServerNodes.removeAt(index);
    this.selectChange();
  }

  updateData() {
    this.protectedResourceApiService
      .ShowResource({ resourceId: this.rowData.uuid })
      .subscribe((res: any) => {
        if (res.subType === DataMap.Resource_Type.OceanBaseCluster.value) {
          const clusterInfo = JSON.parse(
            get(res, 'extendInfo.clusterInfo', '{}')
          );
          const obServerNodes = get(clusterInfo, 'obServerAgents');
          this.deleteServerRow(0);
          this.formGroup.patchValue({
            name: res.name,
            username: res.auth.authKey,
            password: res.auth?.authPwd,
            obClientAgents: map(clusterInfo.obClientAgents, 'parentUuid'),
            interval: res.extendInfo?.logArchiveInterval
          });
          this.selectChange(map(obServerNodes, item => item.parentUuid));
          for (const node of obServerNodes) {
            this.addServerRow(node);
          }
        }
      });
  }

  selectChange(data?, id?) {
    const nodes = map(
      this.formGroup.get('obServerNodes').value,
      item => item.proxy
    );
    each(this.proxyOptions, item => {
      assign(item, {
        disabled: includes(data ? data : nodes, id ? id : item.uuid)
      });
    });
    this.proxyOptions = [...this.proxyOptions];
  }

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [`${DataMap.Resource_Type.OceanBaseCluster.value}Plugin`]
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
        if (MultiCluster.isMulti && isEmpty(this.rowData)) {
          resource = getMultiHostOps(resource);
        }
        each(resource, item => {
          const tmp = item.environment;
          hostArray.push({
            ...tmp,
            key: tmp.uuid,
            value: tmp.uuid,
            label: `${tmp.name}(${tmp.endpoint})`,
            isLeaf: true,
            parentUuid: tmp.uuid,
            disabled: false
          });
        });
        this.proxyOptions = hostArray;
        this.clientProxyOptions = cloneDeep(hostArray);
        if (this.rowData) {
          this.updateData();
        }
      }
    );
  }

  getParams() {
    const obServerAgentsArr = [];
    const clientAgents = [];
    const serverAgents = [];
    const obClientAgentsArr = [];
    each(this.formGroup.value.obClientAgents, item => {
      obClientAgentsArr.push({
        parentUuid: item,
        nodeType: 'OBClient'
      }),
        clientAgents.push({
          uuid: item
        });
    });
    each(this.formGroup.value.obServerNodes, item => {
      obServerAgentsArr.push({
        parentUuid: item.proxy,
        ip: item.businessIp,
        port: item.port,
        nodeType: 'OBServer'
      }),
        serverAgents.push({
          uuid: item.proxy
        });
    });
    const params: any = {
      name: this.formGroup.value.name,
      type: ResourceType.DATABASE,
      subType: DataMap.Resource_Type.OceanBaseCluster.value,
      extendInfo: {
        clusterInfo: JSON.stringify({
          obServerAgents: obServerAgentsArr,
          obClientAgents: obClientAgentsArr
        }),
        logArchiveInterval: this.formGroup.get('interval').value
      },
      dependencies: {
        clientAgents,
        serverAgents
      },
      auth: {
        authType: 2,
        authKey: this.formGroup.value.username,
        authPwd: this.formGroup.value.password
      }
    };
    return params;
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const params = this.getParams() as any;
      if (this.rowData) {
        this.protectedEnvironmentApiService
          .UpdateProtectedEnvironment({
            envId: this.rowData.uuid,
            UpdateProtectedEnvironmentRequestBody: params
          })
          .subscribe({
            next: res => {
              observer.next();
              observer.complete();
            },
            error: err => {
              observer.error(err);
              observer.complete();
            }
          });
      } else {
        this.protectedEnvironmentApiService
          .RegisterProtectedEnviroment({
            RegisterProtectedEnviromentRequestBody: params
          })
          .subscribe({
            next: res => {
              cacheGuideResource(res);
              observer.next();
              observer.complete();
            },
            error: err => {
              observer.error(err);
              observer.complete();
            }
          });
      }
    });
  }
}
