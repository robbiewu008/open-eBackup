import { DatePipe } from '@angular/common';
import { Component, Input, OnInit } from '@angular/core';
import {
  CommonConsts,
  CopiesService,
  CopyControllerService,
  DataMapService,
  extendSummaryCopiesParams,
  I18NService,
  LANGUAGE,
  LiveMountAction,
  LiveMountPolicyApiService,
  MountTargetLocation,
  RetentionPolicy,
  SchedulePolicy,
  TargetCPU,
  TargetMemory,
  VmwareService
} from 'app/shared';
import {
  assign,
  each,
  find,
  first,
  isEmpty,
  isUndefined,
  map as _map,
  reject,
  remove,
  toString
} from 'lodash';
import { combineLatest, Observable, Observer } from 'rxjs';
import { map } from 'rxjs/operators';

@Component({
  selector: 'aui-live-mount-vmware-summary',
  templateUrl: './live-mount-summary.component.html',
  styleUrls: ['./live-mount-summary.component.less'],
  providers: [DatePipe]
})
export class LiveMountSummaryComponent implements OnInit {
  info = [];
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
  @Input() activeIndex;
  @Input() componentData;
  @Input() isTask;

  constructor(
    private datePipe: DatePipe,
    private i18n: I18NService,
    private vmwareService: VmwareService,
    private dataMapService: DataMapService,
    private copiesApiService: CopiesService,
    private copyControllerService: CopyControllerService,
    private liveMountPolicyApiService: LiveMountPolicyApiService
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
    combineLatest(
      this.getResource(),
      this.getCopies(),
      this.getPolicies(),
      this.getStorages(),
      this.getNetworks()
    ).subscribe(res => {
      const parameters = JSON.parse(
        this.componentData.liveMountData.parameters
      );
      assign(this.componentData, {
        selectionResource: res[0],
        selectionCopy: res[1],
        selectionPolicy: res[2],
        selectionMount: assign(
          {
            name: parameters.name,
            power_on: parameters.config.power_on,
            startup_network_adaptor: parameters.config.startup_network_adaptor,
            bindWidthStatus:
              !isEmpty(toString(parameters.performance.min_bandwidth)) &&
              !isEmpty(toString(parameters.performance.max_bandwidth)),
            iopsStatus:
              !isEmpty(toString(parameters.performance.min_iops)) &&
              !isEmpty(toString(parameters.performance.max_iops)),
            targetCPU: parameters.config.cpu.use_original
              ? TargetCPU.OriginalConfig
              : TargetCPU.SpecifyConfig,
            targetMemory: parameters.config.memory.use_original
              ? TargetMemory.OriginalConfig
              : TargetMemory.SpecifyConfig,
            latencyStatus: !isEmpty(toString(parameters.performance.latency)),
            target_location: this.componentData.liveMountData.target_location,
            targetStorageLocationName:
              this.componentData.liveMountData.target_location ===
              MountTargetLocation.Original
                ? ''
                : find(
                    res[3],
                    item =>
                      item.uuid ===
                      parameters.config.specify_location_config.storage_location
                  )['name'],
            networkTableData:
              this.componentData.liveMountData.target_location ===
              MountTargetLocation.Original
                ? ''
                : _map(
                    parameters.config.specify_location_config.network,
                    network => {
                      return assign(network, {
                        name: network.adapter_name,
                        selectionName: find(
                          res[4],
                          item => item.uuid === network.target_network_uuid
                        )['name']
                      });
                    }
                  )
          },
          {
            ...parameters.performance,
            ...parameters.config.cpu,
            ...parameters.config.memory
          }
        )
      });
      this.getComponentData();
    });
  }

