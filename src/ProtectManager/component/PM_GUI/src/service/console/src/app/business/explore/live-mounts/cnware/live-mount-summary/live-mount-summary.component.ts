import { DatePipe } from '@angular/common';
import { Component, Input, OnInit } from '@angular/core';
import {
  CommonConsts,
  CopiesService,
  DataMapService,
  I18NService,
  LANGUAGE,
  LiveMountAction,
  LiveMountPolicyApiService,
  MountTargetLocation,
  RetentionPolicy,
  SchedulePolicy,
  TargetCPU,
  TargetMemory
} from 'app/shared';
import {
  assign,
  first,
  isArray,
  isEmpty,
  remove,
  toString as _toString
} from 'lodash';
import { forkJoin, Observable, Observer } from 'rxjs';
import { map } from 'rxjs/operators';

@Component({
  selector: 'aui-live-mount-cnware-summary',
  templateUrl: './live-mount-summary.component.html',
  styleUrls: ['./live-mount-summary.component.less'],
  providers: [DatePipe]
})
export class LiveMountSummaryComponent implements OnInit {
  @Input() activeIndex;
  @Input() componentData;
  @Input() isTask;

  targetCPU = TargetCPU;
  targetMemory = TargetMemory;
  schedulePolicy = SchedulePolicy;
  retentionPolicy = RetentionPolicy;
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
    private i18n: I18NService,
    private datePipe: DatePipe,
    private dataMapService: DataMapService,
    private copiesApiService: CopiesService,
    private liveMountPolicyApiService: LiveMountPolicyApiService
  ) {}

  ngOnInit(): void {
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

  getTaskLiveMountData() {
    const parameters = JSON.parse(
      this.componentData.liveMountData?.parameters || '{}'
    );
    assign(this.componentData, {
      selectionMount: assign(
        {
          name: parameters.name,
          power_on: parameters.config?.power_on || true,
          target_location: parameters?.target_location,
          targetCPU: parameters?.config?.cpu?.use_original
            ? TargetCPU.OriginalConfig
            : TargetCPU.SpecifyConfig,
          targetMemory: parameters?.config.memory.use_original
            ? TargetMemory?.OriginalConfig
            : TargetMemory?.SpecifyConfig,
          num_virtual_sockets: parameters.config?.cpu?.num_virtual_sockets,
          num_cores_per_virtual: parameters.config?.cpu?.num_cores_per_virtual,
          memory_size: parameters.config?.memory?.memory_size || '',
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
    // 用于任务即时挂载展示
    const selectionMount = this.getSelectionMount();
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
    const selectionMount = this.getSelectionMount();
    this.info.push(
      selectionResource,
      selectionCopy,
      selectionPolicy,
      selectionMount
    );
  }

  getSelectionMount() {
    const selectionMount = {
      header: this.i18n.get('common_mount_options_label'),
      children: [
        {
          key: 'target_location',
          label: this.i18n.get('protection_mount_to_label'),
          value:
            this.componentData.selectionMount?.target_location ===
            MountTargetLocation.Original
              ? this.i18n.get('common_restore_to_origin_location_label')
              : this.i18n.get('common_restore_to_new_location_label')
        }
      ]
    };
    if (
      this.componentData.selectionMount?.target_location ===
      MountTargetLocation.Original
    ) {
      selectionMount.children = selectionMount.children.concat([
        {
          key: 'originLocation',
          label: this.i18n.get('common_location_label'),
          value:
            this.componentData.selectionResource?.resource_location ||
            this.componentData.liveMountData?.target_resource_path
        },
        {
          key: 'vmName',
          label: this.i18n.get('protection_vm_name_label'),
          value: this.componentData.selectionMount?.name
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
          key: 'powerOn',
          label: this.i18n.get('protection_mount_auto_power_on_label'),
          value: this.dataMapService.getLabel(
            'Switch_Status',
            this.componentData.selectionMount?.power_on
          )
        }
      ]);
    } else {
      selectionMount.children = selectionMount.children.concat([
        {
          key: 'vmName',
          label: this.i18n.get('protection_vm_name_label'),
          value: this.componentData.selectionMount?.name
        },
        {
          key: 'computerLocation',
          label: this.i18n.get('protection_computer_location_label'),
          value: isArray(this.componentData.selectionMount?.computerLocation)
            ? this.componentData.selectionMount?.computerLocation[0]?.path
            : this.componentData.liveMountData?.target_resource_path
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
          key: 'powerOn',
          label: this.i18n.get('protection_mount_auto_power_on_label'),
          value: this.dataMapService.getLabel(
            'Switch_Status',
            this.componentData.selectionMount?.power_on
          )
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
        value: this.componentData.selectionMount?.burst_time
      },
      {
        key: 'latency',
        label: this.i18n.get('protection_nor_latency_label'),
        value: this.dataMapService.getLabel(
          'LiveMount_Latency',
          this.componentData.selectionMount?.latency
        )
      }
    ]);
    if (
      !(
        this.componentData.selectionMount?.burst_iops ||
        this.componentData.selectionMount?.burst_bandwidth
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

  getLiveMountData() {
    forkJoin([this.getCopies(), this.getPolicies()]).subscribe(response => {
      const [copy, policy] = response;
      const parameters = JSON.parse(
        this.componentData.liveMountData.parameters
      );
      assign(this.componentData, {
        selectionResource: {
          resource_name: this.componentData.liveMountData.resource_name,
          resource_location: this.componentData.liveMountData.resource_path
        },
        selectionCopy: copy || {},
        selectionPolicy: policy || {},
        selectionMount: assign(
          {
            name: parameters.name,
            power_on: parameters.config?.power_on || true,
            target_location: this.componentData.liveMountData.target_location,
            targetCPU: parameters.config.cpu.use_original
              ? TargetCPU.OriginalConfig
              : TargetCPU.SpecifyConfig,
            targetMemory: parameters.config.memory.use_original
              ? TargetMemory.OriginalConfig
              : TargetMemory.SpecifyConfig,
            num_virtual_sockets: parameters.config?.cpu?.num_virtual_sockets,
            num_cores_per_virtual:
              parameters.config?.cpu?.num_cores_per_virtual,
            memory_size: parameters.config?.memory?.memory_size || '',
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
      this.getComponentData();
    });
  }
}
