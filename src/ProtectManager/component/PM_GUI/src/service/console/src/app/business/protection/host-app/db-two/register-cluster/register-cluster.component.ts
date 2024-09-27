import { Component, Input, OnInit } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  BaseUtilService,
  DataMap,
  DataMapService,
  I18NService,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType,
  MultiCluster,
  getMultiHostOps,
  isJson
} from 'app/shared';
import { TableConfig } from 'app/shared/components/pro-table';
import {
  USER_GUIDE_CACHE_DATA,
  cacheGuideResource
} from 'app/shared/consts/guide-config';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { each, filter, find, get, includes, isEmpty, map } from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';

@Component({
  selector: 'aui-register-cluster',
  templateUrl: './register-cluster.component.html',
  styleUrls: ['./register-cluster.component.less']
})
export class RegisterClusterComponent implements OnInit {
  item;
  dataDetail;
  optsConfig;
  optItems = [];
  proxyOptions = [];
  isMulti = MultiCluster.isMulti;
  typeOptions = this.dataMapService.toArray('dbTwoType').map(item => {
    return {
      ...item,
      isLeaf: true
    };
  });
  dataMap = DataMap;

  tableData = {
    data: [],
    total: 0
  };
  tableConfig: TableConfig;
  formGroup: FormGroup;
  valid$ = new Subject<boolean>();

  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  usernameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.lengthErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };
  agentsErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMinLength: this.i18n.get('common_host_number_least_2_label')
  };

  @Input() rowData;
  constructor(
    private appUtilsService: AppUtilsService,
    public baseUtilService: BaseUtilService,
    private fb: FormBuilder,
    public i18n: I18NService,
    private dataMapService: DataMapService,
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

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(),
          this.baseUtilService.VALID.maxLength(64)
        ]
      }),
      type: new FormControl(DataMap.dbTwoType.dpf.value, {
        validators: [this.baseUtilService.VALID.required()]
      }),
      node: new FormControl([], {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.minLength(2)
        ]
      })
    });

    if (this.rowData) {
      this.getDataDetail();
    }
  }

  getDataDetail() {
    this.protectedResourceApiService
      .ShowResource({ resourceId: this.rowData.uuid })
      .subscribe(res => {
        this.formGroup.patchValue({
          name: res.name,
          type: res.extendInfo?.clusterType,
          node: map(get(res, 'dependencies.agents'), (item: any) => item.uuid)
        });

        this.dataDetail = res;
      });
  }

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [`${DataMap.Application_Type.db2.value}Plugin`]
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
            isLeaf: true
          });
        });
        this.proxyOptions = hostArray;
      }
    );
  }

  getParams() {
    const deletedNode = filter(
      get(this.dataDetail, 'dependencies.agents'),
      item => !find(this.formGroup.value.node, val => val === item.uuid)
    );

    return {
      name: this.formGroup.value.name,
      type: ResourceType.DATABASE,
      subType: DataMap.Resource_Type.dbTwoCluster.value,
      extendInfo: {
        clusterType: this.formGroup.value.type
      },
      dependencies: {
        agents: map(this.formGroup.value.node, item => {
          return {
            uuid: item
          };
        }),
        '-agents': map(deletedNode, item => {
          return {
            uuid: item.uuid
          };
        })
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
              // 缓存向导数据
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
