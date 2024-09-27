import {
  BaseUtilService,
  I18NService,
  DataMapService,
  CommonConsts,
  DataMap,
  ResourceType,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  MultiCluster,
  getMultiHostOps
} from 'app/shared';
import { FormGroup, FormBuilder, FormControl } from '@angular/forms';
import { Component, OnInit, TemplateRef, ViewChild } from '@angular/core';
import { Observable, Observer } from 'rxjs';
import {
  isNumber,
  each,
  map,
  isEmpty,
  differenceBy,
  filter,
  includes
} from 'lodash';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  USER_GUIDE_CACHE_DATA,
  cacheGuideResource
} from 'app/shared/consts/guide-config';

@Component({
  selector: 'aui-king-base-register',
  templateUrl: './king-base-register.component.html',
  styleUrls: ['./king-base-register.component.less']
})
export class KingBaseRegisterComponent implements OnInit {
  data;
  formGroup: FormGroup;
  isTest = false;
  proxyOptions = [];
  agentsErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMinLength: this.i18n.get('common_host_number_least_2_label'),
    invalidMaxLength: this.i18n.get('common_host_max_number_label', [2])
  };
  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  typeOptions = this.dataMapService
    .toArray('PostgreSql_Cluster_Type')
    .map(item => {
      item.isLeaf = true;
      return item;
    });

  @ViewChild('footerTpl', { static: true }) footerTpl: TemplateRef<any>;

  constructor(
    private appUtilsService: AppUtilsService,
    private fb: FormBuilder,
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    public baseUtilService: BaseUtilService
  ) {}

  ngOnInit() {
    this.initForm();
    this.updateData();
    this.getProxyOptions();
  }

  showGuideNew(item): boolean {
    return (
      USER_GUIDE_CACHE_DATA.active &&
      includes(USER_GUIDE_CACHE_DATA.host, item.endpoint)
    );
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl(
        {
          value: !isEmpty(this.data) ? this.data.name : '',
          disabled: !!this.data
        },
        {
          validators: [
            this.baseUtilService.VALID.name(),
            this.baseUtilService.VALID.maxLength(64)
          ]
        }
      ),
      type: new FormControl(!isEmpty(this.data) ? this.data.clusterType : '', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      agents: new FormControl(
        !isEmpty(this.data) ? map(this.data.dependencies.agents, 'uuid') : [],
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.minLength(2),
            this.baseUtilService.VALID.maxLength(2)
          ]
        }
      )
    });
  }

  updateData() {
    if (!this.data) {
      return;
    }
    this.protectedResourceApiService
      .ShowResource({ resourceId: this.data.uuid })
      .subscribe(res => {
        const data = {
          name: res.name,
          type: res.extendInfo?.clusterType,
          agents: map(res['dependencies']['agents'], 'uuid')
        };
        this.formGroup.patchValue(data);
      });
  }

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [`${DataMap.Resource_Type.KingBaseInstance.value}Plugin`]
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
        if (MultiCluster.isMulti && isEmpty(this.data)) {
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

  getParams() {
    let reduceAgents = [];
    if (this.data) {
      reduceAgents = differenceBy(
        this.data.dependencies.agents.map(item => item.uuid),
        this.formGroup.value.agents
      );
    }
    return {
      name: this.formGroup.get('name').value,
      type: ResourceType.DATABASE,
      subType: DataMap.Resource_Type.KingBaseCluster.value,
      extendInfo: {
        clusterType: this.formGroup.value.type
      },
      dependencies: {
        agents: map(this.formGroup.value.agents, item => {
          return { uuid: item };
        }),
        '-agents': !isEmpty(this.data)
          ? reduceAgents.map(item => {
              return { uuid: item };
            })
          : []
      }
    };
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const params = this.getParams();
      if (this.data) {
        this.protectedEnvironmentApiService
          .UpdateProtectedEnvironment({
            envId: this.data.uuid,
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
        return;
      }
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
    });
  }
}
