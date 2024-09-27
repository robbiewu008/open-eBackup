import { Component, Input, OnInit } from '@angular/core';
import {
  allAppType,
  ApplicationType,
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

  noDataTip = this.i18n.get('system_resourceset_no_data_label');

  constructor(
    public globalService: GlobalService,
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private appUtilsService: AppUtilsService,
    private resourceSetService: ResourceSetApiService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit(): void {
    this.getResource();
  }

  getResource() {
    this.resourceSetService
      .QueryResourceSetTypeCount({
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
      resourceSetType: ResourceSetType.StorageEquipment
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
    });

    this.allSelectedApps.push({
      label: this.i18n.get('common_copies_label'),
      type: 'copy'
    });
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
            val.resourceSubType === app.resType &&
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
    let tmpApp = find(res, { scopeModule: allApp[key].type });
    if (tmpApp) {
      this.allSelectedApps.push({
        type: key,
        num: tmpApp.resourceNum,
        label: this.appNameMap[key]
      });
    }
  }

  getCloudAccurateNum(resourceSetType) {
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
        type: ResourceType.TENANT
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

  copyNumChange(e) {
    let tmpCopy = this.allSelectedApps[this.allSelectedApps.length - 1];
    if (e === 0) {
      // 当没有副本时就不展示副本一栏了
      if (find(this.allSelectedApps, item => item.type === 'copy')) {
        this.allSelectedApps.pop();
      }
    } else if (!tmpCopy?.num) {
      // 防止搜索等筛选条件干掉总数量
      tmpCopy.num = e;
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