  getTaskLiveMountData() {
    const parameters = JSON.parse(
      this.componentData.liveMountData?.parameters || '{}'
    );
    // 两个位置和表格数据后续适配
    assign(this.componentData, {
      selectionMount: assign(
        {
          name: parameters.name,
          power_on: parameters.config.power_on,
          startup_network_adaptor: parameters.config.startup_network_adaptor,
          bindWidthStatus:
            !isEmpty(toString(parameters.performance.min_bandwidth)) &&
            !isEmpty(toString(parameters.performance.max_bandwidth)),
          iopsStatus:
            !isEmpty(toString(parameters.performance.min_iops)) &&
            !isEmpty(toString(parameters.performance.max_iops)),
          targetCPU: parameters.config.cpu.use_original
            ? TargetCPU.OriginalConfig
            : TargetCPU.SpecifyConfig,
          targetMemory: parameters.config.memory.use_original
            ? TargetMemory.OriginalConfig
            : TargetMemory.SpecifyConfig,
          latencyStatus: !isEmpty(toString(parameters.performance.latency)),
          target_location: parameters?.targetLocation,
          targetStorageLocationName:
            parameters?.targetLocation === MountTargetLocation.Original
              ? ''
              : find(
                  [],
                  item =>
                    item.uuid ===
                    parameters.config.specify_location_config.storage_location
                )?.name || '',
          networkTableData:
            parameters?.targetLocation === MountTargetLocation.Original
              ? ''
              : _map(
                  parameters.config.specify_location_config.network,
                  network => {
                    return assign(network, {
                      name: network.adapter_name,
                      selectionName: find(
                        [],
                        item => item.uuid === network.target_network_uuid
                      )?.name
                    });
                  }
                )
        },
        {
          ...parameters.performance,
          ...parameters.config.cpu,
          ...parameters.config.memory
        }
      )
    });
    const selectionMount = this.getSelectionMountData();
    this.info.push(selectionMount);
  }

  getComponentData() {
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

    const selectionPolicy = {
      header: this.i18n.get('common_mount_update_policy_label'),
      children: [
        {
          key: 'name',
          label: this.i18n.get('common_name_label'),
          value: this.componentData.selectionPolicy.name
        },
        {
          key: 'copyDataSelectionPolicy',
          label: this.i18n.get('common_copy_data_label'),
          value: this.componentData.selectionPolicy.copyDataSelectionPolicy
        },
        {
          key: 'scheduleInterval',
          label: this.i18n.get('common_scheduled_label'),
          value: this.componentData.selectionPolicy.scheduleInterval
            ? this.i18n.get('common_param_comma_param_label', [
                `${this.executionPeriodLabel}${this.spaceLabel}${
                  this.componentData.selectionPolicy.scheduleInterval
                }${this.spaceLabel}${this.dataMapService.getLabel(
                  'Interval_Unit',
                  this.componentData.selectionPolicy.scheduleIntervalUnit
                )}`,
                `${this.firstExecuteTimeLabel}${
                  this.spaceLabel
                }${this.datePipe.transform(
                  this.componentData.selectionPolicy.scheduleStartTime,
                  'yyyy-MM-dd HH:mm:ss'
                )}`
              ])
            : '--'
        },
        {
          key: 'retentionValue',
          label: this.i18n.get('common_retention_label'),
          value: this.componentData.selectionPolicy.retentionValue
        },
        {
          key: 'liveMountCount',
          label: this.i18n.get('explore_account_of_object_label'),
          value: this.componentData.selectionPolicy.liveMountCount
        }
      ]
    };

    const selectionMount = this.getSelectionMountData();
    this.info.push(
      selectionResource,
      selectionCopy,
      selectionPolicy,
      selectionMount
    );
  }

