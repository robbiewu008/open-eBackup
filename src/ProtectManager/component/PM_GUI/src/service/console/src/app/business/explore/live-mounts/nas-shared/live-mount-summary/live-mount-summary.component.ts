import { DatePipe } from '@angular/common';
import { Component, Input, OnInit } from '@angular/core';
import {
  CommonConsts,
  CopiesService,
  DataMap,
  DataMapService,
  I18NService,
  LANGUAGE,
  LiveMountAction,
  LocalStorageApiService,
  RetentionPolicy,
  SchedulePolicy,
  TargetCPU,
  TargetMemory
} from 'app/shared';
import {
  assign,
  find,
  first,
  isArray,
  isEmpty,
  map as _map,
  reject,
  remove,
  toString as _toString
} from 'lodash';
import { map } from 'rxjs/operators';

@Component({
  selector: 'aui-live-mount-nas-shared-summary',
  templateUrl: './live-mount-summary.component.html',
  styleUrls: ['./live-mount-summary.component.less'],
  providers: [DatePipe]
})
export class LiveMountSummaryComponent implements OnInit {
  @Input() activeIndex;
  @Input() componentData;
  @Input() isTask;
  schedulePolicy = SchedulePolicy;
  retentionPolicy = RetentionPolicy;
  targetCPU = TargetCPU;
  targetMemory = TargetMemory;
  spaceLabel = this.i18n.language === LANGUAGE.CN ? '' : ' ';
  executionPeriodLabel = this.i18n.get(
    'protection_execution_period_label',
    [],
    true
  );
  firstExecuteTimeLabel = this.i18n.get(
    'explore_first_execute_label',
    [],
    true
  );

  info = [];

  constructor(
    private datePipe: DatePipe,
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private copiesApiService: CopiesService,
    private LocalStorageApiService: LocalStorageApiService
  ) {}

  ngOnInit() {
    this.getSummaryData();
  }

  getSummaryData() {
    if (this.activeIndex === 0) {
      return;
    }

    this.info = [];
    if (this.componentData.action === LiveMountAction.View) {
      if (!this.isTask) {
        this.getLiveMountData();
      } else {
        this.getTaskLiveMountData();
      }
    } else {
      this.getComponentData();
    }
  }

  getLiveMountData() {
    this.getCopies().subscribe(res => {
      const parameters = JSON.parse(
        this.componentData.liveMountData.parameters
      );
      const nfsInfo =
        find(
          JSON.parse(this.componentData.liveMountData.file_system_share_info),
          { type: +DataMap.Shared_Mode.nfs.value }
        ) || {};
      const cifsInfo =
        find(
          JSON.parse(this.componentData.liveMountData.file_system_share_info),
          { type: +DataMap.Shared_Mode.cifs.value }
        ) || {};
      assign(this.componentData, {
        selectionResource: {
          resource_name: this.componentData.liveMountData.resource_name,
          resource_location: this.componentData.liveMountData.resource_path
        },
        selectionCopy: res,
        selectionPolicy: {},
        selectionMount: assign(
          {
            name: nfsInfo.fileSystemName
              ? nfsInfo.fileSystemName.replace('mount_', '')
              : cifsInfo.fileSystemName
              ? cifsInfo.fileSystemName.replace('mount_', '')
              : '',
            nfsEnable: !isEmpty(nfsInfo),
            cifsEnable: !isEmpty(cifsInfo),
            clientType: nfsInfo.advanceParams?.clientType,
            client: nfsInfo.advanceParams?.clientName,
            unixType: nfsInfo?.accessPermission,
            rootType: nfsInfo.advanceParams?.rootSquash,
            cifsShareName: cifsInfo.advanceParams?.shareName,
            nfsShareName: nfsInfo.advanceParams?.sharePath || '--',
            userType: 2,
            userName: cifsInfo.advanceParams?.usernames,
            permissionType: cifsInfo?.accessPermission,
            bindWidthStatus:
              !isEmpty(_toString(parameters.performance.min_bandwidth)) &&
              !isEmpty(_toString(parameters.performance.max_bandwidth)),
            iopsStatus:
              !isEmpty(_toString(parameters.performance.min_iops)) &&
              !isEmpty(_toString(parameters.performance.max_iops)),
            latencyStatus: !isEmpty(_toString(parameters.performance.latency))
          },
          parameters.performance
        )
      });
      this.LocalStorageApiService.getLogicPortUsingGET({
        akDoException: false,
        protocol:
          !isEmpty(nfsInfo) && !isEmpty(cifsInfo)
            ? '3'
            : !isEmpty(nfsInfo)
            ? '1,3'
            : '2,3'
      }).subscribe(
        res => {
          this.getComponentData(res.logicPortList);
        },
        err => {
          this.getComponentData();
        }
      );
    });
  }

