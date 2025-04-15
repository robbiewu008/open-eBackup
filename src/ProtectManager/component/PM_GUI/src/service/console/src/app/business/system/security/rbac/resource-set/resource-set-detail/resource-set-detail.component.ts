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
import { Component, Input, OnInit } from '@angular/core';
import {
  allAppType,
  ApplicationType,
  ClientManagerApiService,
  DataMap,
  DataMapService,
  GlobalService,
  I18NService,
  ProtectedResourceApiService,
  ResourceSetApiService,
  ResourceSetType,
  ResourceType,
  RouterUrl
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  defer,
  each,
  filter,
  find,
  isArray,
  isString,
  set,
  size,
  some
} from 'lodash';

@Component({
  selector: 'aui-resource-set-detail',
  templateUrl: './resource-set-detail.component.html',
  styleUrls: ['./resource-set-detail.component.less']
})
export class ResourceSetDetailComponent implements OnInit {
  @Input() openPage;
  @Input() data;

  tabActiveIndex = 'resources';
  allSelectedApps = [];
  allSelectionMap = {};
  resourceSetType = ResourceSetType;
  isSLA = false; // 用于标识是否需要展示
  isNoData = false;
  deviceOptions = this.dataMapService.toArray('Device_Storage_Type');
  agentOptions = this.dataMapService.toArray('Host_Proxy_Type');

  // 所有tab的名称
  appNameMap = {
    database: this.i18n.get('common_database_label'),
    bigData: this.i18n.get('common_bigdata_label'),
    virtualization: this.i18n.get('common_virtualization_label'),
    container: this.i18n.get('common_container_label'),
    cloud: this.i18n.get('common_huawei_clouds_label'),
    application: this.i18n.get('common_application_label'),
    fileService: this.i18n.get('common_file_system_label'),
    AGENT: this.i18n.get('protection_client_label'),
    SLA: 'SLA',
    QOS: this.i18n.get('common_limit_rate_policy_label'),
    AIR_GAP: 'Air Gap',
    PREVENT_EXTORTION_AND_WORM:
      this.i18n.get('deploy_type') === DataMap.Deploy_Type.x3000.value
        ? this.i18n.get('common_worm_policy_label')
        : this.i18n.get('common_anti_policy_label'),
    REPORT: this.i18n.get('common_report_label'),
    LIVE_MOUNT_POLICY: this.i18n.get('common_mount_update_policy_label')
  };
  singleLayerApp = allAppType.singleLayerApp;
  virtualCloudApp = allAppType.virtualCloudApp;
  hasCopyData = false;

  noDataTip = this.i18n.get('system_resourceset_no_data_label');

  constructor(
    public globalService: GlobalService,
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private appUtilsService: AppUtilsService,
    private resourceSetService: ResourceSetApiService,
    private clientManagerApiService: ClientManagerApiService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit(): void {
    this.getResource();
  }

  getResource() {
    this.resourceSetService
      .queryResourceSetTypeCount({
        resourceSetId: this.data.uuid
      })
      .subscribe((res: any) => {
        if (!res) {
          return;
        }

        this.getApp(res);
        this.isNoData = !this.allSelectedApps.length;
      });
  }

  getApp(res) {
    // 展示被选中的应用
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
      resourceSetType: ResourceSetType.StorageEquipment,
      jobTargetType: [
        DataMap.Job_Target_Type.DoradoV7.value,
        DataMap.Job_Target_Type.OceanStorDoradoV7.value,
        DataMap.Job_Target_Type.OceanStorDorado_6_1_3.value,
        DataMap.Job_Target_Type.OceanStor_6_1_3.value,
        DataMap.Job_Target_Type.OceanStor_v5.value,
        DataMap.Job_Target_Type.OceanStorPacific.value,
        DataMap.Job_Target_Type.OceanStorDorado.value,
        DataMap.Job_Target_Type.OceanProtect.value
      ]
    });
    let otherApp = [
      ResourceSetType.Agent,
      ResourceSetType.SLA,
      ResourceSetType.QOS,
      ResourceSetType.Worm,
      ResourceSetType.AirGap,
      ResourceSetType.Report,
      ResourceSetType.LiveMount
    ];
    each(otherApp, item => {
      set(allApp, `${item}`, { type: item });
    });
    for (const key in allApp) {
      // 我们从总应用里取出需要展示的应用，并把数据塞进去
      let appList = [];
      let allNum = 0;
      if (isArray(allApp[key])) {
        [appList, allNum] = this.parseNormalApp(
          res,
          allApp,
          key,
          appList,
          allNum
        );
      } else {
        this.parseOtherApp(res, allApp, key);
      }

      if (appList.length) {
        // 这里是可保护资源们的处理
        this.allSelectedApps.push({
          label: this.appNameMap[key],
          apps: appList,
          num: allNum
        });
      }
    }
    each(this.allSelectedApps, item => {
      if (
        !!item?.apps &&
        find(item.apps, val =>
          [
            ResourceSetType.NasFileSystem,
            ResourceSetType.NasShare,
            ResourceSetType.StorageEquipment
          ].includes(val.resourceSetType)
        )
      ) {
        // 让组件先获取数据更新数量
        defer(() => {
          this.globalService.emitStore({
            action: this.i18n.get('protection_storage_device_label'),
            state: true
          });
          this.globalService.emitStore({
            action: this.i18n.get('common_nas_shares_label'),
            state: true
          });
          this.globalService.emitStore({
            action: this.i18n.get('common_nas_file_systems_label'),
            state: true
          });
        });
      }

      // 这三个虚拟化应用无法直接根据子资源类型来筛选数量，需要直接获取左树第一层的数量去减
      this.parseVirtualAppNum(item);

      // openstack和hcs由于有visible这个的存在，全选会导致选上界面上不显示的域，所以需要获取后去掉数量
      if (
        !!item?.apps &&
        find(
          item.apps,
          val => ResourceSetType.OpenStack === val.resourceSetType
        )
      ) {
        this.getCloudAccurateNum(ResourceSetType.OpenStack);
      }

      if (
        !!item?.apps &&
        find(item.apps, val => ResourceSetType.HCSStack === val.resourceSetType)
      ) {
        this.getCloudAccurateNum(ResourceSetType.HCSStack);
      }

      // 这几个应用有集群节点的概念，和该应用的另一子资源类型相同，所以需要实时获取
      this.parseSpecialDatabaseNum(item);
    });