  getSelectionMountData() {
    const selectionMount = {
      header: this.i18n.get('common_mount_options_label'),
      children: [
        {
          key: 'target_location',
          label: this.i18n.get('protection_mount_to_label'),
          value: this.componentData.selectionMount.target_location
        }
      ]
    };

    if (
      this.componentData.selectionMount.target_location ===
      MountTargetLocation.Original
    ) {
      selectionMount.children = selectionMount.children.concat([
        {
          key: 'originLocation',
          label: this.i18n.get('common_location_label'),
          value:
            this.componentData.selectionResource.original_location ||
            this.componentData.liveMountData.target_resource_path
        },
        {
          key: 'vmName',
          label: this.i18n.get('protection_vm_name_label'),
          value: isUndefined(this.componentData.selectionMount.isOverwrite)
            ? this.componentData.selectionMount.name
            : !this.componentData.selectionMount.isOverwrite
            ? this.componentData.selectionMount.name
            : this.componentData.selectionResource.resource_name
        },
        {
          key: 'targetCPU',
          label: this.i18n.get('common_target_cpu_label'),
          value: ''
        },
        {
          key: 'targetMemory',
          label: this.i18n.get('explore_target_memory_label'),
          value: ''
        },
        {
          key: 'startupNetworkAdaptor',
          label: this.i18n.get('protection_startup_network_adaptor_label'),
          value: this.dataMapService.getLabel(
            'Switch_Status',
            this.componentData.selectionMount.startup_network_adaptor
          )
        },
        {
          key: 'powerOn',
          label: this.i18n.get('protection_mount_auto_power_on_label'),
          value: this.dataMapService.getLabel(
            'Switch_Status',
            this.componentData.selectionMount.power_on
          )
        }
      ]);
    } else {
      selectionMount.children = selectionMount.children.concat([
        {
          key: 'vmName',
          label: this.i18n.get('protection_vm_name_label'),
          value: this.componentData.selectionMount.name
        },
        {
          key: 'computerLocation',
          label: this.i18n.get('protection_computer_location_label'),
          value:
            this.componentData.selectionMount.location ||
            this.componentData.liveMountData.target_resource_path
        },
        {
          key: 'targetStorageLocation',
          label: this.i18n.get('protection_vm_storage_location_label'),
          value: this.componentData.selectionMount.targetStorageLocationName
        },
        {
          key: 'networkLocation',
          label: this.i18n.get('common_network_location_label'),
          value: ''
        },
        {
          key: 'targetCPU',
          label: this.i18n.get('common_target_cpu_label'),
          value: ''
        },
        {
          key: 'targetMemory',
          label: this.i18n.get('explore_target_memory_label'),
          value: ''
        },
        {
          key: 'startupNetworkAdaptor',
          label: this.i18n.get('protection_startup_network_adaptor_label'),
          value: this.dataMapService.getLabel(
            'Switch_Status',
            this.componentData.selectionMount.startup_network_adaptor
          )
        },
        {
          key: 'powerOn',
          label: this.i18n.get('protection_mount_auto_power_on_label'),
          value: this.dataMapService.getLabel(
            'Switch_Status',
            this.componentData.selectionMount.power_on
          )
        }
      ]);
      if (isEmpty(this.componentData.selectionMount.networkTableData)) {
        selectionMount.children = reject(selectionMount.children, item => {
          return item.key === 'networkLocation';
        });
      }
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

  getResource() {
    const params = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        resourceSubType: this.componentData.childResourceType,
        resource_id: this.componentData.liveMountData.resource_id
      })
    };
    return this.copyControllerService.queryCopySummaryResourceV2(params).pipe(
      map(res => {
        each(res.records, item => extendSummaryCopiesParams(item));
        return first(res.records);
      })
    );
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

  getPolicies() {
    if (!this.componentData.liveMountData.policy_id) {
      return new Observable<any>((observer: Observer<any>) => {
        observer.next({});
        observer.complete();
      });
    }
    return this.liveMountPolicyApiService.getPolicyUsingGET({
      policyId: this.componentData.liveMountData.policy_id
    });
  }

  getStorages() {
    if (
      this.componentData.liveMountData.target_location ===
      MountTargetLocation.Original
    ) {
      return new Observable<any>((observer: Observer<any>) => {
        observer.next([]);
        observer.complete();
      });
    }
    return this.vmwareService.listComputeResDatastoreV1ComputeResourcesComputeResUuidDatastoresGet(
      {
        computeResUuid: this.componentData.liveMountData.target_resource_id
      }
    );
  }

  getNetworks() {
    if (
      this.componentData.liveMountData.target_location ===
      MountTargetLocation.Original
    ) {
      return new Observable<any>((observer: Observer<any>) => {
        observer.next([]);
        observer.complete();
      });
    }
    return this.vmwareService.listComputeResNetworkV1ComputeResourcesComputeResUuidNetworksGet(
      {
        computeResUuid: this.componentData.liveMountData.target_resource_id
      }
    );
  }
}
