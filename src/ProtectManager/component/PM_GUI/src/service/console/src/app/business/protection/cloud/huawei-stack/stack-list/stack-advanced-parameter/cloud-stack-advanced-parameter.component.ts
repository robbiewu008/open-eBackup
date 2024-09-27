import { Component, OnInit, ViewChild } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { MessageService } from '@iux/live';
import {
  BaseUtilService,
  ClientManagerApiService,
  CommonConsts,
  CookieService,
  DataMap,
  I18NService,
  MultiCluster,
  ProtectedResourceApiService
} from 'app/shared';
import { ProtectFilterComponent } from 'app/shared/components/protect-filter/protect-filter.component';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  defer,
  each,
  includes,
  isArray,
  isEmpty,
  isString,
  map,
  reject
} from 'lodash';
import { Subject } from 'rxjs';

@Component({
  selector: 'aui-cloud-stack-advanced-parameter',
  templateUrl: './cloud-stack-advanced-parameter.component.html',
  styleUrls: ['./cloud-stack-advanced-parameter.component.less']
})
export class CloudStackAdvancedParameterComponent implements OnInit {
  resourceData;
  resourceType;
  selectedNode;
  hostOptions = [];
  formGroup: FormGroup;
  valid$ = new Subject<boolean>();
  dataMap = DataMap;
  @ViewChild(ProtectFilterComponent, { static: false })
  ProtectFilterComponent: ProtectFilterComponent;
  hostBuiltinLabel = this.i18n.get('protection_hcs_host_builtin_label');
  hostExternalLabel = this.i18n.get('protection_hcs_host_external_label');
  hasArchive = false;

  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;
  extParams;

  constructor(
    public fb: FormBuilder,
    private appUtilsService: AppUtilsService,
    private i18n: I18NService,
    public message: MessageService,
    private cookieService: CookieService,
    public baseUtilService: BaseUtilService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private clientManagerApiService: ClientManagerApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getProxyOptions();
    this.updateData();
  }

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        pluginType: `HCScontainerPlugin`
      })
    };
    if (!this.resourceData.sla_id) {
      assign(extParams.conditions, {
        linkStatus: [DataMap.resource_LinkStatus_Special.normal.value]
      });
    }
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.clientManagerApiService.queryAgentListInfoUsingGET(params),
      resource => {
        // 多集群场景下HCS的注册 保护 恢复场景 过滤内置代理主机，云上HCS不需要内置代理
        if (MultiCluster.isMulti || this.isHcsUser) {
          resource = reject(
            resource,
            item =>
              item.extendInfo.scenario === DataMap.proxyHostType.builtin.value
          );
        }
        let bindAgents;
        try {
          bindAgents = this.resourceData.protectedObject?.extParameters?.agents?.split(
            ';'
          );
        } catch (error) {
          bindAgents = [];
        }
        resource = reject(
          resource,
          item =>
            item.linkStatus !==
              DataMap.resource_LinkStatus_Special.normal.value &&
            !includes(bindAgents, item.uuid)
        );
        const hostArray = [];
        each(resource, item => {
          hostArray.push({
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: `${item.name}(${item.endpoint})`,
            linkStatus: item.linkStatus,
            isLeaf: true
          });
        });
        this.hostOptions = hostArray;
      }
    );
  }

  updateData() {
    if (!this.resourceData.protectedObject?.extParameters) {
      return;
    }
    const extParameters = isString(
      this.resourceData.protectedObject?.extParameters
    )
      ? JSON.parse(this.resourceData.protectedObject?.extParameters)
      : this.resourceData.protectedObject?.extParameters;
    if (!isEmpty(extParameters.resource_filters)) {
      defer(() =>
        this.ProtectFilterComponent.setFilter(extParameters.resource_filters)
      );
    }
    assign(extParameters, {
      proxyHost: extParameters.agents?.split(';') || [],
      slaOverwrite: extParameters.overwrite || false,
      slaPolicy: extParameters.binding_policy || [
        'APPLY_TO_ALL',
        'APPLY_TO_NEW'
      ]
    });
    this.extParams = extParameters;
    this.formGroup.patchValue(extParameters);
    setTimeout(() => {
      this.valid$.next(this.formGroup.valid);
    }, 500);
  }

  initForm() {
    this.formGroup = this.fb.group({
      proxyHost: new FormControl([]),
      slaOverwrite: new FormControl(false),
      slaPolicy: new FormControl(['APPLY_TO_ALL', 'APPLY_TO_NEW'])
    });
  }

  initData(data: any, resourceType: string) {
    this.resourceData = isArray(data) ? data[0] : data;
    this.resourceType = resourceType;
  }

  onOK() {
    const ext_parameters = {};
    assign(ext_parameters, {
      agents:
        this.formGroup.value.proxyHost
          ?.filter(item => {
            return includes(map(this.hostOptions, 'value'), item);
          })
          .join(';') || null
    });

    if (this.resourceType === DataMap.Resource_Type.Project.value) {
      const stackFilters = this.ProtectFilterComponent.getAllFilters();
      assign(ext_parameters, {
        resource_filters: !isEmpty(stackFilters) ? stackFilters : null,
        overwrite: this.formGroup.value.slaOverwrite,
        binding_policy: this.formGroup.value.slaPolicy
      });
    }

    // 修改索引
    if (this.resourceType !== DataMap.Resource_Type.Project.value) {
      each(
        [
          'backup_res_auto_index',
          'archive_res_auto_index',
          'enable_security_archive'
        ],
        key => {
          if (this.formGroup.get(key)) {
            assign(ext_parameters, {
              [key]: this.formGroup.get(key).value
            });
          }
        }
      );
    }

    return {
      ext_parameters
    };
  }
}