  getTaskLiveMountData() {
    const parameters = JSON.parse(
      this.componentData.liveMountData?.parameters || '{}'
    );
    const nfsInfo =
      find(
        JSON.parse(
          this.componentData.liveMountData?.file_system_share_info || '{}'
        ),
        { type: +DataMap.Shared_Mode.nfs.value }
      ) || {};
    const cifsInfo =
      find(
        JSON.parse(
          this.componentData.liveMountData?.file_system_share_info || '{}'
        ),
        { type: +DataMap.Shared_Mode.cifs.value }
      ) || {};
    assign(this.componentData, {
      selectionMount: assign(
        {
          name: nfsInfo.fileSystemName
            ? nfsInfo.fileSystemName.replace('mount_', '')
            : cifsInfo.fileSystemName
            ? cifsInfo.fileSystemName.replace('mount_', '')
            : '',
          nfsEnable: !isEmpty(nfsInfo),
          cifsEnable: !isEmpty(cifsInfo),
          clientType: nfsInfo.advanceParams?.clientType,
          client: nfsInfo.advanceParams?.clientName,
          unixType: nfsInfo?.accessPermission,
          rootType: nfsInfo.advanceParams?.rootSquash,
          cifsShareName: cifsInfo.advanceParams?.shareName,
          nfsShareName: nfsInfo.advanceParams?.sharePath || '--',
          userType: 2,
          userName: cifsInfo.advanceParams?.usernames,
          permissionType: cifsInfo?.accessPermission,
          bindWidthStatus:
            !isEmpty(_toString(parameters.performance.min_bandwidth)) &&
            !isEmpty(_toString(parameters.performance.max_bandwidth)),
          iopsStatus:
            !isEmpty(_toString(parameters.performance.min_iops)) &&
            !isEmpty(_toString(parameters.performance.max_iops)),
          latencyStatus: !isEmpty(_toString(parameters.performance.latency))
        },
        parameters.performance
      )
    });
    const selectionMount = this.getSelectionMountData([], []);
    this.info.push(selectionMount);
  }

