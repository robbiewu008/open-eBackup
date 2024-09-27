import { Component, OnInit } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  BaseUtilService,
  ClientManagerApiService,
  DataMap,
  DataMapService,
  I18NService,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  cloneDeep,
  differenceBy,
  filter,
  find,
  get,
  isEmpty,
  map,
  trim
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-register-group',
  templateUrl: './register-group.component.html',
  styleUrls: ['./register-group.component.less']
})
export class RegisterGroupComponent implements OnInit {
  rowData;
  isModify;
  type;
  hostName;
  formGroup: FormGroup;
  dataMap = DataMap;
  singleAgentsOptions = [];
  groupAgentOptions = [];

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private appUtilsService: AppUtilsService,
    private clientManagerApiService: ClientManagerApiService,
    public baseUtilService: BaseUtilService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit(): void {
    this.isModify = !!this.rowData;
    this.initForm();
    this.getProxyOptions();
    this.updateData();
  }

  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  usernameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.lengthErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };
  passwordErrorTip = {
    ...this.baseUtilService.pwdErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [2048])
  };
  concurrencyErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.integerErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 10])
  };

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [`${DataMap.Resource_Type.ExchangeGroup.value}Plugin`]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      res => {
        const resource = filter(res, item => !isEmpty(item.environment));
        const filterResource = map(resource, 'environment');
        let hostArray = map(filterResource, item => {
          return {
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: `${item.name}(${item.endpoint})`,
            isLeaf: true
          };
        });
        if (!this.isModify) {
          hostArray = hostArray.filter(
            item =>
              item.linkStatus ===
              DataMap.resource_LinkStatus_Special.normal.value
          );
        }
        this.singleAgentsOptions = hostArray;
        this.groupAgentOptions = cloneDeep(this.singleAgentsOptions);
        if (
          this.isModify &&
          this.rowData.extendInfo.subType ===
            DataMap.Resource_Type.ExchangeSingle.value
        ) {
          this.hostName = find(hostArray, item => {
            return item.value === this.rowData.extendInfo.agentUuid;
          }).label;
        }
        if (
          this.isModify &&
          this.rowData.extendInfo.subType ===
            DataMap.Resource_Type.ExchangeGroup.value
        ) {
          let agentArray = this.rowData.extendInfo.agentUuid.split(';');
          let hostNameArray = [];
          agentArray.forEach(item => {
            hostNameArray.push(
              find(hostArray, ite => {
                return ite.value === item;
              }).label
            );
          });
          this.hostName = hostNameArray.join(';');
        }
      }
    );
  }

  updateData() {
    if (!this.rowData) {
      return;
    }
    let data = {
      name: this.rowData.name,
      type: this.rowData.subType,
      userName: get(this.rowData, 'auth.authKey', ''),
      concurrency: Number(
        get(this.rowData, 'extendInfo.maxConcurrentJobNumber', '1')
      )
    };
    if (this.rowData.subType === DataMap.Resource_Type.ExchangeSingle.value) {
      assign(data, {
        single_agents: this.rowData.extendInfo.agentUuid
      });
    } else {
      assign(data, {
        group_agents: this.rowData.extendInfo.agentUuid.split(';')
      });
    }
    this.formGroup.patchValue(data);
  }

  initForm() {
    this.formGroup = this.fb.group({
      type: new FormControl(DataMap.Resource_Type.ExchangeSingle.value),
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(),
          this.baseUtilService.VALID.maxLength(64)
        ]
      }),
      single_agents: new FormControl(),
      group_agents: new FormControl(),
      userName: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ]
      }),
      password: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(2048)
        ]
      }),
      concurrency: new FormControl(1, {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 10)
        ]
      })
    });

    this.formGroup.get('type').valueChanges.subscribe(res => {
      if (res === DataMap.Resource_Type.ExchangeSingle.value) {
        this.formGroup
          .get('single_agents')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup.get('group_agents').clearValidators();
      } else {
        this.formGroup
          .get('group_agents')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup.get('single_agents').clearValidators();
      }
      this.formGroup.get('group_agents').updateValueAndValidity();
      this.formGroup.get('single_agents').updateValueAndValidity();
    });
  }

  getParams() {
    let agents = [];
    let reduceAgents = [];
    if (
      this.formGroup.value.type === DataMap.Resource_Type.ExchangeGroup.value
    ) {
      agents = map(this.formGroup.value.group_agents, item => {
        return { uuid: item };
      });
      if (this.rowData) {
        const reduceArr = differenceBy(
          this.rowData.extendInfo.agentUuid.split(';'),
          this.formGroup.value.group_agents
        );
        reduceAgents = map(reduceArr, item => ({ uuid: item }));
      }
    } else {
      agents = [{ uuid: this.formGroup.value.single_agents }];
    }
    return {
      name: this.formGroup.value.name,
      type: ResourceType.DATABASE,
      subType: this.formGroup.value.type,
      extendInfo: {
        // 后端老代码需要下发isGroup 前端只需要使用subType进行判断即可
        isGroup:
          this.formGroup.value.type ===
          DataMap.Resource_Type.ExchangeSingle.value
            ? '0'
            : '1',
        maxConcurrentJobNumber: String(this.formGroup.value.concurrency)
      },
      auth: {
        authType: 2,
        authKey: trim(this.formGroup.value.userName),
        authPwd: trim(this.formGroup.value.password)
      },
      dependencies: {
        agents,
        '-agents': reduceAgents
      }
    };
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const params = this.getParams();
      if (this.rowData) {
        this.protectedEnvironmentApiService
          .UpdateProtectedEnvironment({
            UpdateProtectedEnvironmentRequestBody: params as any,
            envId: this.rowData.uuid
          })
          .subscribe(
            res => {
              observer.next();
              observer.complete();
            },
            err => {
              observer.error(err);
              observer.complete();
            }
          );
      } else {
        this.protectedEnvironmentApiService
          .RegisterProtectedEnviroment({
            RegisterProtectedEnviromentRequestBody: params as any
          })
          .subscribe(
            res => {
              observer.next();
              observer.complete();
            },
            err => {
              observer.error(err);
              observer.complete();
            }
          );
      }
    });
  }
}