    this.allSelectedApps.push({
      label: this.i18n.get('common_copies_label'),
      type: 'copy'
    });
  }

  private parseSpecialDatabaseNum(item: any) {
    if (!item?.apps) {
      return;
    }
    each(
      [
        ResourceSetType.PostgreSQL,
        ResourceSetType.Informix,
        ResourceSetType.KingBase,
        ResourceSetType.MySQL,
        ResourceSetType.DB2
      ],
      appType => {
        const tmpApp = find(item?.apps, { resourceSetType: appType });
        if (!!tmpApp) {
          item.num -= tmpApp.count;
          tmpApp.count = 0;
          defer(() => {
            this.globalService.emitStore({
              action: tmpApp.label,
              state: true
            });
          });
        }
      }
    );
  }

  private parseVirtualAppNum(item: any) {
    each(
      [
        ResourceSetType.FusionCompute,
        ResourceSetType.FusionOne,
        ResourceSetType.HyperV
      ],
      appType => {
        if (
          !!item?.apps &&
          some(item?.apps, val => appType === val.resourceSetType)
        ) {
          this.getCloudVirtualTopNum(appType);
        }
      }
    );
  }

  parseNormalApp(res, allApp, key, appList, allNum) {
    // 可保护资源
    each(allApp[key], app => {
      // 存储设备与nas有父子关系，需要单独处理
      // 部分应用与客户端有父子关系，需要处理,这里客户端有插件类型和普通类型
      let tmpApp = [];
      if (
        [ResourceSetType.NasFileSystem, ResourceSetType.NasShare].includes(
          app.resourceSetType
        )
      ) {
        tmpApp = filter(
          res,
          val =>
            [app.resourceSetType, ResourceSetType.StorageEquipment].includes(
              val.scopeModule
            ) &&
            (val.resourceSubType === app.resType ||
              (val.resourceSubType === DataMap.Resource_Type.ndmp.value &&
                app.resourceSetType === ResourceSetType.NasFileSystem)) &&
            this.filterAgent(val)
        );
      } else if (app.resourceSetType === ResourceSetType.StorageEquipment) {
        tmpApp = filter(
          res,
          val =>
            [
              ResourceSetType.StorageEquipment,
              ResourceSetType.NasFileSystem,
              ResourceSetType.NasShare
            ].includes(val.scopeModule) &&
            some(this.deviceOptions, { value: val.resourceSubType }) &&
            this.filterAgent(val)
        );
      } else {
        tmpApp = filter(
          res,
          val =>
            (val.scopeModule === app.resourceSetType ||
              (val.scopeModule === ResourceSetType.FilesetTemplate &&
                app.resourceSetType === ResourceSetType.Fileset)) &&
            !(
              this.virtualCloudApp.includes(app.slaId) &&
              !!app.resourceSetKey &&
              !app?.resourceSetKey.includes(val.resourceSubType)
            ) &&
            this.filterAgent(val)
        );
      }

      if (!!tmpApp.length) {
        each(tmpApp, val => {
          app.count += val.resourceNum;
          allNum += val.resourceNum;
        });
        appList.push(app);
      }
    });
    return [appList, allNum];
  }

  private filterAgent(val: any): boolean {
    return !(
      val.scopeModule !== ResourceSetType.Agent &&
      (some(this.agentOptions, { value: val.resourceSubType }) ||
        (isString(val?.resourceSubType) &&
          val.resourceSubType.includes('Plugin')))
    );
  }

  parseOtherApp(res, allApp, key) {
    // 其他资源
    let tmpApp = filter(
      res,
      item =>
        item.scopeModule === allApp[key].type ||
        (item.scopeModule === ResourceSetType.ReportSubscription &&
          allApp[key].type === ResourceSetType.Report)
    );
    if (!!tmpApp.length) {
      this.allSelectedApps.push({
        type: key,
        num: tmpApp.reduce((accumulator, { resourceNum }) => {
          return accumulator + resourceNum;
        }, 0),
        label: this.appNameMap[key]
      });
    }
  }

  getCloudVirtualTopNum(resourceSetType) {
    const tmpParams = { resourceSetId: this.data.uuid };
    switch (resourceSetType) {
      case ResourceSetType.HyperV:
        assign(tmpParams, {
          type: ResourceType.Virtualization,
          subType: [
            DataMap.Resource_Type.hyperVScvmm.value,
            DataMap.Resource_Type.hyperVCluster.value,
            DataMap.Resource_Type.hyperVHost.value
          ]
        });
        break;
      case ResourceSetType.FusionCompute:
        assign(tmpParams, {
          subType: ResourceType.FUSION_COMPUTE,
          type: ResourceType.PLATFORM
        });
        break;
      case ResourceSetType.FusionOne:
        assign(tmpParams, {
          subType: ResourceType.FUSION_ONE,
          type: ResourceType.PLATFORM
        });
        break;
    }
    const extParams = {
      conditions: JSON.stringify(tmpParams)
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        const tmpVirtualization = find(this.allSelectedApps, {
          label: this.i18n.get('common_virtualization_label')
        });
        const tmpApp = find(tmpVirtualization.apps, {
          resourceSetType: resourceSetType
        });
        tmpApp.count -= size(resource);
        tmpVirtualization.num -= size(resource);
      }
    );
  }

  getCloudAccurateNum(resourceSetType) {
    // hcs和openstack有visible属性，但资源集父选子不会区分，所以需要手动获取去删除
    const params = {
      pageNo: 0,
      pageSize: 2000
    };
    const defaultConditions = {
      visible: '0'
    };

    if (resourceSetType === ResourceSetType.OpenStack) {
      assign(defaultConditions, {
        subType: [ResourceType.OpenStackDomain],
        resourceSetId: this.data.uuid
      });
    } else if (resourceSetType === ResourceSetType.HCSStack) {
      assign(defaultConditions, {
        subType: DataMap.Resource_Type.HCSTenant.value,
        type: ResourceType.TENANT,
        resourceSetId: this.data.uuid
      });
    }

    assign(params, {
      conditions: JSON.stringify(defaultConditions)
    });
    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      let tmpCloud = find(this.allSelectedApps, {
        label: this.i18n.get('common_huawei_clouds_label')
      });
      let tmpApp = find(tmpCloud.apps, { resourceSetType: resourceSetType });
      tmpApp.count -= res.totalCount;
      tmpCloud.num -= res.totalCount;
    });
  }

  specialNumChange(e) {
    // 有些应用可能需要单独获取一次实际数量，资源集父传子可能选中的数量不准确
    let tmpApplication;
    if (
      [
        ResourceSetType.PostgreSQL,
        ResourceSetType.Informix,
        ResourceSetType.KingBase,
        ResourceSetType.MySQL,
        ResourceSetType.DB2
      ].includes(e.appType)
    ) {
      tmpApplication = find(this.allSelectedApps, {
        label: this.i18n.get('common_database_label')
      });
    }
    let tmpApp = find(tmpApplication.apps, { resourceSetType: e.appType });
    tmpApp.count += e.num;
    tmpApplication.num += e.num;
  }

  nasNumChange(e) {
    // nas三兄弟从内而外更新数量数据，这里还要同步更新文件系统总数量
    let diffNum = 0;
    let tmpFileSystem = find(this.allSelectedApps, {
      label: this.i18n.get('common_file_system_label')
    });
    let tmpApp = find(tmpFileSystem.apps, { resourceSetType: e.appType });
    diffNum = e.num - tmpApp.count;
    tmpApp.count = e.num;
    tmpFileSystem.num += diffNum;
  }

  agentNumChange(e) {
    // 会有跟随资源授权而授权的客户端，所以得直接查数量

    let tmpClient = find(this.allSelectedApps, {
      label: this.i18n.get('protection_client_label')
    });
    tmpClient.num = e.num;
  }

  copyNumChange(e) {
    let tmpCopy = this.allSelectedApps[this.allSelectedApps.length - 1];
    if (e === 0 && !this.hasCopyData) {
      // 当没有副本时就不展示副本一栏了
      if (find(this.allSelectedApps, item => item.type === 'copy')) {
        this.allSelectedApps.pop();
      }
    } else if (!tmpCopy?.num) {
      // 防止搜索等筛选条件干掉总数量
      tmpCopy.num = e;
      this.hasCopyData = true;
    }
    this.isNoData = !this.allSelectedApps.length;
  }

  beforeExpanded = collapse => {
    let messageTitle = collapse.lvTitle.split(' ');
    messageTitle.pop();
    messageTitle = messageTitle.join(' ');
    this.globalService.emitStore({
      action: messageTitle,
      state: true
    });
  };

  goBack() {
    this.openPage.emit();
  }
}
