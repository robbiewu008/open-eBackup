import {
  ChangeDetectorRef,
  Component,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  AbstractControl,
  FormArray,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import { ModalRef, OptionItem } from '@iux/live';
import {
  AppService,
  BaseUtilService,
  CAPACITY_UNIT,
  CapacityCalculateLabel,
  ClientManagerApiService,
  CommonConsts,
  CopiesService,
  DatabasesService,
  DataMap,
  DataMapService,
  disableOracleRestoreNewLocation,
  HostService,
  I18NService,
  MODAL_COMMON,
  ProtectedResourceApiService,
  ResourceService,
  RestoreApiV2Service,
  RestoreType,
  RestoreV2LocationType,
  RestoreV2Type
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import { TableConfig, TableData } from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  countBy,
  defer,
  each,
  filter,
  find,
  first,
  get,
  includes,
  isArray,
  isEmpty,
  isNumber,
  isString,
  map,
  orderBy,
  padEnd,
  reject,
  set,
  size,
  toLower,
  toString,
  trim,
  uniq,
  uniqBy
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-oracle-restore',
  templateUrl: './oracle-restore.component.html',
  styleUrls: ['./oracle-restore.component.less'],
  providers: [CapacityCalculateLabel]
})
export class OracleRestoreComponent implements OnInit {
  formGroup: FormGroup;
  targetHosts = [];
  targetHostOption = [];
  rowCopyIsAsm;
  originalInstanceOptions: OptionItem[] = [];
  rowCopy;
  oldVersion;
  targetVersion;
  targetHostIPs;
  restoreToNewLocationOnly;
  disableNewLocation = false; // oracle有些场景不支持新位置恢复
  _restoreType = RestoreType;
  _restoreV2Type = RestoreV2Type;
  restoreType;
  filterParams = [];
  restoreLocationType = RestoreV2LocationType;
  dbFilterParams = {};
  dataMap = DataMap;
  optsConfig: ProButton[]; // 自动匹配&&重置
  restoreDiskConfig: TableConfig;
  restoreDiskData: TableData;
  totalDisk = 0;
  selectedDiskNumber = 0;
  proxyHostOptions = []; // 存储快照副本高级配置需要选择代理主机
  targetDiskOptions = [];
  originalDiskData; // 从副本中取出的磁盘信息
  cacheSelectedDisk = []; // 磁盘缓存，用于实现磁盘互斥选择
  singleNodeOpts: OptionItem[]; // 存放集群下面的节点数据
  selectedHostIsCluster = false; // 选中的是目标主机/集群
  isSnapshotCopy = false;
  showAlert = false;
  hostBuiltinLabel = this.i18n.get('protection_hcs_host_builtin_label');
  hostExternalLabel = this.i18n.get('protection_hcs_host_external_label');
  scriptErrorTip = {
    invalidName: this.i18n.get('common_script_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [8192])
  };
  numberOfChannelRangeErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 254])
  };
  destinationPathErrorTip = {
    invalidPath: this.baseUtilService.invalidPathLabel,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [256])
  };
  targetHostErrorTip = assign(
    {},
    {
      invalidSameDb: this.i18n.get(
        'protection_target_host_restore_oneline_db_label'
      ),
      ...this.baseUtilService.requiredErrorTip
    }
  );
  instanceInputErrorTip = assign(
    {},
    {
      invalidName: this.i18n.get('protection_target_instance_input_error_label')
    }
  );
  concurrencyErrorTip = {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 8]),
    invalidInteger: this.i18n.get('common_valid_integer_label')
  };
  originalError;
  originHostLabel;
  resourceEnvironment;
  restoreTargetLabel;
  instanceInputLabel = this.i18n.get(
    'protection_target_instance_input_tip_label'
  );
  isCluster = false;

  @ViewChild('tipTpl', { static: false }) tipTpl: TemplateRef<any>;
  @ViewChild('selectDiskTpl', { static: true }) selectDiskTpl: TemplateRef<any>;
  constructor(
    public fb: FormBuilder,
    private modal: ModalRef,
    public i18n: I18NService,
    private hostApiService: HostService,
    private dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private drawModalService: DrawModalService,
    private databasesService: DatabasesService,
    private resourcesService: ResourceService,
    private restoreV2Service: RestoreApiV2Service,
    private protectedResourceApiService: ProtectedResourceApiService,
    public capacityCalculateLabel: CapacityCalculateLabel,
    private appUtilsService: AppUtilsService,
    private clientManagerApiService: ClientManagerApiService,
    private cdr: ChangeDetectorRef,
    private copiesService: CopiesService,
    private appService: AppService
  ) {}

  ngOnInit() {
    this.formatRowCopy();
    this.initForm();
    this.updateData();
    this.getHostLabel();
    this.getHosts();
  }

  // 通过agent接口获取实时的磁盘信息
  queryDiskInfo(item, node?) {
    // 选中集群时 agentId为集群节点的hostId
    const param = {
      agentId: node ? node.hostId : item.rootUuid,
      envId: item.rootUuid,
      pageNo: CommonConsts.PAGE_START_EXTRA,
      pageSize: CommonConsts.PAGE_SIZE,
      appType: DataMap.Resource_Type.oracle.value,
      resourceIds: [item.uuid],
      conditions: JSON.stringify({
        queryType: 'diskInfo',
        hostId: node ? node.hostId : item.rootUuid
      })
    };
    this.appService.ListResourcesDetails(param).subscribe(
      res => {
        this.formatDiskOptions(
          get(first(res.records), 'extendInfo.disk_infos', '[]')
        );
      },
      error => {
        this.setValid();
      }
    );
  }

  getHostLabel() {
    return !isEmpty(this.rowCopy.resource_environment_ip)
      ? `${this.rowCopy.resource_environment_name}(${this.rowCopy.resource_environment_ip})`
      : this.rowCopy.resource_environment_name;
  }

  get dbConfig() {
    return (this.formGroup.get('dbConfig') as FormArray).controls;
  }

  get instanceConfig() {
    return this.formGroup.get('instanceConfig') as FormArray;
  }

  updateData() {
    let properties;
    if (this.rowCopy.scn) {
      // scn恢复需要特殊取值
      properties =
        JSON.parse(this.rowCopy.rowCoyByScn.resource_properties || '{}') || {};
    } else {
      properties = JSON.parse(this.rowCopy.resource_properties || '{}') || {};
    }
    if (this.rowCopy.resource_sub_type === DataMap.Resource_Type.oracle.value) {
      this.originalInstanceOptions = this.formatSelectionOptions(
        [properties.extendInfo],
        'inst_name'
      );
      this.rowCopyIsAsm = get(properties, 'extendInfo.is_asm_inst');
    } else {
      const instancesArr = JSON.parse(
        get(properties, 'extendInfo.instances', '[]')
      );
      this.originalInstanceOptions = this.formatSelectionOptions(
        instancesArr,
        'inst_name'
      );
      this.rowCopyIsAsm = get(instancesArr[0], 'is_asm_inst');
    }
    this.oldVersion = properties.version;
    if (this.restoreToNewLocationOnly) {
      setTimeout(() => {
        this.formGroup.get('restoreTo').setValue(RestoreV2LocationType.NEW);
      }, 100);
    }
    this.isCluster =
      this.rowCopy.resource_sub_type ===
      DataMap.Resource_Type.oracleCluster.value;
  }

  asyncValidSameNameDB() {
    return (
      control: AbstractControl
    ): Promise<{ [key: string]: any } | null> => {
      return new Promise(resolve => {
        if (this.restoreType === RestoreType.FileRestore) {
          resolve(null);
          return;
        }
        this.checkLinkStatus(control, resolve);
      });
    };
  }

  checkLinkStatus(ctrl: AbstractControl, resolve) {
    this.protectedResourceApiService
      .ListResources({
        pageNo: 0,
        pageSize: CommonConsts.PAGE_SIZE * 5,
        akLoading: false,
        conditions: JSON.stringify({
          subType: [
            DataMap.Resource_Type.oracle.value,
            DataMap.Resource_Type.oracleCluster.value
          ],
          rootUuid: includes(ctrl.value, this.rowCopy?.name)
            ? this.rowCopy.environment_uuid
            : ctrl.value
        })
      })
      .subscribe(
        res => {
          const normalDB = find(res.records, item => {
            return (
              item.extendInfo?.linkStatus ===
                DataMap.resource_LinkStatus_Special.normal.value &&
              toLower(this.rowCopy.name) === toLower(item.name)
            );
          });
          if (normalDB) {
            const errorKey = 'protection_target_host_restore_oneline_db_label';
            this.targetHostErrorTip = assign({}, this.targetHostErrorTip, {
              invalidSameDb: this.i18n.get(errorKey, [normalDB?.name])
            });
            this.originalError = this.i18n.get(errorKey, [normalDB?.name]);
            resolve({ invalidSameDb: { value: ctrl.value } });
          } else {
            this.originalError = '';
          }
          resolve(null);
        },
        () => resolve(null)
      );
  }

  initForm() {
    this.formGroup = this.fb.group({
      restoreTo: new FormControl(RestoreV2LocationType.ORIGIN),
      name: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()],
        asyncValidators: [this.asyncValidSameNameDB()]
      }),
      targetHost: new FormControl(''),
      destinationPath: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.maxLength(256),
          this.baseUtilService.VALID.path(this.rowCopy.environment_os_type)
        ]
      }),
      bctStatus: new FormControl(false),
      numberOfChannelOpen: new FormControl(false),
      numberOfChannels: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 254)
        ]
      }),
      proxyHost: new FormControl([]),
      scriptOpen: new FormControl(false),
      preProcessing: new FormControl(''),
      postProcessing: new FormControl(''),
      restore_failed_script: new FormControl(''),
      singleNode: new FormControl(''),
      dbConfig: this.fb.array([]),
      instanceConfig: this.fb.array([]), // 非存储快照副本需要选择原实例与目标实例
      power_on: new FormControl(true),
      concurrency: new FormControl(3, {
        validators: [
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 8)
        ]
      })
    });
    if (this.isSnapshotCopy) {
      // 只有存储快照副本需要选择代理主机和选择磁盘
      this.initDiskConfig();
      this.getProxyHosts();
    }
    defer(() => {
      this.formGroup
        .get('name')
        .setValue(
          `${this.rowCopy?.resource_environment_name || ''}/${
            this.rowCopy?.name
          }`
        );
      this.formGroup.get('name').disable();
    });
    this.formGroup.get('restoreTo').valueChanges.subscribe(val => {
      defer(() => {
        if (this.restoreLocationType.NEW === val) {
          this.formGroup
            .get('targetHost')
            .setValidators([this.baseUtilService.VALID.required()]);
          this.formGroup
            .get('targetHost')
            .setAsyncValidators([this.asyncValidSameNameDB()]);
          this.formGroup
            .get('destinationPath')
            .setValidators([
              this.baseUtilService.VALID.maxLength(256),
              this.baseUtilService.VALID.path(this.rowCopy.environment_os_type)
            ]);
          this.formGroup.get('name').clearValidators();
          this.formGroup.get('name').clearAsyncValidators();
        } else {
          this.formGroup
            .get('name')
            .setAsyncValidators([this.asyncValidSameNameDB()]);
          this.formGroup
            .get('name')
            .setValidators([this.baseUtilService.VALID.required()]);
          this.formGroup.get('targetHost').clearValidators();
          this.formGroup.get('targetHost').clearAsyncValidators();
          this.formGroup.get('destinationPath').clearValidators();
        }
        this.formGroup.get('targetHost').setValue('');
        this.instanceConfig.clear();
        this.formGroup.get('name').updateValueAndValidity();
        this.formGroup.get('targetHost').updateValueAndValidity();
        this.formGroup.get('destinationPath').updateValueAndValidity();
      });
    });
    this.formGroup.get('numberOfChannelOpen').valueChanges.subscribe(val => {
      if (val) {
        this.formGroup
          .get('numberOfChannels')
          .setValidators([
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 254),
            this.baseUtilService.VALID.required()
          ]);
      } else {
        this.formGroup.get('numberOfChannels').clearValidators();
      }
      this.formGroup.get('numberOfChannels').updateValueAndValidity();
    });

    this.formGroup.get('scriptOpen').valueChanges.subscribe(res => {
      if (res) {
        this.formGroup
          .get('preProcessing')
          .setValidators([
            this.rowCopy.environment_os_type === DataMap.Os_Type.windows.value
              ? this.baseUtilService.VALID.name(
                  CommonConsts.REGEX.windowsScript,
                  false
                )
              : this.baseUtilService.VALID.name(
                  CommonConsts.REGEX.linuxScript,
                  false
                ),
            this.baseUtilService.VALID.maxLength(8192)
          ]);
        this.formGroup
          .get('postProcessing')
          .setValidators([
            this.rowCopy.environment_os_type === DataMap.Os_Type.windows.value
              ? this.baseUtilService.VALID.name(
                  CommonConsts.REGEX.windowsScript,
                  false
                )
              : this.baseUtilService.VALID.name(
                  CommonConsts.REGEX.linuxScript,
                  false
                ),
            this.baseUtilService.VALID.maxLength(8192)
          ]);
        this.formGroup
          .get('restore_failed_script')
          .setValidators([
            this.rowCopy.environment_os_type === DataMap.Os_Type.windows.value
              ? this.baseUtilService.VALID.name(
                  CommonConsts.REGEX.windowsScript,
                  false
                )
              : this.baseUtilService.VALID.name(
                  CommonConsts.REGEX.linuxScript,
                  false
                ),
            this.baseUtilService.VALID.maxLength(8192)
          ]);
      } else {
        this.formGroup.get('preProcessing').clearValidators();
        this.formGroup.get('postProcessing').clearValidators();
        this.formGroup.get('restore_failed_script').clearValidators();
      }
      this.formGroup.get('preProcessing').updateValueAndValidity();
      this.formGroup.get('postProcessing').updateValueAndValidity();
      this.formGroup.get('restore_failed_script').updateValueAndValidity();
    });

    this.formGroup.get('targetHost').valueChanges.subscribe(root_uuid => {
      if (!root_uuid) {
        return;
      }
      this.getDependenciesById(root_uuid);
      if (this.isSnapshotCopy) {
        const targetHost = find(this.targetHostOption, { value: root_uuid });
        this.selectedHostIsCluster =
          targetHost.subType === DataMap.Resource_Type.oracleCluster.value;
        // 单机和集群都需要通过接口去获取实时的磁盘信息disk_infos
        if (this.selectedHostIsCluster) {
          this.formGroup
            .get('singleNode')
            .setValidators(this.baseUtilService.VALID.required());
          const instances = JSON.parse(
            get(targetHost, 'extendInfo.instances', '[]')
          );
          this.singleNodeOpts = map(instances, item => ({
            value: item.hostId,
            label: item.inst_name,
            isLeaf: true,
            ...item
          }));
          each(this.restoreDiskData.data, item => {
            delete item.originDiskOptions;
            delete item.target_disk;
            delete item.diskOptions;
            delete item.disk_uuid;
          });
        } else {
          this.formGroup.get('singleNode').clearValidators();
          this.queryDiskInfo(targetHost);
        }
        this.formGroup.get('singleNode').updateValueAndValidity();
        this.setValid();
      }
      const params = {
        pageNo: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        conditions: JSON.stringify({
          root_uuid,
          type: 'Application',
          sub_type: DataMap.Resource_Type.OracleApp.value
        })
      };
      this.resourcesService
        .queryResourcesV1ResourceGet(params)
        .subscribe(res => {
          this.targetHostIPs = undefined;
          if (!size(res.items)) {
            return;
          }

          const item = first(res.items);
          this.targetVersion = item.version;
          const targetHost = this.targetHosts.find(
            host => host.host_id === item.root_uuid
          );
          if (targetHost.is_cluster) {
            const targetHostIPs = [];
            each(targetHost.cluster_info, hostId => {
              const targetCluster = this.targetHosts.find(
                host => host.host_id === hostId
              );
              if (!targetCluster || !targetCluster.endpoint) {
                return;
              }
              targetHostIPs.push(targetCluster.endpoint);
            });
            this.targetHostIPs = !size(targetHostIPs)
              ? undefined
              : toString(targetHostIPs);
          }
        });
    });

    this.formGroup.get('singleNode').valueChanges.subscribe(hostId => {
      if (!hostId) {
        return;
      }
      const selectedHost = find(this.targetHostOption, {
        value: this.formGroup.value.targetHost
      });
      const selectedNode = find(this.singleNodeOpts, { hostId });
      this.queryDiskInfo(selectedHost, selectedNode);
    });
  }

  anoymizeScriptChange(e) {
    this.formGroup.get('anonymizationScript').setValue(e);
  }

  formatRowCopy() {
    if (this.rowCopy.resource_properties) {
      let resourceProperties;
      let properties;
      if (this.rowCopy.scn) {
        resourceProperties = JSON.parse(
          this.rowCopy.rowCoyByScn.resource_properties
        );
        properties = JSON.parse(this.rowCopy.rowCoyByScn.properties);
      } else {
        resourceProperties = JSON.parse(this.rowCopy.resource_properties);
        properties = JSON.parse(this.rowCopy.properties);
      }
      this.disableNewLocation = disableOracleRestoreNewLocation(
        this.rowCopy,
        properties,
        resourceProperties
      );
      this.rowCopy.environment_os_type =
        resourceProperties.environment_os_type ||
        resourceProperties.environment?.osType;
      this.rowCopy.environment_uuid =
        resourceProperties.environment_uuid ||
        resourceProperties.environment?.uuid ||
        resourceProperties.rootUuid;
      this.originHostLabel = `${get(
        resourceProperties,
        'environment_name'
      )}(${get(resourceProperties, 'environment_endpoint')})`;
      this.isSnapshotCopy =
        !!this.rowCopy?.storage_snapshot_flag ||
        get(
          resourceProperties,
          'ext_parameters.storage_snapshot_flag',
          false
        ) ||
        get(
          resourceProperties,
          'protectedObject.extParameters.storage_snapshot_flag',
          false
        );
      if (this.isSnapshotCopy) {
        // 日志副本的磁盘信息取离它最近的数据副本中的
        if (
          this.rowCopy?.backup_type === DataMap.CopyData_Backup_Type.log.value
        ) {
          this.getNearestDataCopy();
        } else {
          this.initOriginalDiskData(properties);
        }
      }
    }
    if (this.rowCopy.resource_name) {
      this.rowCopy.name = this.rowCopy.resource_name;
    }
    this.rowCopy.path = this.rowCopy.resource_location || this.rowCopy.path;
    this.restoreToNewLocationOnly =
      includes(
        [
          DataMap.CopyData_generatedType.replicate.value,
          DataMap.CopyData_generatedType.cascadedReplication.value
        ],
        this.rowCopy.generated_by
      ) ||
      this.rowCopy.is_replicated ||
      this.rowCopy?.resource_status === DataMap.Resource_Status.notExist.value;
    if (this.restoreType === RestoreType.FileRestore) {
      // 表级恢复，不支持恢复到新位置，gui暂时屏蔽新位置。
      this.restoreToNewLocationOnly = false;
    }
  }

  initDiskConfig() {
    this.optsConfig = [
      {
        id: 'reset',
        label: this.i18n.get('common_reset_label'),
        onClick: () => {
          this.resetMatch();
        }
      }
    ];
    this.restoreDiskConfig = {
      table: {
        async: false,
        colDisplayControl: false,
        columns: [
          {
            key: 'disk_path',
            name: this.i18n.get('common_restore_disk_name_label'),
            width: 180
          },
          {
            key: 'asm_path',
            name: this.i18n.get('protection_asm_disk_group_name_label'),
            width: 180
          },
          {
            key: 'disk_size_label',
            name: this.i18n.get('common_capacity_label'),
            width: 100,
            thAlign: 'left',
            sort: {
              customSort: () => {
                return this.restoreDiskData.data.reverse();
              }
            }
          },
          {
            key: 'target_path',
            name: this.i18n.get('common_target_disk_label'),
            cellRender: this.selectDiskTpl
          }
        ]
      },
      pagination: {
        winTablePagination: true,
        showPageSizeOptions: false,
        mode: 'simple'
      }
    };
    each(this.originalDiskData, item => {
      assign(item, {
        disk_size_label: this.capacityCalculateLabel.transform(
          get(item, 'disk_size'),
          '1.3-3',
          CAPACITY_UNIT.KB
        )
      });
    });
    this.totalDisk = size(this.originalDiskData);
    this.restoreDiskData = {
      data: this.originalDiskData,
      total: size(this.originalDiskData)
    };
  }

  initOriginalDiskData(data) {
    const diskInfos = get(data, 'disk_infos', '[]');
    this.originalDiskData = isEmpty(diskInfos)
      ? []
      : orderBy(
          isString(diskInfos) ? JSON.parse(diskInfos) : diskInfos,
          'disk_size',
          ['desc']
        );
  }

  getHosts(recordsTemp?, startPage?) {
    this.protectedResourceApiService
      .ListResources({
        pageSize: CommonConsts.PAGE_SIZE,
        pageNo: startPage || 0,
        queryDependency: true,
        conditions: JSON.stringify({
          subType:
            this.rowCopy.resource_sub_type ===
            DataMap.Resource_Type.oracle.value
              ? [DataMap.Resource_Type.oracle.value]
              : [
                  DataMap.Resource_Type.oracle.value,
                  DataMap.Resource_Type.oracleCluster.value
                ]
        })
      })
      .subscribe(res => {
        if (!recordsTemp) {
          recordsTemp = [];
        }
        if (!isNumber(startPage)) {
          startPage = CommonConsts.PAGE_START;
        }
        startPage++;
        recordsTemp = [...recordsTemp, ...res.records];
        if (
          startPage === Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE) ||
          res.totalCount === 0
        ) {
          recordsTemp = uniqBy(recordsTemp, 'rootUuid');
          // 过滤离线主机
          recordsTemp = filter(recordsTemp, item => {
            return (
              item.environment?.linkStatus ===
              DataMap.resource_LinkStatus_Special.normal.value
            );
          });
          // 过滤操作系统，不能跨操作系统恢复
          recordsTemp = filter(
            recordsTemp,
            item =>
              item.environment?.osType === this.rowCopy.environment_os_type
          );
          this.targetHosts = recordsTemp;
          each(recordsTemp, item => {
            this.targetHostOption = this.targetHostOption.concat(
              assign(item, {
                isLeaf: true,
                value: item.rootUuid || item.parentUuid,
                label: `${item.environment?.name}(${item.environment?.endpoint})`,
                disabled: this.disableHost(item)
              })
            );
          });
          this.modal.getInstance().lvOkDisabled = this.formGroup.invalid;
          return;
        }
        this.getHosts(recordsTemp, startPage);
      });
  }

  disableHost(data) {
    // 存储快照副本不能选择非存储快照注册的环境--storages存储资源为空的环境
    if (this.isSnapshotCopy) {
      const storages = get(data, 'extendInfo.storages', []);
      let is_asm_inst;
      if (data.subType === DataMap.Resource_Type.oracle.value) {
        is_asm_inst = get(data, 'extendInfo.is_asm_inst');
      } else {
        const instances = get(data, 'extendInfo.instances', '[]');
        is_asm_inst = get(JSON.parse(instances)[0], 'is_asm_inst');
      }
      return isEmpty(storages) || is_asm_inst !== this.rowCopyIsAsm;
    } else {
      return false;
    }
  }

  getProxyHosts() {
    const extParams = {
      pageSize: CommonConsts.PAGE_SIZE_MAX,
      conditions: JSON.stringify({
        pluginType: `${DataMap.Resource_Type.oracle.value}Plugin`,
        linkStatus: [String(DataMap.resource_LinkStatus.normal.value)]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      param => this.clientManagerApiService.queryAgentListInfoUsingGET(param),
      resource => {
        const filterResource = filter(
          resource,
          item => item.osType === DataMap.Os_Type.linux.value
        );
        this.proxyHostOptions = map(filterResource, item => ({
          ...item,
          key: item.uuid,
          value: item.uuid,
          label: `${item.name}(${item.endpoint})`,
          isLeaf: true
        }));
      }
    );
  }

  getNearestDataCopy() {
    // 时间轴恢复会携带restoreTimeStamp,需要进行转化
    let timeStamp: number;
    const rowCopyTimeStamp =
      this.rowCopy?.restoreTimeStamp || this.rowCopy.timestamp;
    timeStamp = Number(padEnd(rowCopyTimeStamp, 16, '0'));
    const params = {
      pageSize: CommonConsts.PAGE_SIZE,
      pageNo: CommonConsts.PAGE_START,
      orders: ['-display_timestamp'],
      conditions: JSON.stringify({
        resource_id: this.rowCopy.resource_id,
        backup_type: [
          DataMap.CopyData_Backup_Type.full.value,
          DataMap.CopyData_Backup_Type.incremental.value,
          DataMap.CopyData_Backup_Type.diff.value
        ]
      })
    };
    this.copiesService.queryResourcesV1CopiesGet(params).subscribe(res => {
      const filterRes = find(res.items, item => {
        return Number(item.timestamp) <= timeStamp;
      });
      if (!isEmpty(filterRes)) {
        this.initOriginalDiskData(JSON.parse(filterRes.properties));
        this.initDiskConfig();
      }
    });
  }

  buildRestoreCmd(params, observer: Observer<void>) {
    this.restoreV2Service
      .CreateRestoreTask({ CreateRestoreTaskRequestBody: params })
      .subscribe(
        res => {
          observer.next();
          observer.complete();
        },
        err => {
          observer.error(err);
          observer.complete();
        }
      );
  }

  // 集群和单机的disk来自不同的地方
  formatDiskOptions(diskInfoString) {
    this.resetMatch();
    const diskInfos = orderBy(JSON.parse(diskInfoString), 'disk_size', [
      'desc'
    ]);
    this.showAlert = size(diskInfos) < this.totalDisk;
    // 初始化下拉选项
    each(diskInfos, item => {
      assign(item, {
        label: item.disk_path,
        disk_size_label: this.capacityCalculateLabel.transform(
          item.disk_size,
          '1.1-3',
          CAPACITY_UNIT.KB
        ),
        value: item.disk_path,
        isLeaf: true,
        disabled: false,
        target_size: item.disk_size
      });
    });
    // 每一行下拉选项都会进行比较大小后置灰
    each(this.restoreDiskData.data, item => {
      const diskOptions = map(diskInfos, disk => ({
        ...disk,
        disabled: item.disk_size !== disk.disk_size
      }));
      // 将处理后的下拉数据保存一份副本
      assign(item, {
        diskOptions,
        originDiskOptions: diskOptions
      });
    });
  }

  instanceSelectionChange(event) {
    const cacheSelectedInstace = reject(
      map(
        this.instanceConfig.controls,
        control => control.value.selectInstance
      ),
      item => isEmpty(item)
    );
    each(this.originalInstanceOptions, item => {
      item.disabled = includes(cacheSelectedInstace, item.value);
    });
    this.originalInstanceOptions = [...this.originalInstanceOptions];
  }

  diskSelectionChange(event, target) {
    // 每选中一个 其他下拉框排除这一项
    this.cacheSelectedDisk = reject(
      map(this.restoreDiskData.data, 'target_disk'),
      item => isEmpty(item)
    );
    this.selectedDiskNumber = size(this.cacheSelectedDisk);
    each(this.restoreDiskData.data, item => {
      if (item.disk_path === target.disk_path) {
        // 跳过当前选中的行
        return;
      }
      item.diskOptions = filter(
        item.originDiskOptions,
        disk =>
          !includes(this.cacheSelectedDisk, disk.label) ||
          disk.label === item.target_disk
      );
    });
    this.setValid();
  }

  setValid() {
    this.modal.getInstance().lvOkDisabled =
      this.formGroup.invalid ||
      (this.formGroup.value.restoreTo === RestoreV2LocationType.NEW &&
        this.selectedDiskNumber !== this.totalDisk);
  }

  resetMatch() {
    each(this.restoreDiskData.data, item => {
      if (item.originDiskOptions) {
        item.diskOptions = [...item.originDiskOptions];
      }
      delete item.target_disk;
      delete item.target_disk_uuid;
    });
    this.selectedDiskNumber = 0;
    this.setValid();
  }

  getDependenciesById(root_uuid) {
    this.instanceConfig.clear();
    const selectedHost = find(this.targetHostOption, { value: root_uuid });
    if (this.isSnapshotCopy) {
      return;
    }
    each(this.originalInstanceOptions, item =>
      assign(item, {
        disabled: false
      })
    );
    this.originalInstanceOptions = [...this.originalInstanceOptions];
    // 集群需要查询其中的环境信息
    if (selectedHost.subType === DataMap.Resource_Type.oracle.value) {
      this.instanceConfig.push(
        this.fb.group({
          rootUuid: selectedHost.rootUuid,
          endpoint: selectedHost.path,
          selectInstance: new FormControl(
            '',
            this.baseUtilService.VALID.required()
          ),
          targetInstance: new FormControl('', {
            validators: [
              this.baseUtilService.VALID.name(
                RegExp("^[^|;&$><`'@!+\\n]*$"),
                false
              ),
              this.validRepeatInstance()
            ]
          })
        })
      );
      return;
    }
    this.protectedResourceApiService
      .ShowResource({
        resourceId: root_uuid
      })
      .subscribe(
        res => {
          const dependencies = get(res, 'dependencies.agents', []);
          each(dependencies, item => {
            this.instanceConfig.push(
              this.fb.group({
                rootUuid: item.rootUuid,
                endpoint: item.path,
                selectInstance: new FormControl(
                  '',
                  this.baseUtilService.VALID.required()
                ),
                targetInstance: new FormControl('', {
                  validators: [
                    this.baseUtilService.VALID.name(
                      RegExp("^[^|;&$><`'@!+\\n]*$"),
                      false
                    ),
                    this.validRepeatInstance()
                  ]
                })
              })
            );
          });
        },
        err => {
          this.instanceConfig.clear();
        }
      );
  }

  validRepeatInstance(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) {
        return null;
      }
      // 从array中取出每个targetInstance
      const filterArr = map(
        this.instanceConfig.controls,
        ctrl => ctrl.get('targetInstance').value
      ).filter(target => !!target);
      // 根据出现的次数进行分组
      const countObj = countBy(filterArr);
      // 取出出现次数大于1的key作为新数组
      const repeatInstances = filter(
        uniq(filterArr),
        item => countObj[item] > 1
      );
      if (includes(repeatInstances, control.value)) {
        assign(this.instanceInputErrorTip, {
          repeatInstance: this.i18n.get('protection_same_instance_tips_label', [
            control.value
          ])
        });
        return { repeatInstance: control.value };
      }
      return null;
    };
  }

  formatSelectionOptions(data, targetKey) {
    const options: OptionItem[] = [];
    if (isArray(data)) {
      each(data, item => {
        options.push({
          value: item[targetKey],
          label: item[targetKey],
          isLeaf: true
        });
      });
    }
    return options;
  }

  getTargetParams() {
    return {
      ...this.formGroup.value,
      name: `${this.rowCopy?.resource_environment_name || ''}/${
        this.rowCopy?.name
      }`,
      restore_location: this.formGroup.value.restoreTo,
      restoreLocation: this.formGroup.value.restoreTo,
      requestParams: this.getParams()
    };
  }

  getParams() {
    const restoreTargetHost = {};
    const dbConfigControls = (this.formGroup.get('dbConfig') as FormArray)
      .controls;
    dbConfigControls.forEach(control => {
      restoreTargetHost[control.value.key] = control.value.newParam
        ? control.value.newParam
        : control.value.originParam;
    });
    const params = {
      copyId: this.rowCopy.uuid,
      agents: [],
      targetEnv:
        this.formGroup.value.restoreTo === this.restoreLocationType.NEW
          ? this.formGroup.value.targetHost
          : this.rowCopy.environment_uuid,
      targetLocation: this.formGroup.value.restoreTo,
      restoreType: this.restoreType,
      targetObject:
        this.formGroup.value.restoreTo === this.restoreLocationType.ORIGIN
          ? this.rowCopy.resource_id
          : find(this.targetHostOption, {
              value: this.formGroup.value.targetHost
            })?.uuid,
      extendInfo: {
        restoreFrom:
          this.rowCopy?.backup_type === DataMap.CopyData_Backup_Type.log.value
            ? 'log'
            : 'data',
        bctStatus: this.formGroup.value.bctStatus,
        CHANNELS: this.formGroup.value.numberOfChannels || '',
        UUID: this.rowCopy.dbUuid || this.rowCopy.resource_id || '',
        RESTORE_TARGET_HOST: isEmpty(restoreTargetHost)
          ? ''
          : JSON.stringify(restoreTargetHost),
        RESTORE_PATH:
          this.formGroup.value.restoreTo === this.restoreLocationType.NEW
            ? this.formGroup.value.destinationPath
            : '',
        instances:
          this.formGroup.value.restoreTo === this.restoreLocationType.NEW &&
          !this.isSnapshotCopy
            ? JSON.stringify(
                map(this.instanceConfig.controls, ({ value }) => {
                  return {
                    agent_id: value['rootUuid'],
                    agent_ip: value['endpoint'],
                    instance_name: trim(value['targetInstance']),
                    src_instance_name: value['selectInstance']
                  };
                })
              )
            : '[]',
        // 存储快照副本 目标主机为集群时选择节点
        hostUuid:
          this.isSnapshotCopy && this.selectedHostIsCluster
            ? this.formGroup.value.singleNode
            : ''
      },
      scripts: {
        preScript: this.formGroup.value.scriptOpen
          ? this.formGroup.value.preProcessing || ''
          : '',
        postScript: this.formGroup.value.scriptOpen
          ? this.formGroup.value.postProcessing || ''
          : '',
        failPostScript: this.formGroup.value.scriptOpen
          ? this.formGroup.value.restore_failed_script || ''
          : ''
      }
    };
    if (this.restoreType === RestoreV2Type.CommonRestore) {
      set(
        params,
        'extendInfo.isStartDB',
        this.formGroup.value.power_on ? 1 : 0
      );
    }
    // 存储快照备份副本
    if (this.isSnapshotCopy) {
      if (this.formGroup.value.restoreTo === RestoreV2LocationType.NEW) {
        const diskInfos = map(this.restoreDiskData.data, item => {
          const { disk_path, disk_size, target_disk, wwn } = item;
          return {
            wwn,
            mount_point: get(item, 'mount_point', ''),
            disk_path,
            disk_size,
            target_size: find(item.originDiskOptions, { value: target_disk })
              .target_size,
            target_path: target_disk,
            target_disk_uuid: find(item.originDiskOptions, {
              value: target_disk
            }).disk_uuid,
            disk_uuid: get(item, 'disk_uuid', '')
          };
        });
        assign(params.extendInfo, {
          diskInfos: JSON.stringify(diskInfos)
        });
      }
      const proxyHost = map(this.formGroup.value.proxyHost, item => {
        const targetHost = find(this.proxyHostOptions, { uuid: item });
        return {
          uuid: targetHost.uuid,
          ip: targetHost.endpoint
        };
      });
      assign(params.extendInfo, {
        proxyHost: JSON.stringify(proxyHost),
        concurrent_requests: String(this.formGroup.value.concurrency)
      });
      delete params.extendInfo.CHANNELS;
      delete params.extendInfo.bctStatus;
      delete params.extendInfo.RESTORE_PATH;
    }
    // 时间轴恢复
    if (this.rowCopy.restoreTimeStamp) {
      assign(params.extendInfo, {
        restoreFrom: 'pit',
        restoreTimestamp: this.rowCopy.restoreTimeStamp,
        restoreCopyId: this.rowCopy.uuid || ''
      });
    }
    // scn恢复
    if (this.rowCopy.scn) {
      assign(params.extendInfo, {
        restoreFrom: 'scn',
        restoreScn: this.rowCopy.scn,
        restoreCopyId: this.rowCopy.uuid || ''
      });
    }
    this.restoreTargetLabel =
      this.formGroup.value.restoreTo === this.restoreLocationType.NEW
        ? `${
            find(this.targetHostOption, {
              value: this.formGroup.value.targetHost
            })?.label
          }`
        : `${this.rowCopy?.resource_environment_name || ''}/${
            this.rowCopy?.name
          }`;
    return params;
  }

  restore(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = this.getParams();
      this.drawModalService.create({
        ...MODAL_COMMON.generateDrawerOptions(),
        lvModalKey: 'copy-info-message',
        ...{
          lvType: 'dialog',
          lvDialogIcon: 'lv-icon-popup-danger-48',
          lvHeader: this.i18n.get('common_restore_tips_label'),
          lvContent: this.tipTpl,
          lvWidth: 500,
          lvOkType: 'primary',
          lvCancelType: 'default',
          lvOkDisabled: false,
          lvFocusButtonId: 'cancel',
          lvCloseButtonDisplay: true,
          lvOk: () => this.buildRestoreCmd(params, observer),
          lvCancel: () => {
            observer.error(null);
            observer.complete();
          },
          lvAfterClose: result => {
            if (result && result.trigger === 'close') {
              observer.error(null);
              observer.complete();
            }
          }
        }
      });
    });
  }
}
