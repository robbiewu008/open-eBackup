/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
import { ChangeDetectorRef, Component, OnInit } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import { ModalRef } from '@iux/live';
import {
  allAppType,
  ApplicationType,
  BaseUtilService,
  DataMap,
  DataMapService,
  GlobalService,
  I18NService,
  ResourceSetApiService,
  ResourceSetType,
  RouterUrl
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  defer,
  each,
  find,
  includes,
  intersection,
  isEmpty,
  map,
  some,
  uniq
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-create-resourceset',
  templateUrl: './create-resourceset.component.html',
  styleUrls: ['./create-resourceset.component.less']
})
export class CreateResourcesetComponent implements OnInit {
  data;
  formGroup: FormGroup;
  applicationList = [];
  applicationType = ApplicationType;
  resourceSetType = ResourceSetType;
  activeIndex = 0;
  singleLayerApp = allAppType.singleLayerApp;
  virtualCloudApp = allAppType.virtualCloudApp;

  lastSelection = [];
  allSelectionMap: any = {};
  modifyResourceGroupList = []; // 用于存放修改时修改了的资源组
  deviceOptions = this.dataMapService.toArray('Device_Storage_Type');

  nameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('common_storage_pool_name_invalid_label')
  };
  descErrorTip = {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [255])
  };

  constructor(
    public modal: ModalRef,
    public globalService: GlobalService,
    public baseUtilService: BaseUtilService,
    private fb: FormBuilder,
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private dataMapService: DataMapService,
    private appUtilsService: AppUtilsService,
    private resourceSetService: ResourceSetApiService
  ) {}

  ngOnInit(): void {
    this.initForm();
    this.initAvailableApplication();
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: ['', [this.baseUtilService.VALID.required(), this.validName()]],
      filter: [''],
      desc: ['', [this.baseUtilService.VALID.maxLength(255)]]
    });

    this.formGroup.statusChanges.subscribe(res => {
      this.modal.getInstance().lvOkDisabled = this.formGroup.invalid;
    });
    if (this.data) {
      this.formGroup.patchValue({
        name: this.data[0].name,
        desc: this.data[0].description
      });
    }
  }

  validName(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) {
        return null;
      }

      const value = control.value;
      const reg_name = /^[a-zA-Z0-9_\u4e00-\u9fa5]{1}[\u4e00-\u9fa5\w-\.]*$/;

      if (!reg_name.test(value) || value.length < 4 || value.length > 64) {
        return { invalidName: { value: control.value } };
      }

      return null;
    };
  }

  initAvailableApplication() {
    const allApp = this.appUtilsService.getApplicationConfig();
    allApp.fileService.unshift({
      id: 'storage-device',
      slaId: ApplicationType.NASFileSystem,
      key: '',
      hide: false,
      label: this.i18n.get('protection_storage_device_label'),
      prefix: 'S',
      color: '#000000',
      protected_count: 0,
      count: 0,
      protectionUrl: RouterUrl.ProtectionStorageDeviceInfo,
      copyUrl: RouterUrl.ProtectionStorageDeviceInfo,
      resType: DataMap.Resource_Type.NASFileSystem.value,
      resourceSetType: ResourceSetType.StorageEquipment
    });
    let appTypes = [
      { label: this.i18n.get('common_database_label'), apps: allApp.database },
      { label: this.i18n.get('common_bigdata_label'), apps: allApp.bigData },
      {
        label: this.i18n.get('common_virtualization_label'),
        apps: allApp.virtualization,
        key: 'virtualization'
      },
      {
        label: this.i18n.get('common_container_label'),
        apps: allApp.container
      },
      {
        label: this.i18n.get('common_huawei_clouds_label'),
        apps: allApp.cloud
      },
      {
        label: this.i18n.get('common_application_label'),
        apps: allApp.application
      },
      {
        label: this.i18n.get('common_file_systems_label'),
        apps: allApp.fileService
      },
      {
        label: this.i18n.get('protection_clients_label'),
        type: ResourceSetType.Agent
      },
      {
        label: this.i18n.get('common_sla_label'),
        type: ResourceSetType.SLA
      },
      {
        label: this.i18n.get('common_limit_rate_policy_label'),
        type: ResourceSetType.QOS
      },
      {
        label: this.i18n.get('Air Gap'),
        type: ResourceSetType.AirGap
      },
      {
        label: this.i18n.get('common_mount_update_policy_label'),
        type: ResourceSetType.LiveMount
      },
      {
        label:
          this.i18n.get('deploy_type') === DataMap.Deploy_Type.x3000.value
            ? this.i18n.get('common_worm_policy_label')
            : this.i18n.get('common_anti_policy_label'),
        type: ResourceSetType.Worm
      },
      {
        label: this.i18n.get('common_report_label'),
        type: ResourceSetType.Report
      }
    ];
    this.resourceSetService.queryAvailableScopeModule({}).subscribe(res => {
      this.initApplication(uniq(res.filter(item => !!item)), appTypes);
    });
  }

  initApplication(appList, appTypes) {
    // 生成当前已接入的资源列表集合

    appTypes = appTypes.filter((item: any) => {
      if (item?.apps) {
        item.apps = item.apps?.filter(
          app =>
            appList.includes(app?.resourceSetType) ||
            (appList.includes(ResourceSetType.FilesetTemplate) &&
              app?.resourceSetType === ResourceSetType.Fileset)
        );
      }

      return (
        (appList.includes(item?.type) ||
          (appList.includes(ResourceSetType.ReportSubscription) &&
            item.type === ResourceSetType.Report) ||
          !!item.apps?.length) &&
        !(
          this.appUtilsService.isDistributed &&
          includes([ResourceSetType.AirGap, ResourceSetType.Worm], item.type)
        )
      );
    });

    for (const appType of appTypes) {
      this.applicationList.push(appType);
    }

    if (this.data) {
      this.getResource();
    }
  }

  getResource() {
    // 用于修改时标识上次的已选择
    this.resourceSetService
      .queryResourceSetTypeCount({
        resourceSetId: this.data[0].uuid
      })
      .subscribe(res => {
        if (!res) {
          return;
        }
        // nas由于存储设备存在父选子跨资源集类型，所以单独处理
        this.lastSelection = map(res, (item: any) => {
          if (ResourceSetType.StorageEquipment === item.scopeModule) {
            return some(this.deviceOptions, { value: item.resourceSubType })
              ? item.scopeModule
              : [DataMap.Resource_Type.NASFileSystem.value].includes(
                  item.resourceSubType
                )
              ? ResourceSetType.NasFileSystem
              : ResourceSetType.NasShare;
          }
          return item.scopeModule;
        });
        this.selectChange();
      });
  }

  beforeExpanded = collapse => {
    // 展开时触发获取数据
    this.globalService.emitStore({
      action: collapse.lvId,
      state: true
    });

    this.applicationList.forEach(item => {
      if (item?.apps) {
        item.apps.forEach(app => {
          app.showAllSelect = app.label === collapse.lvId ? true : false;
        });
      }
    });
  };

  afterCollapse = collapse => {
    // 收起时就不展示全选
    this.applicationList.forEach(item => {
      if (item?.apps) {
        const app = item.apps.find(app => app.label === collapse.lvId);
        if (app) {
          app.showAllSelect = false;
        }
      }
    });
  };

  selectChange(e?) {
    // 用于已选、未选判断
    each(this.applicationList, item => {
      if (item?.apps) {
        let flag = false;
        each(item.apps, app => {
          app.selected = this.getSelected(app.resourceSetType);
          flag = flag || app.selected;
        });
        item.selected = flag;
      } else {
        // SLA、QOS、AGENT等
        item.selected = this.getSelected(item.type);
      }
    });
  }

  /**
   * 获取是否被选中，如果修改场景则存表,否则正常选中逻辑判断
   */
  getSelected(type) {
    if (type === ResourceSetType.Fileset) {
      // 文件集由于文件集模板使用不同的资源集类型所以得单独处理
      return this.parseSpecialSelected(
        type,
        ResourceSetType.Fileset,
        ResourceSetType.FilesetTemplate
      );
    }
    if (type === ResourceSetType.Report) {
      return this.parseSpecialSelected(
        type,
        ResourceSetType.Report,
        ResourceSetType.ReportSubscription
      );
    }
    return (
      (this.lastSelection.includes(type) && !(type in this.allSelectionMap)) ||
      (type in this.allSelectionMap &&
        (!isEmpty(this.allSelectionMap[type]?.data) ||
          !!this.allSelectionMap[type].isAllSelected)) ||
      this.getResourceGroupChange(type)
    );
  }

  parseSpecialSelected(type, mainType, otherType) {
    return (
      (!!intersection(this.lastSelection, [mainType, otherType]).length &&
        !(type in this.allSelectionMap) &&
        !(otherType in this.allSelectionMap)) ||
      ((type in this.allSelectionMap || otherType in this.allSelectionMap) &&
        (!isEmpty(this.allSelectionMap[type]?.data) ||
          !isEmpty(this.allSelectionMap[otherType]?.data) ||
          !!this.allSelectionMap[mainType]?.isAllSelected ||
          !!this.allSelectionMap[otherType]?.isAllSelected))
    );
  }

  getResourceGroupChange(type) {
    // 资源组需要单独加个逻辑
    if (isEmpty(this.allSelectionMap[ResourceSetType.RESOURCE_GROUP]?.data)) {
      return false;
    }

    return find(
      this.allSelectionMap[ResourceSetType.RESOURCE_GROUP].data,
      item => item.scopeType === type
    );
  }

  selectAll(e, app) {
    // 用于全选判断
    e.stopPropagation();
    app.allSelect = true;
    defer(() => this.selectChange());
  }

  cancelSelectAll(e, app) {
    // 用于取消全选判断
    e.stopPropagation();
    app.allSelect = false;
    defer(() => this.selectChange());
  }

  resourceGroupChange(e) {
    if (!this.modifyResourceGroupList.includes(e)) {
      this.modifyResourceGroupList.push(e);
    }
  }

  getParams() {
    const params = {
      name: this.formGroup.value.name,
      description: this.formGroup.value.desc
    };

    if (!!this.data) {
      assign(params, {
        resourceSetId: this.data[0].uuid
      });
    }

    let resourceSetRelationList = [];
    let allSelectList = [];
    if (isEmpty(this.allSelectionMap)) {
      return params;
    }

    for (const key in this.allSelectionMap) {
      // 特殊资源自成一体
      const type = includes(
        [
          ResourceSetType.QOS,
          ResourceSetType.SLA,
          ResourceSetType.Agent,
          ResourceSetType.LiveMount,
          ResourceSetType.Worm,
          ResourceSetType.AirGap,
          ResourceSetType.Report,
          ResourceSetType.ReportSubscription,
          ResourceSetType.RESOURCE_GROUP
        ],
        key
      )
        ? key
        : 'RESOURCE';
      if (type === ResourceSetType.RESOURCE_GROUP) {
        // 这时候对value进行遍历得出我们的资源组参数
        this.configResourceGroupData(key, resourceSetRelationList, type);
      } else {
        let tmpAllselectData = this.allSelectionMap[key]?.data || [];
        // Nas如果父亲被选中后还下发子级，就会造成数量重复计算
        if (
          includes(
            [ResourceSetType.NasFileSystem, ResourceSetType.NasShare],
            key
          )
        ) {
          tmpAllselectData = tmpAllselectData.filter(
            item =>
              !some(
                this.allSelectionMap[ResourceSetType.StorageEquipment]?.data,
                parent =>
                  parent.uuid === item?.environment?.uuid ||
                  parent.uuid === item?.parentUuid
              )
          );
        }
        const resourceDto = {
          scopeModule: key,
          resourceIdList: map(
            tmpAllselectData,
            item => item?.uuid || item?.policyId || item?.id
          )
        };

        if (!!this.allSelectionMap[key]?.isAllSelected) {
          assign(resourceDto, {
            isAllSelected: this.allSelectionMap[key]?.isAllSelected,
            resourceIdList: []
          });
          allSelectList.push(key);
        }
        const targetRelation = resourceSetRelationList.find(
          item => item.type === type
        );
        if (targetRelation) {
          targetRelation.resourceDtoList.push(resourceDto);
        } else {
          resourceSetRelationList.push({
            type,
            resourceDtoList: [resourceDto]
          });
        }
      }
    }

    if (!isEmpty(resourceSetRelationList)) {
      // 如果资源全选了，要帮资源组全选
      this.allSelectResourceGroup(resourceSetRelationList, allSelectList);

      assign(params, { resourceSetRelationList: resourceSetRelationList });
    }

    return params;
  }

  private allSelectResourceGroup(
    resourceSetRelationList: any[],
    allSelectList: any[]
  ) {
    let tmpResourceGroupList: any = find(resourceSetRelationList, {
      type: ResourceSetType.RESOURCE_GROUP
    });
    if (!tmpResourceGroupList) {
      tmpResourceGroupList = map(allSelectList, item => {
        return {
          scopeModule: item,
          resourceIdList: [],
          isAllSelected: true
        };
      });
      resourceSetRelationList.unshift({
        type: ResourceSetType.RESOURCE_GROUP,
        resourceDtoList: tmpResourceGroupList
      });
    } else {
      each(allSelectList, item => {
        const tmpResourceGroup = find(tmpResourceGroupList.resourceDtoList, {
          scopeModule: item
        });
        if (!!tmpResourceGroup) {
          tmpResourceGroup.resourceIdList = [];
          tmpResourceGroup.isAllSelected = true;
        } else {
          tmpResourceGroupList.resourceDtoList.push({
            scopeModule: item,
            resourceIdList: [],
            isAllSelected: true
          });
        }
      });
    }
  }

  configResourceGroupData(
    key: string,
    resourceSetRelationList: any[],
    type: string
  ) {
    let tmpResourceDtoList = [];
    each(this.allSelectionMap[key]?.data, item => {
      let tmpResourceDto = find(
        tmpResourceDtoList,
        resource => resource.scopeModule === item.scopeType
      );
      if (tmpResourceDto) {
        tmpResourceDto.resourceIdList.push(item.uuid);
      } else {
        tmpResourceDtoList.push({
          scopeModule: item.scopeType,
          resourceIdList: [item.uuid]
        });
      }
    });
    // 如果进入修改时有这个应用的资源组，而再次下发时没有了，则表明要清理该资源的资源组
    each(this.modifyResourceGroupList, item => {
      if (!find(tmpResourceDtoList, { scopeModule: item })) {
        tmpResourceDtoList.push({
          scopeModule: item,
          resourceIdList: []
        });
      }
    });
    resourceSetRelationList.push({
      type,
      resourceDtoList: tmpResourceDtoList
    });
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (!!this.data) {
        this.resourceSetService
          .modifyResourceSet({
            ResourceSetRequest: this.getParams()
          })
          .subscribe({
            next: () => {
              observer.next();
              observer.complete();
            },
            error: error => {
              observer.error(error);
              observer.complete();
            }
          });
      } else {
        this.resourceSetService
          .createResourceSet({
            ResourceSetRequest: this.getParams()
          })
          .subscribe({
            next: () => {
              observer.next();
              observer.complete();
            },
            error: error => {
              observer.error(error);
              observer.complete();
            }
          });
      }
    });
  }
}
