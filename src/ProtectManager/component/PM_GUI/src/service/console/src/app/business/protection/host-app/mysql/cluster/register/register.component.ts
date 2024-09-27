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
import { Component, OnInit } from '@angular/core';
import { Observable, Observer } from 'rxjs';
import {
  isNumber,
  each,
  map,
  get,
  filter,
  find,
  size,
  set,
  isEmpty,
  includes
} from 'lodash';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  USER_GUIDE_CACHE_DATA,
  cacheGuideResource
} from 'app/shared/consts/guide-config';

@Component({
  selector: 'aui-register',
  templateUrl: './register.component.html',
  styleUrls: ['./register.component.less']
})
export class RegisterComponent implements OnInit {
  data;
  originAgents;
  formGroup: FormGroup;
  proxyOptions = [];
  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  typeOptions = this.dataMapService.toArray('Mysql_Cluster_Type').map(item => {
    item.isLeaf = true;
    return item;
  });

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
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(),
          this.baseUtilService.VALID.maxLength(64)
        ]
      }),
      type: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      agents: new FormControl([], {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.minLength(1)
        ]
      })
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
        this.originAgents = get(res, 'dependencies.agents');
        this.formGroup.patchValue(data);
      });
  }

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [`${DataMap.Resource_Type.MySQLInstance.value}Plugin`]
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
    const params = {
      name: this.formGroup.value.name,
      type: ResourceType.DATABASE,
      subType: DataMap.Resource_Type.MySQLCluster.value,
      extendInfo: {
        clusterType: this.formGroup.value.type
      },
      dependencies: {
        agents: map(this.formGroup.value.agents, item => {
          return { uuid: item };
        })
      }
    };
    const deleteNode =
      filter(
        this.originAgents,
        item => !find(this.formGroup.value.agents, val => val === item.uuid)
      ) || [];

    if (size(deleteNode)) {
      set(
        params.dependencies,
        '-agents',
        map(deleteNode, item => {
          return {
            uuid: item.uuid
          };
        })
      );
    }
    return params;
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
