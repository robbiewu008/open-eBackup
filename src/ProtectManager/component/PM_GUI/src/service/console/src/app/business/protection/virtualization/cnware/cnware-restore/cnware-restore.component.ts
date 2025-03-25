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
import {
  Component,
  OnDestroy,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { MessageService, OptionItem } from '@iux/live';
import {
  AppService,
  BaseUtilService,
  CAPACITY_UNIT,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  ProtectedResourceApiService,
  ResourceDetailType,
  ResourceType,
  RestoreApiV2Service,
  RestoreV2LocationType,
  RestoreV2Type,
  StorageLocation,
  isJson,
  getBootTypeWarnTipByType
} from 'app/shared';
import {
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  cloneDeep,
  defer,
  differenceBy,
  each,
  every,
  filter,
  find,
  first,
  get,
  includes,
  isArray,
  isEmpty,
  isNumber,
  map,
  omit,
  pick,
  reject,
  set,
  size,
  trim,
  values
} from 'lodash';
import { Observable, Observer, Subject, switchMap, takeUntil } from 'rxjs';

@Component({
  selector: 'aui-cnware-restore',
  templateUrl: './cnware-restore.component.html',
  styleUrls: ['./cnware-restore.component.less']
})
export class CnwareRestoreComponent implements OnInit, OnDestroy {
  rowCopy;
  restoreType;
  _isEmpty = isEmpty;
  isDiskRestore = false;
  formGroup: FormGroup;
  restoreLocationType = RestoreV2LocationType;
  restoreV2Type = RestoreV2Type;
  storageLocation = StorageLocation;
  unitconst = CAPACITY_UNIT;
  dataMap = DataMap;
  restoreToNewLocationOnly = false;
  recoveryDiskTableConfig: TableConfig;
  recoveryDiskTableData: TableData;
  originDiskTableConfig: TableConfig;
  originDiskTableData: TableData;
  networkTableConfig: TableConfig;
  networkTableData: TableData;
  irDiskTableConfig: TableConfig;
  irDiffDiskTableConfig: TableConfig;
  CopyDataVerifyStatus = DataMap.HCSCopyDataVerifyStatus;
  verifyStatus;
  copyVerifyDisableLabel;
  targetDisksOptions = [];
  cacheSelectedDisk = [];

  targetDatastoreOptions = [];
  cacheSelectedDatastore = [];
  originalDatastoreOptions = [];

  targetPortGroupOptions = [];
  cachePortGroupOptions = [];
  nutaniClusterOptions = [];
  nutaniHostOptions = [];
  isNutanix = false;

  resourceProperties;
  properties; // 副本properties
  rowCopyBootType; // 副本中记录的bootType，有可能为空
  originLocation;
  serverTreeData = [];
  proxyOptions = [];
  EXIST_DISK = '1';
  NEW_DISK = '2';
  NFS_STORAGE_TYPE = 3; // nfs存储只能选择精简置备
  bootOptionsWarnTip: string; // 引导选项不一致提示
  typeOptions: OptionItem[] = [
    {
      label: this.i18n.get('protection_recovery_exist_disk_label'),
      value: this.EXIST_DISK,
      isLeaf: true
    },
    {
      label: this.i18n.get('protection_recovery_new_disk_label'),
      value: this.NEW_DISK,
      isLeaf: true
    }
  ];
  preallocationOptions = this.dataMapService
    .toArray('preallocationType')
    .filter(item => {
      return (item.isLeaf = true);
    });
  offPreallocationOptions = this.dataMapService
    .toArray('preallocationType')
    .filter(item => {
      item.isLeaf = true;
      return includes([DataMap.preallocationType.off.value], item.value);
    });
  nameMaxLength = 128;
  nameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('common_bongding_port_name_tips_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [128])
  };
  networkHelpLabel = this.i18n.get(
    'protection_cnware_restore_network_help_label',
    [ResourceType.CNWARE]
  );

  // 搜索之前保持原来的数据
  originalTreeData = [];
  // 搜索关键字
  searchVmKey: string;
  currentSearchKey: string;
  searchTrigger$ = new Subject<string>();
  destroy$ = new Subject();

  valid$ = new Subject<boolean>();

  @ViewChild('datastoreTpl', { static: true }) datastoreTpl: TemplateRef<any>;
  @ViewChild('targetDiskTpl', { static: true }) targetDiskTpl: TemplateRef<any>;
  @ViewChild('sizeTpl', { static: true }) sizeTpl: TemplateRef<any>;
  @ViewChild('recoveryTypeTpl', { static: true }) recoveryTypeTpl: TemplateRef<
    any
  >;
  @ViewChild('portGroupTpl', { static: true }) portGroupTpl: TemplateRef<any>;
  @ViewChild('targetIpTpl', { static: true }) targetIpTpl: TemplateRef<any>;
  @ViewChild('targetIpHeaderTpl', { static: true })
  targetIpHeaderTpl: TemplateRef<any>;
  @ViewChild('networkNameTpl', { static: true }) networkNameTpl: TemplateRef<
    any
  >;
  @ViewChild('preallocationTpl', { static: true })
  preallocationTpl: TemplateRef<any>;
  @ViewChild('thExtHelp', { static: true }) thExtHelp: TemplateRef<any>;
  @ViewChild('originalPreallocation', { static: true })
  originalPreallocation: TemplateRef<any>;

  constructor(
    private fb: FormBuilder,
    public i18n: I18NService,
    private appService: AppService,
    private dataMapService: DataMapService,
    private messageService: MessageService,
    private appUtilsService: AppUtilsService,
    private baseUtilService: BaseUtilService,
    private restoreV2Service: RestoreApiV2Service,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnDestroy(): void {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngOnInit(): void {
    this.isNutanix =
      this.rowCopy?.resource_type === DataMap.Resource_Type.nutanix.value;
    if (this.isNutanix) {
      this.nameMaxLength = 80;
      this.nameErrorTip.invalidMaxLength = this.i18n.get(
        'common_valid_maxlength_label',
        [80]
      );
      this.networkHelpLabel = this.i18n.get(
        'protection_cnware_restore_network_help_label',
        [ResourceType.NUTANIX]
      );
    }
    this.initConfig();
    this.initForm();
    defer(() => this.getOriginalVmDisk());
    this.initCopyVerifyDisableLabel();
    this.getProxyOptions();
    this.getOriginalTargetDisk();
    this.getOriginalNetworkCard();
    this.getOriginalPortGroupOptions();
    this.setSearchState();
  }

  // 由于搜索是输入框change触发，防止触发多次搜索。
  setSearchState() {
    this.searchTrigger$
      .pipe(
        switchMap(searchKey => {
          this.searchVmKey = searchKey;
          return this.protectedResourceApiService.ListResources({
            akLoading: false,
            pageNo: CommonConsts.PAGE_START,
            pageSize: CommonConsts.PAGE_SIZE_MAX,
            conditions: JSON.stringify({
              subType: this.isDiskRestore
                ? [
                    DataMap.Resource_Type.cNwareHostPool.value,
                    DataMap.Resource_Type.cNwareCluster.value,
                    DataMap.Resource_Type.cNwareHost.value,
                    DataMap.Resource_Type.cNwareVm.value
                  ]
                : [
                    DataMap.Resource_Type.cNwareHostPool.value,
                    DataMap.Resource_Type.cNwareCluster.value,
                    DataMap.Resource_Type.cNwareHost.value
                  ],
              name: [['~~'], searchKey]
            })
          });
        }),
        takeUntil(this.destroy$)
      )
      .subscribe(res => {
        // 响应结果回来时，搜索值已清空，则不更新树
        if (!this.currentSearchKey) {
          return;
        }
        each(res.records, item => {
          assign(item, {
            label: item.name,
            contentToggleIcon: this.getResourceIcon(item),
            children:
              (this.isDiskRestore &&
                item.subType === DataMap.Resource_Type.cNwareVm.value) ||
              item.subType === DataMap.Resource_Type.cNwareHost.value
                ? null
                : [],
            disabled: this.isDiskRestore
              ? item.subType !== DataMap.Resource_Type.cNwareVm.value
              : item.subType !== DataMap.Resource_Type.cNwareHost.value,
            isLeaf: this.isDiskRestore
              ? item.subType === DataMap.Resource_Type.cNwareVm.value
              : item.subType === DataMap.Resource_Type.cNwareHost.value,
            expanded: false
          });
        });
        this.serverTreeData = res.records;
      });
  }

  initCopyVerifyDisableLabel() {
    if (
      includes([this.CopyDataVerifyStatus.noGenerate.value], this.verifyStatus)
    ) {
      this.copyVerifyDisableLabel = this.i18n.get(
        'common_generate_verify_file_disable_label'
      );
    }
    if (
      includes([this.CopyDataVerifyStatus.Invalid.value], this.verifyStatus)
    ) {
      this.copyVerifyDisableLabel = this.i18n.get(
        'common_invalid_verify_file_disable_label'
      );
    }
  }

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [`${this?.rowCopy?.resource_type}Plugin`]
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

  initConfig() {
    this.isDiskRestore = this.rowCopy.diskRestore === true;
    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('protection_fc_disk_name_label')
      },
      {
        key: 'type',
        name: this.i18n.get('common_bus_type_label'),
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('cnwareDiskType')
        }
      },
      {
        key: 'volSizeInBytes',
        name: this.i18n.get('protection_fc_disk_capacity_label'),
        cellRender: this.sizeTpl
      }
    ];
    this.originDiskTableConfig = {
      table: {
        async: false,
        columns: [...cols],
        compareWith: 'uuid',
        colDisplayControl: false,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        selectionChange: selection => {
          const canceledDisk = differenceBy(
            this.recoveryDiskTableData?.data,
            selection,
            'uuid'
          );
          this.cacheSelectedDisk = reject(this.cacheSelectedDisk, item =>
            includes(map(canceledDisk, 'recoveryDisk'), item)
          );

          this.recoveryDiskTableData = {
            data: selection,
            total: size(selection)
          };

          if (!isEmpty(this.targetDisksOptions)) {
            each(this.recoveryDiskTableData.data, item => {
              assign(item, {
                diskOptions: filter(
                  this.targetDisksOptions,
                  value =>
                    value.uuid === item.recoveryDisk ||
                    this.fiterDisk(value, item)
                )
              });
            });
          }
          if (this.isDiskRestore) {
            this.setDiskDatastoreOptions();
          }
          this.disableOkBtn();
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true,
        showTotal: true,
        pageSize: CommonConsts.PAGE_SIZE_SMALL
      }
    };
    this.recoveryDiskTableConfig = {
      table: {
        async: false,
        columns: [
          ...cols,
          {
            key: 'recoveryType',
            width: 180,
            name: this.i18n.get('common_type_label'),
            hidden: !this.isDiskRestore,
            cellRender: this.recoveryTypeTpl
          },
          {
            key: 'recoveryDisk',
            width: this.isDiskRestore ? 460 : 360,
            name: this.isDiskRestore
              ? this.i18n.get('common_target_disk_label')
              : this.i18n.get('protection_tagert_database_label'),
            thExtra: this.thExtHelp,
            cellRender: this.isDiskRestore
              ? this.targetDiskTpl
              : this.datastoreTpl
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
            name: this.isNutanix
              ? this.i18n.get('protection_nutanix_target name_label')
              : this.i18n.get('protection_port_group_name_label'),
            cellRender: this.portGroupTpl
          },
          {
            key: 'targetIp',
            name: this.i18n.get('protection_nutanix_target_ip_label'),
            thExtra: this.targetIpHeaderTpl,
            cellRender: this.targetIpTpl,
            hidden: !this.isNutanix
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
    if (this.restoreType === RestoreV2Type.InstanceRestore) {
      this.irDiskTableConfig = {
        table: {
          async: false,
          columns: [
            ...cols,
            {
              key: 'preallocation',
              name: this.i18n.get('protection_original_preallocation_label'),
              cellRender: this.originalPreallocation
            },
            {
              key: 'recoveryPreallocation',
              name: this.i18n.get('protection_target_preallocation_label'),
              cellRender: this.preallocationTpl
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
      this.irDiffDiskTableConfig = {
        table: {
          async: false,
          columns: [
            ...cols,
            {
              key: 'recoveryPool',
              width: 360,
              name: this.i18n.get('protection_target_storage_pool_label'),
              cellRender: this.datastoreTpl
            },
            {
              key: 'preallocation',
              name: this.i18n.get('protection_original_preallocation_label'),
              cellRender: this.originalPreallocation
            },
            {
              key: 'recoveryPreallocation',
              name: this.i18n.get('protection_target_preallocation_label'),
              cellRender: this.preallocationTpl
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
  }

  clearTargetDisk() {
    each(this.recoveryDiskTableData?.data, item => {
      item.recoveryDisk = '';
      item.recoveryDatastore = '';
      assign(item, {
        diskOptions: []
      });
    });
    this.targetDatastoreOptions = [];
    this.disableOkBtn();
  }

  clearTargetPortGroup() {
    each(this.networkTableData?.data, item => {
      item.recoveryPortGroup = '';
      item.targetIp = '';
      assign(item, {
        portGroupOptions: []
      });
    });
    this.targetPortGroupOptions = [];
    this.disableOkBtn();
  }

  listenForm() {
    this.formGroup.get('restoreTo').valueChanges.subscribe(res => {
      if (this.isDiskRestore) {
        this.clearTargetDisk();
      } else {
        if (!this.isNutanix) {
          this.clearTargetPortGroup();
        }
      }
      if (res === RestoreV2LocationType.ORIGIN) {
        this.formGroup.get('targetServer').clearValidators();
        this.formGroup.get('targetCluster').clearValidators();
        this.formGroup.get('targetRecoveryPool').clearValidators();
        if (this.isDiskRestore) {
          this.getOriginalTargetDisk();
        } else {
          if (!this.isNutanix) {
            this.getOriginalPortGroupOptions();
          }
        }
      } else {
        if (this.isNutanix) {
          this.formGroup
            .get('targetCluster')
            .setValidators([this.baseUtilService.VALID.required()]);
          this.getNutanixClusterOps();
        } else {
          this.getEnvironment();
          this.formGroup
            .get('targetServer')
            .setValidators([this.baseUtilService.VALID.required()]);
        }
        if (this.isDiskRestore) {
          defer(() =>
            this.getNewTargetDisk(first(this.formGroup.value.targetServer))
          );
        } else {
          if (!this.isNutanix) {
            defer(() =>
              this.getPortGroupOptions(first(this.formGroup.value.targetServer))
            );
          }
        }
        if (
          this.restoreType === RestoreV2Type.InstanceRestore &&
          this.formGroup.value.targetPool === StorageLocation.Same
        ) {
          this.formGroup
            .get('targetRecoveryPool')
            .setValidators([this.baseUtilService.VALID.required()]);
        } else {
          this.formGroup.get('targetRecoveryPool').clearValidators();
        }
      }
      this.formGroup.get('targetServer').updateValueAndValidity();
      this.formGroup.get('targetCluster').updateValueAndValidity();
      this.formGroup.get('targetRecoveryPool').updateValueAndValidity();
    });

    this.formGroup.get('targetServer').valueChanges.subscribe(res => {
      if (isEmpty(res) || this.isNutanix) {
        return;
      }
      defer(() => {
        if (this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN) {
          return;
        }
        const selectedRes = first(res);
        if (this.isDiskRestore) {
          this.getNewTargetDisk(selectedRes);
          if (isEmpty(this.rowCopyBootType)) {
            // rowCopyBootType为空时直接提示启动方式无法获取
            this.getBootOptionsTip(selectedRes);
          } else {
            this.getResourceDetail(selectedRes);
          }
        } else {
          this.getDataStore(selectedRes);
          this.getPortGroupOptions(selectedRes);
        }
      });
    });

    this.formGroup.get('targetPool').valueChanges.subscribe(res => {
      if (res === StorageLocation.Same) {
        this.formGroup
          .get('targetRecoveryPool')
          .setValidators([this.baseUtilService.VALID.required()]);
      } else {
        this.formGroup.get('targetRecoveryPool').clearValidators();
      }
      this.formGroup.get('targetRecoveryPool').updateValueAndValidity();
      this.resetDiskTablePreallocation();
    });

    this.formGroup.get('targetCluster').valueChanges.subscribe(res => {
      if (!res) return;
      this.formGroup.get('targetServer').setValue([]);
      this.getNutanixHostOps(res);
    });

    // 切换存储池，置备类型重新选择
    this.formGroup
      .get('targetRecoveryPool')
      .valueChanges.subscribe(() => this.resetDiskTablePreallocation());
  }

  resetDiskTablePreallocation() {
    each(this.recoveryDiskTableData?.data, item => {
      assign(item, {
        recoveryPreallocation: ''
      });
    });
  }

  initForm() {
    this.resourceProperties = JSON.parse(
      this.rowCopy?.resource_properties || '{}'
    );
    this.originLocation = this.resourceProperties.path;
    this.restoreToNewLocationOnly =
      includes(
        [
          DataMap.CopyData_generatedType.replicate.value,
          DataMap.CopyData_generatedType.reverseReplication.value,
          DataMap.CopyData_generatedType.cascadedReplication.value
        ],
        this.rowCopy?.generated_by
      ) ||
      this.rowCopy.is_replicated ||
      this.rowCopy?.resource_status === DataMap.Resource_Status.notExist.value;
    const properties = JSON.parse(this.rowCopy.properties);
    this.properties = properties;
    this.rowCopyBootType = get(properties, 'bootType', null);
    this.verifyStatus = properties?.verifyStatus;

    this.formGroup = this.fb.group({
      restoreTo: new FormControl(RestoreV2LocationType.ORIGIN),
      vmName: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(CommonConsts.REGEX.cnwareName, true),
          this.baseUtilService.VALID.maxLength(this.nameMaxLength)
        ]
      }),
      targetCluster: new FormControl(''),
      targetServer: new FormControl([]),
      targetPool: new FormControl(StorageLocation.Same),
      targetRecoveryPool: new FormControl(''),
      proxyHost: new FormControl([]),
      restoreAutoPowerOn: new FormControl(true),
      copyVerify: new FormControl(false),
      cleanOriginVM: new FormControl(false),
      openInterface: new FormControl(false)
    });

    if (this.isDiskRestore) {
      this.formGroup.get('vmName').clearValidators();
    }

    this.listenForm();

    this.formGroup.valueChanges.subscribe(() =>
      defer(() => this.disableOkBtn())
    );

    if (this.restoreToNewLocationOnly) {
      defer(() =>
        this.formGroup.get('restoreTo').setValue(RestoreV2LocationType.NEW)
      );
    }
  }

  getOriginalNetworkCard() {
    if (this.isDiskRestore) {
      return;
    }
    const properties = JSON.parse(this.rowCopy?.properties);
    const networks = properties?.interfaceList;
    this.networkTableData = {
      data: networks || [],
      total: size(networks) || 0
    };
  }

  getOriginalPortGroupOptions() {
    if (
      (this.restoreToNewLocationOnly && !this.isNutanix) ||
      this.isDiskRestore
    ) {
      return;
    }
    this.getPortGroupOptions({
      uuid: this.resourceProperties.parent_uuid,
      root_uuid: this.resourceProperties.root_uuid
    });
  }

  getPortGroupOptions(targetServer) {
    if (this.isDiskRestore) {
      return;
    }
    if (isEmpty(targetServer)) {
      return;
    }
    this.appUtilsService
      .getResourcesDetails(
        targetServer,
        this.isNutanix
          ? ResourceDetailType.nutanixNetwork
          : ResourceDetailType.portGroup
      )
      .subscribe(recordsTemp => {
        this.targetPortGroupOptions = map(recordsTemp, item => {
          return assign(item, {
            label: item.parentName
              ? `${item.name} (${item.parentName})`
              : item.name,
            isLeaf: true
          });
        });
        this.cachePortGroupOptions = [];
        this.setPortGroupOptions();
        this.disableOkBtn();
      });
  }

  getDefaultPortGroup(item): string {
    // cnware原位置恢复，默认选中原端口
    if (
      this.rowCopy?.resource_type === DataMap.Resource_Type.cNware.value &&
      this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN &&
      isJson(item.metaData)
    ) {
      const metaData = JSON.parse(item.metaData);
      if (find(this.targetPortGroupOptions, { uuid: metaData?.portGroupId })) {
        return metaData?.portGroupId;
      }
    }
    return '';
  }

  setPortGroupOptions() {
    if (!isEmpty(this.networkTableData?.data)) {
      each(this.networkTableData?.data, item => {
        item.recoveryPortGroup = this.getDefaultPortGroup(item);
        item.targetIp = '';
        assign(item, {
          portGroupOptions: cloneDeep(this.targetPortGroupOptions)
        });
      });
    }
  }

  getOriginalVmDisk() {
    const properties = JSON.parse(this.rowCopy.properties);
    let needRestoreDisks = properties?.volList;
    if (isEmpty(needRestoreDisks)) {
      needRestoreDisks = properties.extendInfo?.volList || [];
    }
    each(needRestoreDisks, item => {
      if (this.isDiskRestore) {
        assign(item, {
          diskOptions: [],
          recoveryDiskType: this.EXIST_DISK
        });
      } else {
        assign(item, {
          datastoreOptions: []
        });
      }
    });
    if (this.isDiskRestore) {
      this.originDiskTableData = {
        data: needRestoreDisks,
        total: size(needRestoreDisks)
      };
    } else {
      this.recoveryDiskTableData = {
        data: needRestoreDisks,
        total: size(needRestoreDisks)
      };
    }
  }

  getOriginalTargetDisk(isOriginal = true) {
    if (this.restoreToNewLocationOnly && !this.isNutanix) {
      return;
    }
    if (this.isDiskRestore) {
      this.getTargetDisk(this.resourceProperties);
    }
    this.getDataStore(
      {
        uuid: this.resourceProperties.parent_uuid,
        root_uuid: this.resourceProperties.root_uuid
      },
      isOriginal
    );
  }

  getNewTargetDisk(targetServer) {
    if (!targetServer) {
      return;
    }
    this.getTargetDisk(targetServer);
    this.getDataStore({
      uuid: targetServer.parentUuid,
      root_uuid: targetServer.rootUuid
    });
  }

  getEnvironment(recordsTemp?: any[], startPage?: number) {
    if (!isEmpty(this.serverTreeData)) {
      return;
    }
    this.protectedResourceApiService
      .ListResources({
        queryDependency: true,
        pageNo: startPage || CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE_MAX,
        conditions: JSON.stringify({
          subType: this?.rowCopy?.resource_type,
          type: this?.rowCopy?.resource_type
        })
      })
      .subscribe((res: any) => {
        const agentId = get(
          res.records[0]?.dependencies?.agents[0],
          'uuid',
          null
        );
        if (!recordsTemp) {
          recordsTemp = [];
        }
        if (!isNumber(startPage)) {
          startPage = CommonConsts.PAGE_START;
        }
        startPage++;
        recordsTemp = [...recordsTemp, ...res.records];
        if (
          startPage ===
            Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE_MAX) ||
          res.totalCount === CommonConsts.PAGE_START
        ) {
          this.serverTreeData = map(recordsTemp, item => {
            return {
              ...item,
              agentId,
              label: item.name,
              disabled: true,
              contentToggleIcon: this.getResourceIcon(item),
              children: [],
              isLeaf: false,
              expanded: false
            };
          });
          // 保存数据，用于搜索复原
          this.originalTreeData = cloneDeep(this.serverTreeData);
          return;
        }
        this.getEnvironment(recordsTemp, startPage);
      });
  }
  getNutanixClusterOps() {
    const extParams = {
      conditions: JSON.stringify({
        subType: DataMap.Resource_Type.nutanixCluster.value
      })
    };

    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        const arr = [];
        each(resource, item => {
          arr.push({
            ...item,
            label: item.name,
            value: item.uuid,
            key: item.uuid,
            isLeaf: true
          });
        });
        this.nutaniClusterOptions = arr;
      }
    );
  }

  getNutanixHostOps(cluster) {
    const extParams = {
      conditions: JSON.stringify({
        parentUuid: cluster,
        subType: DataMap.Resource_Type.nutanixHost.value
      })
    };

    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        const arr = [];
        each(resource, item => {
          arr.push({
            ...item,
            label: item.name,
            value: item.uuid,
            key: item.uuid,
            isLeaf: true
          });
        });
        this.nutaniHostOptions = arr;
      }
    );
  }

  fiterDisk(value, item) {
    // 数据盘不能恢复到系统盘
    if (item.bootable !== '1') {
      return (
        !includes(this.cacheSelectedDisk, value.uuid) &&
        value.bootOrder !== '1' &&
        +value.size >= +item.volSizeInBytes
      );
    }
    return (
      !includes(this.cacheSelectedDisk, value.uuid) &&
      +value.size >= +item.volSizeInBytes
    );
  }

  batchRecoveryDiskTable(recordsTemp) {
    each(recordsTemp, item => {
      assign(item, JSON.parse(item.extendInfo?.details));
    });
    this.targetDisksOptions = map(recordsTemp, item => {
      return assign(item, {
        label: item.name,
        isLeaf: true
      });
    });
    this.cacheSelectedDisk = [];
    if (!isEmpty(this.recoveryDiskTableData?.data)) {
      each(this.recoveryDiskTableData?.data, item => {
        item.recoveryDisk = '';
        assign(item, {
          diskOptions: filter(cloneDeep(this.targetDisksOptions), val => {
            return this.fiterDisk(val, item);
          })
        });
      });
    }
  }

  getTargetDisk(targetServer) {
    if (isEmpty(targetServer)) {
      return;
    }
    this.appUtilsService
      .getResourcesDetails(
        targetServer,
        this.isNutanix
          ? DataMap.Resource_Type.nutanixDisk.value
          : DataMap.Resource_Type.cNwareDisk.value
      )
      .subscribe(recordsTemp => {
        this.batchRecoveryDiskTable(recordsTemp);
        this.disableOkBtn();
      });
  }

  getDataStore(targetHost, isOriginal = false) {
    if (isEmpty(targetHost)) {
      return;
    }
    this.appUtilsService
      .getResourcesDetails(
        targetHost,
        this.isNutanix
          ? ResourceDetailType.nutanixStorageContainer
          : ResourceDetailType.storagePool
      )
      .subscribe(recordsTemp => {
        const datastores = map(recordsTemp, item => {
          const details = JSON.parse(item.extendInfo?.details || '{}');
          return assign(item, {
            capacity: details?.available || 0,
            showCapacity: details?.available || 0,
            label: item.name,
            value: item.uuid,
            isLeaf: true
          });
        });
        if (isOriginal && !this.isDiskRestore && !this.isNutanix) {
          this.originalDatastoreOptions = datastores;
          return;
        }
        if (this.isNutanix) {
          this.originalDatastoreOptions = datastores;
        }
        this.targetDatastoreOptions = datastores;
        this.cacheSelectedDatastore = [];
        if (!isEmpty(this.recoveryDiskTableData?.data)) {
          each(this.recoveryDiskTableData?.data, item => {
            item.recoveryDatastore = '';
          });
          this.setDiskDatastoreOptions();
        }
        this.disableOkBtn();
      });
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
          assign(item, {
            label: item.name,
            disabled: this.isDiskRestore
              ? item.subType !== DataMap.Resource_Type.cNwareVm.value
              : item.subType !== DataMap.Resource_Type.cNwareHost.value,
            contentToggleIcon: this.getResourceIcon(item),
            children:
              (this.isDiskRestore &&
                item.subType === DataMap.Resource_Type.cNwareVm.value) ||
              item.subType === DataMap.Resource_Type.cNwareHost.value
                ? null
                : [],
            isLeaf: this.isDiskRestore
              ? item.subType === DataMap.Resource_Type.cNwareVm.value
              : item.subType === DataMap.Resource_Type.cNwareHost.value,
            expanded: false
          });
          if (event?.agentId) {
            // 往下传递客户端ID
            set(item, 'agentId', event.agentId);
          }
          event.children.push(item);
        });
        this.serverTreeData = [...this.serverTreeData];
        // 保存数据，用于搜索复原
        if (!this.currentSearchKey) {
          this.originalTreeData = cloneDeep(this.serverTreeData);
        }
      }
    );
  }

  disableOkBtn() {
    if (this.isDiskRestore) {
      this.valid$.next(
        this.formGroup.valid &&
          !isEmpty(this.recoveryDiskTableData?.data) &&
          every(this.recoveryDiskTableData?.data, item => {
            return item.recoveryDiskType === this.NEW_DISK
              ? !isEmpty(item.targetDiskName) &&
                  !item.nameInvalid &&
                  !isEmpty(item.recoveryDatastore)
              : !isEmpty(item.recoveryDisk);
          })
      );
    } else {
      const validPortGroup =
        !isEmpty(this.networkTableData?.data) &&
        every(
          this.networkTableData?.data,
          item => !isEmpty(item.recoveryPortGroup)
        );
      if (this.restoreType === RestoreV2Type.InstanceRestore) {
        this.valid$.next(
          this.formGroup.value.restoreTo === RestoreV2LocationType.NEW
            ? this.formGroup.valid &&
                !isEmpty(this.recoveryDiskTableData?.data) &&
                every(this.recoveryDiskTableData?.data, item =>
                  this.formGroup.value.targetPool === StorageLocation.Same
                    ? !isEmpty(item.recoveryPreallocation)
                    : !isEmpty(item.recoveryPreallocation) &&
                      !isEmpty(item.recoveryDatastore)
                ) &&
                validPortGroup
            : this.formGroup.valid && validPortGroup
        );
      } else {
        this.valid$.next(
          this.formGroup.value.restoreTo === RestoreV2LocationType.NEW
            ? this.formGroup.valid &&
                !isEmpty(this.recoveryDiskTableData?.data) &&
                every(
                  this.recoveryDiskTableData?.data,
                  item => !isEmpty(item.recoveryDatastore)
                ) &&
                validPortGroup
            : this.formGroup.valid && validPortGroup
        );
      }
    }
  }

  // 设置磁盘数据存储选项
  setDiskDatastoreOptions() {
    each(this.recoveryDiskTableData?.data, item => {
      const datastoreOptions = cloneDeep(this.targetDatastoreOptions);
      each(datastoreOptions, datastore => {
        if (
          !item.recoveryDatastore &&
          item.volSizeInBytes > datastore.showCapacity
        ) {
          assign(datastore, {
            disabled: true,
            disabledTips: this.i18n.get(
              'protection_remain_capacity_insufficient_label'
            )
          });
        }
      });
      item.datastoreOptions = datastoreOptions;
    });
  }

  // 剩余数据存储容量计算
  datastoreChange(item?) {
    if (this.isNutanix) {
      this.disableOkBtn();
      return;
    }
    this.cacheSelectedDatastore = reject(
      map(this.recoveryDiskTableData?.data, 'recoveryDatastore'),
      item => isEmpty(item)
    );
    each(this.targetDatastoreOptions, item => {
      if (includes(this.cacheSelectedDatastore, item.uuid)) {
        let usedCapacity = 0;
        each(this.recoveryDiskTableData?.data, v => {
          if (v.recoveryDatastore === item.uuid) {
            usedCapacity += v.volSizeInBytes;
          }
        });
        assign(item, {
          showCapacity: item.capacity - usedCapacity
        });
      } else {
        assign(item, {
          showCapacity: item.capacity
        });
      }
    });
    // 即时恢复存储池处理，切换存储池清空置备类型，需要重新选择
    if (item && this.restoreType === this.restoreV2Type.InstanceRestore) {
      assign(item, {
        recoveryPreallocation: ''
      });
    }
    this.setDiskDatastoreOptions();
    this.disableOkBtn();
  }

  diskTypeChange(_, disk) {
    this.disableOkBtn();
  }

  validDiskName(name, disk) {
    if (!name) {
      assign(disk, {
        nameInvalid: true,
        nameErrorTip: this.i18n.get('common_required_label')
      });
      return;
    }
    const diskNameReg = /^[a-zA-Z0-9_]{1}[a-zA-Z_0-9-.]*$/;
    const numberOnlyReg = /^[0-9]*$/;
    if (!diskNameReg.test(name) || numberOnlyReg.test(name)) {
      assign(disk, {
        nameInvalid: true,
        nameErrorTip: this.i18n.get('protection_cnware_new_disk_name_label')
      });
      return;
    }
    if (name.length > 80) {
      assign(disk, {
        nameInvalid: true,
        nameErrorTip: this.i18n.get('common_valid_maxlength_label', [80])
      });
      return;
    }
    delete disk.nameInvalid;
    delete disk.nameErrorTip;
  }

  diskNameChange(name, disk) {
    this.validDiskName(name, disk);
    this.disableOkBtn();
  }

  diskChange(_, disk) {
    this.cacheSelectedDisk = reject(
      map(this.recoveryDiskTableData?.data, 'recoveryDisk'),
      item => isEmpty(item)
    );
    each(this.recoveryDiskTableData?.data, item => {
      if (item.uuid === disk.uuid) {
        return;
      }
      item.diskOptions = filter(
        this.targetDisksOptions,
        value => value.uuid === item.recoveryDisk || this.fiterDisk(value, item)
      );
    });
    this.disableOkBtn();
  }

  portGroupChange() {
    this.disableOkBtn();
  }

  preallocationChange() {
    this.disableOkBtn();
  }

  // 检查原位置恢复datastore容量
  checkOriginalDatastore(): boolean {
    const datastoreMap = {};
    each(this.recoveryDiskTableData?.data, item => {
      const datastore = find(
        this.originalDatastoreOptions,
        v => v.uuid === item.datastore?.poolId
      );
      if (datastore) {
        if (!datastoreMap[item.datastore?.poolId]) {
          datastoreMap[item.datastore?.poolId] = {
            total: datastore.capacity,
            used: item.volSizeInBytes
          };
        } else {
          datastoreMap[item.datastore?.poolId]['used'] =
            datastoreMap[item.datastore?.poolId]['used'] + item.volSizeInBytes;
        }
      }
    });
    if (isEmpty(datastoreMap)) {
      return true;
    }
    return every(values(datastoreMap), (item: any) => item.total >= item.used);
  }

  // 即时恢复检查目标端置备类型
  getPreallocationOptions(item) {
    let datastore;
    if (this.formGroup.value.targetPool === this.storageLocation.Same) {
      datastore = find(this.targetDatastoreOptions, {
        value: this.formGroup.value.targetRecoveryPool
      });
    } else {
      datastore = find(this.targetDatastoreOptions, {
        value: item.recoveryDatastore
      });
    }
    if (datastore) {
      const details = JSON.parse(datastore.extendInfo?.details || '{}');
      return Number(details.type) === this.NFS_STORAGE_TYPE
        ? this.offPreallocationOptions
        : this.preallocationOptions;
    }
    return [];
  }

  getTargetPath() {
    return this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
      ? this.resourceProperties?.path
      : this.formGroup.value.targetServer[0]?.path;
  }

  getSubObjects() {
    if (this.isDiskRestore) {
      return map(this.recoveryDiskTableData?.data, item => {
        let targetVolume;
        if (item.recoveryDiskType === this.EXIST_DISK) {
          targetVolume = {
            ...pick(
              find(this.targetDisksOptions, {
                uuid: item.recoveryDisk
              }),
              ['uuid', 'name']
            ),
            datastore: '',
            isNewDisk: 'false'
          };
        } else {
          const targetDatastore = find(this.targetDatastoreOptions, {
            value: item.recoveryDatastore
          });
          targetVolume = {
            name: item.targetDiskName,
            uuid: '',
            datastore: {
              poolId: item.recoveryDatastore,
              name: targetDatastore.name,
              details: targetDatastore?.extendInfo?.details
            },
            isNewDisk: 'true'
          };
        }
        return JSON.stringify({
          uuid: item.uuid,
          name: item.name,
          extendInfo: {
            targetVolume: JSON.stringify(targetVolume)
          }
        });
      });
    } else if (this.restoreType === RestoreV2Type.InstanceRestore) {
      const sameDatastore = find(this.targetDatastoreOptions, {
        value: this.formGroup.value.targetRecoveryPool
      });
      return map(this.recoveryDiskTableData?.data, item => {
        const newDatastore =
          this.formGroup.value.targetPool === StorageLocation.Same
            ? sameDatastore
            : find(this.targetDatastoreOptions, {
                value: item.recoveryDatastore
              });
        const targetDatastore =
          this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
            ? item.datastore
            : {
                name: newDatastore?.name,
                poolId: newDatastore?.uuid,
                details: newDatastore?.extendInfo?.details
              };
        return JSON.stringify({
          uuid: item.uuid,
          name: item.name,
          extendInfo: {
            targetVolume: JSON.stringify({
              datastore: targetDatastore,
              preallocation: item.recoveryPreallocation
            })
          }
        });
      });
    } else {
      return map(this.recoveryDiskTableData?.data, item => {
        const newDatastore = find(this.targetDatastoreOptions, {
          value: item.recoveryDatastore
        });
        const targetDatastore =
          this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
            ? item.datastore
            : this.isNutanix
            ? {
                name: newDatastore?.name,
                moRef: newDatastore?.uuid,
                details: newDatastore?.extendInfo?.details
              }
            : {
                name: newDatastore?.name,
                poolId: newDatastore?.uuid,
                details: newDatastore?.extendInfo?.details
              };
        const tmpVolume = {
          datastore: targetDatastore
        };

        if (this.isNutanix) {
          assign(tmpVolume, {
            volSizeInBytes: item.volSizeInBytes
          });
        }

        return JSON.stringify({
          uuid: item.uuid,
          name: item.name,
          extendInfo: {
            targetVolume: JSON.stringify(tmpVolume)
          }
        });
      });
    }
  }

  getTargetObject() {
    if (this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN) {
      if (this.restoreType === RestoreV2Type.InstanceRestore) {
        return this.resourceProperties.parent_uuid;
      }
      return this.resourceProperties.uuid;
    }
    if (this.isNutanix) {
      return this.dealNutanixPath();
    }
    return this.formGroup.value.targetServer[0]?.uuid;
  }

  getRestoreLocation(): string {
    if (this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN) {
      if (this.isDiskRestore) {
        return this.resourceProperties.path;
      } else {
        const paths = this.resourceProperties.path?.split('/');
        paths.pop();
        return paths.join('/');
      }
    } else {
      if (this.isNutanix) {
        return this.dealNutanixPath();
      }
      return this.formGroup.value.targetServer[0]?.path;
    }
  }

  dealNutanixPath() {
    const targetPath = find(this.nutaniHostOptions, {
      value: first(this.formGroup.value.targetServer)
    })?.path;
    const clusterPath = find(this.nutaniClusterOptions, {
      value: this.formGroup.value.targetCluster
    })?.path;
    if (size(this.formGroup.value.targetServer) === 1) {
      return targetPath;
    } else {
      return clusterPath;
    }
  }

  getRestoreLevel(): string {
    // 普通恢复不加锁
    // 即时恢复开启覆盖原机后加锁
    // 磁盘恢复加锁
    return this.isDiskRestore
      ? '1'
      : this.restoreType === RestoreV2Type.InstanceRestore
      ? '2'
      : '0';
  }

  // 虚拟机搜索
  filterChange(value) {
    this.currentSearchKey = value;
    if (value === this.searchVmKey || this.isNutanix) {
      return;
    }
    if (!trim(value)) {
      this.serverTreeData = cloneDeep(this.originalTreeData);
      return;
    }
    this.searchTrigger$.next(trim(value));
  }

  getResourceDetail(data) {
    // 根据CNWareId查询客户端ID
    if (data?.subType !== DataMap.Resource_Type.cNwareVm.value) {
      return;
    }
    // 如果顶层传递了客户端ID，无需再次查询
    if (data?.agentId) {
      this.queryVMInfoById(data.agentId, data);
      return;
    }
    this.protectedResourceApiService
      .ListResources({
        pageNo: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        queryDependency: true,
        conditions: JSON.stringify({
          uuid: this.resourceProperties?.root_uuid
        })
      })
      .subscribe((res: any) => {
        if (first(res.records)) {
          const onlineAgents = res.records[0]?.dependencies?.agents;
          const agentId = onlineAgents[0].uuid;
          this.queryVMInfoById(agentId, data);
        }
      });
  }

  queryVMInfoById(agentId, vm) {
    const params = {
      agentId: agentId,
      envId: vm.rootUuid, // 顶层CNWareID
      resourceIds: [vm.parentUuid], // CNWare主机ID
      pageNo: CommonConsts.PAGE_START_EXTRA,
      pageSize: CommonConsts.PAGE_SIZE_MAX,
      conditions: JSON.stringify({
        resourceType: DataMap.Resource_Type.cNwareVm.value,
        uuid: vm.uuid // 虚拟机ID
      })
    };

    this.appService.ListResourcesDetails(params).subscribe(res => {
      this.getBootOptionsTip(res.records);
    });
  }

  getBootOptionsTip(rowData?) {
    const event = isArray(rowData) ? first(rowData) : rowData;
    if (event?.subType === DataMap.Resource_Type.cNwareVm.value) {
      const details = JSON.parse(get(event, 'extendInfo.details', '{}'));
      getBootTypeWarnTipByType(this, details?.bootType, this.rowCopyBootType);
    } else {
      this.bootOptionsWarnTip = '';
    }
  }

  getParams() {
    const params = {
      copyId: this.rowCopy.uuid,
      agents: this.formGroup.value.proxyHost,
      targetEnv:
        this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
          ? this.resourceProperties?.environment_uuid
          : this.isNutanix
          ? find(this.nutaniClusterOptions, {
              value: this.formGroup.value.targetCluster
            })?.environment?.uuid
          : this.formGroup.value.targetServer[0]?.rootUuid,
      restoreType: this.isDiskRestore
        ? RestoreV2Type.FileRestore
        : this.restoreType,
      targetLocation: this.formGroup.value.restoreTo,
      subObjects: this.getSubObjects(),
      targetObject: this.getTargetObject(),
      extendInfo: {
        restoreLevel: this.getRestoreLevel(),
        powerState: this.formGroup.value.restoreAutoPowerOn ? '1' : '0',
        copyVerify: this.formGroup.value.copyVerify ? 'true' : 'false',
        restoreLocation: this.getRestoreLocation(),
        resourceLockId: this.rowCopy.resource_id
      }
    };

    let detailInfo;
    detailInfo = map(this.networkTableData?.data, item => {
      const targetPortGroup = find(this.targetPortGroupOptions, {
        uuid: item.recoveryPortGroup
      });
      const details = JSON.parse(targetPortGroup.extendInfo?.details || '{}');
      assign(details, { id: item.recoveryPortGroup });
      return {
        bridge: omit(item, ['parent', 'portGroupOptions', 'recoveryPortGroup']),
        portGroup: details
      };
    });

    if (this.isNutanix) {
      detailInfo = map(this.networkTableData?.data, item => {
        return {
          originNicId: item?.uuid,
          targetNetworkId: item.recoveryPortGroup,
          targetIp: item?.targetIp
        };
      });
    }

    if (!this.isDiskRestore) {
      assign(params.extendInfo, {
        vmName: this.formGroup.value.vmName,
        bridgeInterface: JSON.stringify({
          detail: detailInfo
        })
      });
      // 及时恢复原位置删除原机
      if (
        this.restoreType === RestoreV2Type.InstanceRestore &&
        this.formGroup.value.restoreTo === this.restoreLocationType.ORIGIN
      ) {
        assign(params.extendInfo, {
          cleanOriginVM: this.formGroup.value.cleanOriginVM ? '1' : '0'
        });
      }
      assign(params.extendInfo, {
        openInterface: this.formGroup.value.openInterface ? 'true' : 'false'
      });
      if (this.isNutanix) {
        assign(params.extendInfo, {
          hostUuids: JSON.stringify({
            detail: this.formGroup.value.targetServer
          })
        });
      }
    }
    if (this.rowCopy.status === DataMap.copydata_validStatus.invalid.value) {
      assign(params.extendInfo, {
        force_recovery: true
      });
    }
    return params;
  }

  restore(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = this.getParams();
      if (
        this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN &&
        !this.checkOriginalDatastore()
      ) {
        this.messageService.error(
          this.i18n.get('protection_original_capacity_insufficient_label'),
          {
            lvMessageKey: 'error_key_cnware',
            lvShowCloseButton: true
          }
        );
        observer.error(null);
        observer.complete();
        return;
      }
      this.restoreV2Service
        .CreateRestoreTask({ CreateRestoreTaskRequestBody: params })
        .subscribe({
          next: () => {
            observer.next();
            observer.complete();
          },
          error: err => {
            observer.error(err);
            observer.complete();
          }
        });
    });
  }
}