  getComponentData(logicPortList?) {
    const selectionResource = {
      header: this.i18n.get('common_resource_label'),
      children: [
        {
          key: 'resource_name',
          label: this.i18n.get('common_name_label'),
          value: this.componentData.selectionResource.resource_name
        },
        {
          key: 'resource_location',
          label: this.i18n.get('common_location_label'),
          value: this.componentData.selectionResource.resource_location
        }
      ]
    };
    const selectionCopy = {
      header: this.i18n.get('common_copy_data_label'),
      children: [
        {
          key: 'display_timestamp',
          label: this.i18n.get('common_time_stamp_label'),
          value: this.datePipe.transform(
            this.componentData.selectionCopy.display_timestamp,
            'yyyy-MM-dd HH:mm:ss'
          )
        },
        {
          key: 'cascade_level',
          label: this.i18n.get('explore_generation_label'),
          value: this.componentData.selectionCopy.generation
        },
        {
          key: 'status',
          label: this.i18n.get('common_status_label'),
          value: this.dataMapService.getLabel(
            'copydata_validStatus',
            this.componentData.selectionCopy.status
          )
        },
        {
          key: 'location',
          label: this.i18n.get('common_location_label'),
          value: this.componentData.selectionCopy.location
        },
        {
          key: 'generated_by',
          label: this.i18n.get('common_generated_type_label'),
          value: this.dataMapService.getLabel(
            'CopyData_generatedType',
            this.componentData.selectionCopy.generated_by
          )
        }
      ]
    };
    let nfsInfo = [];
    let cifsInfo = [];
    const allInfo = [];
    if (this.componentData.selectionMount.nfsEnable) {
      nfsInfo.push(
        { key: 'mode', value: 'NFS' },
        {
          key: 'shareName',
          label: this.i18n.get('protection_share_path_info_label'),
          value:
            this.componentData.selectionMount.nfsShareName ||
            `mount_${this.componentData.selectionMount.name}`
        },
        {
          key: 'client',
          label: this.i18n.get('protection_share_client_label'),
          value: this.componentData.selectionMount.client
        },
        {
          key: 'unixType',
          label: this.i18n.get('protection_unix_permission_label'),
          value: this.dataMapService.getLabel(
            'unixPermission',
            this.componentData.selectionMount.unixType
          )
        },
        {
          key: 'rootType',
          label: this.i18n.get('protection_root_permission_label'),
          value: this.dataMapService.getLabel(
            'rootPermission',
            this.componentData.selectionMount.rootType
          )
        }
      );
      allInfo.push(nfsInfo);
    }
    if (this.componentData.selectionMount.cifsEnable) {
      cifsInfo.push(
        { key: 'mode', value: 'CIFS' },
        {
          key: 'shareName',
          label: this.i18n.get('explore_share_name_label'),
          value: this.componentData.selectionMount.cifsShareName
        },
        {
          key: 'userType',
          label: this.i18n.get('common_type_label'),
          value: this.dataMapService.getLabel(
            'Cifs_Domain_Client_Type',
            this.componentData.selectionMount.userType
          )
        },
        {
          key: 'userName',
          label: this.i18n.get('common_username_label'),
          value: isArray(this.componentData.selectionMount.userName)
            ? this.componentData.selectionMount.userName.join(';')
            : this.componentData.selectionMount.userName
        },
        {
          key: 'permissionType',
          label: this.i18n.get('protection_permission_level_label'),
          value: this.dataMapService.getLabel(
            'permissionLevel',
            this.componentData.selectionMount.permissionType
          )
        }
      );
      if (
        this.componentData.selectionMount.userType ===
        DataMap.Cifs_Domain_Client_Type.everyone.value
      ) {
        cifsInfo = reject(cifsInfo, item => {
          return item.key === 'userName';
        });
      }
      allInfo.push(cifsInfo);
    }
    const selectionMount = this.getSelectionMountData(allInfo, logicPortList);
    this.info.push(selectionResource, selectionCopy, selectionMount);
  }

  getSelectionMountData(allInfo, logicPortList) {
    const selectionMount = {
      header: this.i18n.get('common_mount_options_label'),
      children: [
        {
          key: 'fileSystemName',
          label: this.i18n.get('protection_file_system_name_label'),
          value: `mount_${this.componentData.selectionMount.name}`
        },
        {
          key: 'shareMode',
          label: this.i18n.get('explore_share_protocol_label'),
          value: allInfo
        }
      ]
    };
    if (
      this.componentData.action === LiveMountAction.View &&
      !isEmpty(logicPortList)
    ) {
      selectionMount.children = selectionMount.children.concat([
        {
          key: 'shareIp',
          label: this.i18n.get('protection_shared_ip_label'),
          value: _map(logicPortList, item => {
            return { ip: item };
          })
        }
      ]);
    }
    selectionMount.children = selectionMount.children.concat([
      {
        key: 'bindWidth',
        label: this.i18n.get('common_bindwidth_label'),
        value: ''
      },
      {
        key: 'iops',
        label: this.i18n.get('protection_nor_iops_label'),
        value: ''
      },
      {
        key: 'burstTime',
        label: this.i18n.get('explore_max_burst_label'),
        value: this.componentData.selectionMount.burst_time
      },
      {
        key: 'latency',
        label: this.i18n.get('protection_nor_latency_label'),
        value: this.dataMapService.getLabel(
          'LiveMount_Latency',
          this.componentData.selectionMount.latency
        )
      }
    ]);
    if (
      !(
        this.componentData.selectionMount.burst_iops ||
        this.componentData.selectionMount.burst_bandwidth
      )
    ) {
      remove(selectionMount.children, item => {
        return item.key === 'burstTime';
      });
    }
    return selectionMount;
  }

  getCopies() {
    const params = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        uuid: this.componentData.liveMountData.copy_id,
        resource_sub_type: this.componentData.childResourceType
      })
    };
    return this.copiesApiService.queryResourcesV1CopiesGet(params).pipe(
      map(res => {
        return first(res.items);
      })
    );
  }
}
