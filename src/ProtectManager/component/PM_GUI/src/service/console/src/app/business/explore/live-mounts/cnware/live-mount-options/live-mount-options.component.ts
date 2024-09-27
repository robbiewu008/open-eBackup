import {
  Component,
  EventEmitter,
  Input,
  OnInit,
  Output,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import {
  AppService,
  BaseUtilService,
  CommonConsts,
  DataMap,
  I18NService,
  MountTargetLocation,
  ProtectedResourceApiService,
  ResourceType,
  TargetCPU,
  TargetMemory
} from 'app/shared';
import { TableConfig, TableData } from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  each,
  every,
  find,
  first,
  includes,
  isArray,
  isEmpty,
  isNumber,
  map,
  omit,
  pick,
  size,
  trim
} from 'lodash';
import { Subject } from 'rxjs';

@Component({
  selector: 'aui-live-mount-cnware-options',
  templateUrl: './live-mount-options.component.html',
  styleUrls: ['./live-mount-options.component.less']
})
export class LiveMountOptionsComponent implements OnInit {
  @Input() componentData;
  @Input() activeIndex;
  @Output() selectMountOptionChange = new EventEmitter<any>();

  formGroup: FormGroup;
  mountTargetLocation = MountTargetLocation;
  targetCPU = TargetCPU;
  targetMemory = TargetMemory;
  dataMap = DataMap;
  restoreToNewLocationOnly = false;
  originLocation: string;

  selectResource;
  selectCopy;
  networkTableConfig: TableConfig;
  networkTableData: TableData;
  targetPortGroupOptions = [];

  serverTreeData = [];
  proxyOptions = [];

  nameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('common_bongding_port_name_tips_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [128])
  };
  virtualSocketsErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 128])
  });
  coresPerErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidMinSize: this.i18n.get('common_valid_minsize_label', [1]),
    invalidDivisorNum: this.i18n.get('explore_ivalid_number_cores_label')
  });
  memorysErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', ['4MB', '6128GB']),
    invalidMemory: this.i18n.get('explore_ivalid_memory_label')
  });

  @ViewChild('portGroupTpl', { static: true }) portGroupTpl: TemplateRef<any>;
  @ViewChild('networkNameTpl', { static: true }) networkNameTpl: TemplateRef<
    any
  >;

  valid$ = new Subject<boolean>();

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private appService: AppService,
    private appUtilsService: AppUtilsService,
    private baseUtilService: BaseUtilService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit(): void {
    this.initForm();
    this.initTableConfig();
    this.getProxyOptions();
  }

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [`${ResourceType.CNWARE}Plugin`]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        resource = resource.filter(
          item =>
            item.environment?.linkStatus ===
            DataMap.resource_LinkStatus_Special.normal.value
        );
        const hostArray = [];
        each(resource, item => {
          const tmp = item.environment;
          if (
            tmp.extendInfo.scenario === DataMap.proxyHostType.external.value
          ) {
            hostArray.push({
              ...tmp,
              key: tmp.uuid,
              value: tmp.uuid,
              label: `${tmp.name}(${tmp.endpoint})`,
              isLeaf: true
            });
          }
        });
        this.proxyOptions = hostArray;
      }
    );
  }

  getResourceIcon(node) {
    switch (node.subType) {
      case ResourceType.CNWARE:
        return node.linkStatus ===
          DataMap.resource_LinkStatus_Special.normal.value
          ? 'aui-icon-vCenter'
          : 'aui-icon-vCenter-offine';
      case DataMap.Resource_Type.cNwareHostPool.value:
        return 'aui-icon-host-pool';
      case DataMap.Resource_Type.cNwareCluster.value:
        return 'aui-icon-cluster';
      case DataMap.Resource_Type.cNwareHost.value:
        return 'aui-icon-host';
      default:
        return 'aui-sla-vm';
    }
  }

  emitFormValid() {
    this.selectMountOptionChange.emit(this.checkDisableBtn());
    this.valid$.next(this.checkDisableBtn());
  }

  getEnvironment() {
    if (!isEmpty(this.serverTreeData)) {
      return;
    }
    const extParams = {
      conditions: JSON.stringify({
        subType: ResourceType.CNWARE,
        type: ResourceType.CNWARE
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        this.serverTreeData = map(resource, item => {
          return {
            ...item,
            label: item.name,
            disabled: true,
            contentToggleIcon: this.getResourceIcon(item),
            children: [],
            isLeaf: false,
            expanded: false
          };
        });
      }
    );
  }

  expandedChange(event) {
    if (!event.expanded || event.children?.length) {
      return;
    }
    event.children = [];
    this.getExpandedChangeData(event);
  }

  getExpandedChangeData(event) {
    const extParams = {
      conditions: JSON.stringify({
        parentUuid: event.uuid
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        each(resource, item => {
          event.children.push(
            assign(item, {
              label: item.name,
              disabled: !includes(
                [DataMap.Resource_Type.cNwareHost.value],
                item.subType
              ),
              contentToggleIcon: this.getResourceIcon(item),
              children: includes(
                [DataMap.Resource_Type.cNwareHost.value],
                item.subType
              )
                ? null
                : [],
              isLeaf: includes(
                [DataMap.Resource_Type.cNwareHost.value],
                item.subType
              ),
              expanded: false
            })
          );
        });
        this.serverTreeData = [...this.serverTreeData];
      }
    );
  }

  clearTargetPortGroup() {
    each(this.networkTableData?.data, item => {
      item.recoveryPortGroup = '';
    });
    this.targetPortGroupOptions = [];
    this.emitFormValid();
  }

  initForm() {
    this.formGroup = this.fb.group({
      target_location: new FormControl(MountTargetLocation.Original),
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(CommonConsts.REGEX.cnwareName, true),
          this.baseUtilService.VALID.maxLength(128)
        ]
      }),
      computerLocation: new FormControl([]),
      targetCPU: new FormControl(TargetCPU.OriginalConfig),
      targetMemory: new FormControl(TargetCPU.OriginalConfig),
      proxyHost: new FormControl([]),
      power_on: new FormControl(true),
      openInterface: new FormControl(false)
    });

    this.formGroup.get('target_location').valueChanges.subscribe(res => {
      this.clearTargetPortGroup();
      if (res === MountTargetLocation.Original) {
        this.formGroup.get('computerLocation').clearValidators();
        this.getPortGroupOptions({
          uuid: this.selectResource.parent_uuid,
          root_uuid: this.selectResource.root_uuid
        });
      } else {
        this.formGroup
          .get('computerLocation')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.getPortGroupOptions(first(this.formGroup.value.computerLocation));
      }
      this.formGroup.get('computerLocation').updateValueAndValidity();
    });

    this.formGroup.get('computerLocation').valueChanges.subscribe(res => {
      this.clearTargetPortGroup();
      this.getPortGroupOptions(first(res));
    });

    this.listenTargetCPU();
    this.listenTargetMemory();

    this.formGroup.statusChanges.subscribe(() => {
      this.emitFormValid();
    });
  }

  initTableConfig() {
    this.networkTableConfig = {
      table: {
        async: false,
        columns: [
          {
            key: 'name',
            name: this.i18n.get('protection_recovery_network_card_name_label'),
            cellRender: this.networkNameTpl
          },
          {
            key: 'port',
            name: this.i18n.get('protection_port_group_name_label'),
            cellRender: this.portGroupTpl
          }
        ],
        compareWith: 'uuid',
        colDisplayControl: false
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true,
        showTotal: true,
        pageSize: CommonConsts.PAGE_SIZE_SMALL
      }
    };
  }

  validNumberCores(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!trim(control.value)) {
        return null;
      }

      if (!this.formGroup.value.num_virtual_sockets) {
        return { invalidDivisorNum: { value: control.value } };
      }

      return +control.value !== 0 &&
        +this.formGroup.value.num_virtual_sockets % +control.value === 0
        ? null
        : { invalidDivisorNum: { value: control.value } };
    };
  }

  vavalidTargetMemory(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!trim(control.value)) {
        return null;
      }

      return +control.value % 4 === 0
        ? null
        : { invalidMemory: { value: control.value } };
    };
  }

  listenTargetCPU() {
    this.formGroup.get('targetCPU').valueChanges.subscribe(res => {
      if (res === TargetCPU.OriginalConfig) {
        this.formGroup.removeControl('num_virtual_sockets');
        this.formGroup.removeControl('num_cores_per_virtual');
      } else {
        this.formGroup.addControl(
          'num_virtual_sockets',
          new FormControl('', {
            validators: [
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.rangeValue(1, 128)
            ]
          })
        );
        this.formGroup.addControl(
          'num_cores_per_virtual',
          new FormControl('', {
            validators: [
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.minSize(0),
              this.baseUtilService.VALID.required(),
              this.validNumberCores()
            ]
          })
        );
        this.formGroup
          .get('num_virtual_sockets')
          .valueChanges.subscribe(res => {
            setTimeout(() => {
              if (!this.formGroup.get('num_cores_per_virtual').value) {
                return;
              }
              this.formGroup.get('num_cores_per_virtual').markAsTouched();
              this.formGroup
                .get('num_cores_per_virtual')
                .updateValueAndValidity();
            }, 0);
          });
      }
    });
  }

  listenTargetMemory() {
    this.formGroup.get('targetMemory').valueChanges.subscribe(res => {
      if (res === TargetMemory.OriginalConfig) {
        this.formGroup.removeControl('memory_size');
      } else {
        this.formGroup.addControl(
          'memory_size',
          new FormControl('', {
            validators: [
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.rangeValue(4, 6128 * 1024),
              this.vavalidTargetMemory()
            ]
          })
        );
      }
    });
  }

  getOriginalNetworkCard() {
    const properties = JSON.parse(this.selectCopy?.properties || '{}');
    const networks = properties.interfaceList || [];
    this.networkTableData = {
      data: networks,
      total: size(networks)
    };
  }

  getPortGroup(
    targetServer,
    agentsId,
    recordsTemp?: any[],
    startPage?: number
  ) {
    const params = {
      agentId: agentsId,
      envId: targetServer.rootUuid || targetServer.root_uuid,
      resourceIds: [targetServer.uuid || targetServer.root_uuid],
      pageNo: startPage || 1,
      pageSize: 200,
      akDoException: false,
      conditions: JSON.stringify({
        resourceType: 'PortGroup',
        uuid: targetServer.uuid
      })
    };

    this.appService.ListResourcesDetails(params).subscribe(res => {
      if (!recordsTemp) {
        recordsTemp = [];
      }
      if (!isNumber(startPage)) {
        startPage = 1;
      }
      recordsTemp = [...recordsTemp, ...res.records];
      if (
        startPage === Math.ceil(res.totalCount / 200) ||
        res.totalCount === 0
      ) {
        this.targetPortGroupOptions = map(recordsTemp, item => {
          const details = JSON.parse(item.extendInfo?.details || '{}');
          return assign(item, {
            label: item.parentName
              ? `${item.name} (${item.parentName})`
              : item.name,
            isLeaf: true
          });
        });
        this.emitFormValid();
        return;
      }
      startPage++;
      this.getPortGroup(targetServer, agentsId, recordsTemp, startPage);
    });
  }

  getPortGroupOptions(targetServer) {
    if (isEmpty(targetServer)) {
      return;
    }
    this.protectedResourceApiService
      .ListResources({
        pageNo: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        queryDependency: true,
        akDoException: false,
        conditions: JSON.stringify({
          uuid: targetServer.rootUuid || targetServer.root_uuid
        })
      })
      .subscribe((res: any) => {
        if (first(res.records)) {
          const onlineAgents = res.records[0]?.dependencies?.agents?.filter(
            item =>
              item.linkStatus ===
              DataMap.resource_LinkStatus_Special.normal.value
          );
          if (isEmpty(onlineAgents)) {
            return;
          }
          const agentsId = onlineAgents[0].uuid;
          this.getPortGroup(targetServer, agentsId);
        }
      });
  }

  portGroupChange() {
    this.emitFormValid();
  }

  checkDisableBtn() {
    return (
      this.formGroup.valid &&
      !isEmpty(this.networkTableData?.data) &&
      every(
        this.networkTableData?.data,
        item => !isEmpty(item.recoveryPortGroup)
      )
    );
  }

  initData() {
    this.selectCopy = this.componentData.selectionCopy;
    try {
      this.selectResource = JSON.parse(
        this.componentData?.selectionResource?.resource_properties
      );
    } catch (error) {
      this.selectResource = this.componentData?.selectionResource;
    }
    this.originLocation = this.selectResource?.path;
    this.getEnvironment();
    this.getOriginalNetworkCard();
    this.emitFormValid();
    this.restoreToNewLocationOnly =
      includes(
        [
          DataMap.CopyData_generatedType.replicate.value,
          DataMap.CopyData_generatedType.reverseReplication.value,
          DataMap.CopyData_generatedType.cascadedReplication.value
        ],
        this.selectCopy?.generated_by
      ) ||
      this.selectCopy?.is_replicated ||
      this.selectCopy?.resource_status ===
        DataMap.Resource_Status.notExist.value;
    if (this.restoreToNewLocationOnly) {
      setTimeout(() => {
        this.formGroup
          .get('target_location')
          .setValue(MountTargetLocation.Others);
      });
    } else {
      this.getPortGroupOptions({
        uuid: this.selectResource?.parent_uuid,
        root_uuid: this.selectResource?.root_uuid
      });
    }
  }

  getComponentData() {
    const requestParams = {
      target_resource_uuid_list: [
        this.formGroup.value.target_location === MountTargetLocation.Original
          ? this.selectResource.parent_uuid
          : this.formGroup.value.computerLocation[0]?.uuid
      ],
      target_location: this.formGroup.value.target_location,
      file_system_share_info_list: [
        {
          fileSystemName: `cnware_mount_${Date.now()}`,
          type: 1,
          accessPermission: 1,
          advanceParams: {
            clientType: 0,
            clientName: '*',
            squash: 1,
            rootSquash: 1,
            portSecure: 1
          }
        }
      ]
    };
    const parameters = {} as any;
    const performance = {};
    const performanceParams = pick(this.formGroup.value, [
      'min_bandwidth',
      'max_bandwidth',
      'burst_bandwidth',
      'min_iops',
      'max_iops',
      'burst_iops',
      'burst_time',
      'latency'
    ]);
    each(performanceParams, (v, k) => {
      if (isEmpty(trim(String(v)))) {
        return;
      }
      if (!this.formGroup.value.latencyStatus && k === 'latency') {
        return;
      }
      performance[k] = v;
    });
    assign(parameters, {
      performance,
      config: {
        power_on: this.formGroup.value.power_on
      },
      name: trim(this.formGroup.value.name),
      agents: isArray(this.formGroup.value.proxyHost)
        ? this.formGroup.value.proxyHost.join(';')
        : '',
      bridgeInterface: JSON.stringify({
        detail: map(this.networkTableData?.data, item => {
          const targetPortGroup = find(this.targetPortGroupOptions, {
            uuid: item.recoveryPortGroup
          });
          const details = JSON.parse(
            targetPortGroup.extendInfo?.details || '{}'
          );
          assign(details, { id: item.recoveryPortGroup });
          return {
            bridge: omit(item, [
              'parent',
              'portGroupOptions',
              'recoveryPortGroup'
            ]),
            portGroup: details
          };
        })
      })
    });

    assign(parameters, {
      openInterface: this.formGroup.value.openInterface ? 'true' : 'false'
    });

    const cpu = {} as any;
    if (this.formGroup.value.targetCPU === TargetCPU.SpecifyConfig) {
      if (this.formGroup.value.num_virtual_sockets) {
        assign(cpu, {
          num_virtual_sockets: this.formGroup.value.num_virtual_sockets
        });
      }
      if (this.formGroup.value.num_cores_per_virtual) {
        assign(cpu, {
          num_cores_per_virtual: this.formGroup.value.num_cores_per_virtual
        });
      }
      assign(cpu, {
        use_original: false
      });
    } else {
      assign(cpu, {
        use_original: true
      });
      delete cpu.num_virtual_sockets;
      delete cpu.num_cores_per_virtual;
    }
    assign(parameters.config, { cpu });

    const memory = {} as any;
    if (this.formGroup.value.targetMemory === TargetMemory.SpecifyConfig) {
      if (this.formGroup.value.memory_size) {
        assign(memory, {
          memory_size: this.formGroup.value.memory_size
        });
      }
      assign(memory, {
        use_original: false
      });
    } else {
      assign(memory, {
        use_original: true
      });
      delete memory.memory_size;
    }
    assign(parameters.config, { memory });

    assign(requestParams, { parameters });
    return assign(this.componentData, {
      requestParams: assign(
        {},
        this.componentData.requestParams,
        requestParams
      ),
      selectionMount: {
        ...this.formGroup.value
      }
    });
  }
}
