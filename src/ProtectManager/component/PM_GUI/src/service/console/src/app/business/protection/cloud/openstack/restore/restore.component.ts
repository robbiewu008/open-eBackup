import { Component, OnInit, TemplateRef, ViewChild } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { OptionItem } from '@iux/live';
import {
  BaseUtilService,
  CapacityCalculateLabel,
  CAPACITY_UNIT,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  ProtectedResourceApiService,
  ResourceType,
  RestoreApiV2Service,
  RestoreV2LocationType
} from 'app/shared';
import {
  ProTableComponent,
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
  includes,
  isEmpty,
  map,
  pick,
  reject,
  size,
  some,
  toNumber,
  trim
} from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';
import { finalize } from 'rxjs/operators';

@Component({
  selector: 'aui-restore',
  templateUrl: './restore.component.html',
  styleUrls: ['./restore.component.less'],
  providers: [CapacityCalculateLabel]
})
export class RestoreComponent implements OnInit {
  rowCopy;
  restoreType;

  _isEmpty = isEmpty;
  resourceProperties;
  resourceServer;
  restoreTableConfig: TableConfig;
  restoreTableData: TableData;
  targetTableConfig: TableConfig;
  targetTableData: TableData;

  formGroup: FormGroup;
  restoreLocationType = RestoreV2LocationType;
  CopyDataVerifyStatus = DataMap.HCSCopyDataVerifyStatus;
  restoreToNewLocationOnly = false;
  serverTreeData = [];
  EXIST_DISK = '1';
  NEW_DISK = '2';
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
  volumeTypeOptions = [];
  proxyOptions = [];
  verifyStatus;
  copyVerifyDisableLabel;
  diskSystemPath = '/dev/vda';

  targetDisksOptions;
  cacheSelectedDisk = [];

  disk$ = new Subject<boolean>();

  @ViewChild('restoreTable', { static: false }) restoreTable: ProTableComponent;
  @ViewChild('targeTable', { static: false }) targeTable: ProTableComponent;
  @ViewChild('diskDeviceTpl', { static: true }) diskDeviceTpl: TemplateRef<any>;
  @ViewChild('sizeTpl', { static: true }) sizeTpl: TemplateRef<any>;
  @ViewChild('recoveryTypeTpl', { static: true }) recoveryTypeTpl: TemplateRef<
    any
  >;

  constructor(
    private fb: FormBuilder,
    public i18n: I18NService,
    private dataMapService: DataMapService,
    private baseUtilService: BaseUtilService,
    private appUtilsService: AppUtilsService,
    private restoreV2Service: RestoreApiV2Service,
    private capacityCalculateLabel: CapacityCalculateLabel,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit(): void {
    this.initConfig();
    this.initForm();
    this.initCopyVerifyDisableLabel();
    this.getProxyOptions();
    this.getResourceServer();
  }

  getResourceServer() {
    if (
      this.rowCopy?.resource_status === DataMap.Resource_Status.notExist.value
    ) {
      this.resourceServer = this.resourceProperties;
      return;
    }
    this.protectedResourceApiService
      .ShowResource({
        resourceId: this.resourceProperties?.uuid,
        akDoException: false
      })
      .pipe(
        finalize(() => {
          this.getTargetDisk(this.resourceServer);
        })
      )
      .subscribe(
        res => {
          this.resourceServer = res;
        },
        () => {
          this.resourceServer = this.resourceProperties;
        }
      );
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

  showTypeWarn() {
    return (
      this.formGroup.value?.restoreTo === RestoreV2LocationType.ORIGIN &&
      some(
        this.targetTableData?.data,
        item =>
          item.recoveryType === this.NEW_DISK &&
          !isEmpty(item.targetVolumeType) &&
          item.targetVolumeType !== item.volumeType
      )
    );
  }

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [
          `${DataMap.globalResourceType.openStackContainer.value}Plugin`
        ]
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
        // 过滤1.2.1版本
        resource = this.baseUtilService.rejectAgentsByVersion(
          resource,
          '1.2.1'
        );
        const hostArray = [];
        each(resource, item => {
          const tmp = item.environment;
          if (tmp.extendInfo.scenario === '0') {
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
    this.resourceProperties = JSON.parse(
      this.rowCopy?.resource_properties || '{}'
    );
    this.restoreToNewLocationOnly =
      this.rowCopy?.generated_by ===
        DataMap.CopyData_generatedType.cascadedReplication.value ||
      this.rowCopy?.resource_status === DataMap.Resource_Status.notExist.value;
    const properties = JSON.parse(this.rowCopy.properties);
    this.verifyStatus = properties?.verifyStatus;

    const colsLeft: TableCols[] = [
      {
        key: 'nameId',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'diskType',
        name: this.i18n.get('common_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Disk_Mode')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Disk_Mode')
        }
      },
      {
        key: 'size',
        name: this.i18n.get('common_capacity_label'),
        cellRender: this.sizeTpl
      }
    ];

    const colsRight: TableCols[] = [
      {
        key: 'nameId',
        width: 300,
        name: this.i18n.get('common_restore_disk_name_label')
      },
      {
        key: 'size',
        width: 120,
        name: this.i18n.get('common_capacity_label'),
        cellRender: this.sizeTpl
      },
      {
        key: 'recoveryType',
        width: 180,
        name: this.i18n.get('common_type_label'),
        cellRender: this.recoveryTypeTpl
      },
      {
        key: 'position',
        name: this.i18n.get('common_target_disk_position_name_label'),
        cellRender: this.diskDeviceTpl
      }
    ];
    this.restoreTableConfig = {
      table: {
        async: false,
        columns: colsLeft,
        compareWith: 'id',
        colDisplayControl: false,
        scroll: { y: '380px' },
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        selectionChange: selection => {
          each(selection, item => {
            assign(item, {
              nameId: `${item.name || '--'}(${item.id})`,
              recoveryType: '1'
            });
          });
          const canceledDisk = differenceBy(
            this.targetTableData?.data,
            selection,
            'id'
          );
          this.cacheSelectedDisk = reject(this.cacheSelectedDisk, item =>
            includes(map(canceledDisk, 'targetDisk'), item)
          );
          this.targetTableData = {
            data: selection,
            total: size(selection)
          };
          if (!isEmpty(this.targetDisksOptions)) {
            each(this.targetTableData.data, item => {
              assign(item, {
                targetDiskOptions: filter(
                  this.targetDisksOptions,
                  value =>
                    value.id === item.targetDisk || this.fiterDisk(value, item)
                )
              });
            });
          }
          each(this.targetTableData.data, item => {
            if (find(this.volumeTypeOptions, { value: item.volumeType })) {
              assign(item, {
                targetVolumeType: item.volumeType
              });
            }
          });
          this.setVaild();
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

    this.targetTableConfig = {
      table: {
        columns: colsRight,
        async: false,
        colDisplayControl: false,
        scroll: { y: '380px' }
      },
      pagination: {
        winTablePagination: true,
        mode: 'simple',
        showPageSizeOptions: false,
        pageSize: CommonConsts.PAGE_SIZE_SMALL
      }
    };

    defer(() => this.getLeftTableData());
  }

  initForm() {
    this.formGroup = this.fb.group({
      restoreTo: new FormControl(RestoreV2LocationType.ORIGIN),
      targetServer: new FormControl([]),
      proxyHost: new FormControl([]),
      restoreAutoPowerOn: new FormControl(false),
      copyVerify: new FormControl(false)
    });

    this.formGroup.get('restoreTo').valueChanges.subscribe(res => {
      this.clearTargetDisk();
      if (res === RestoreV2LocationType.ORIGIN) {
        this.formGroup.get('targetServer').clearValidators();
        defer(() => this.getTargetDisk(this.resourceServer));
      } else {
        this.getTreeData();
        this.formGroup
          .get('targetServer')
          .setValidators([this.baseUtilService.VALID.required()]);
        defer(() =>
          this.getTargetDisk(first(this.formGroup.value.targetServer))
        );
      }
      this.formGroup.get('targetServer').updateValueAndValidity();
    });

    this.formGroup.get('targetServer').valueChanges.subscribe(res => {
      if (isEmpty(res)) {
        return;
      }
      this.getTargetDisk(first(res));
    });

    if (this.restoreToNewLocationOnly) {
      defer(() =>
        this.formGroup.get('restoreTo').setValue(RestoreV2LocationType.NEW)
      );
    }
  }

  clearTargetDisk() {
    each(this.targetTableData?.data, item => {
      item.targetDisk = '';
      assign(item, {
        targetDiskOptions: []
      });
    });
    this.setVaild();
  }

  getTargetDisk(targetServer) {
    if (isEmpty(targetServer)) {
      return;
    }
    this.targetDisksOptions = JSON.parse(
      targetServer.extendInfo?.volInfo || '[]'
    );
    this.cacheSelectedDisk = [];

    if (!isEmpty(this.targetDisksOptions)) {
      this.targetDisksOptions = map(this.targetDisksOptions, disk => {
        return assign(disk, {
          value: disk.id,
          key: disk.id,
          label: `${disk.name || '--'}(${disk.id})`,
          diskType: `${disk.device === this.diskSystemPath}`,
          isLeaf: true
        });
      });
    }

    if (!isEmpty(this.targetTableData?.data)) {
      each(this.targetTableData?.data, item => {
        item.targetDisk = '';
        assign(item, {
          targetDiskOptions: filter(cloneDeep(this.targetDisksOptions), val => {
            return this.fiterDisk(val, item);
          })
        });
      });
    }

    // 获取volumeType
    this.protectedResourceApiService
      .ShowResource({
        resourceId: targetServer.parentUuid || targetServer.parent_uuid,
        akDoException: false
      })
      .subscribe(res => {
        const volumeType = JSON.parse(res.extendInfo.volumeType || '{}');
        this.volumeTypeOptions = map(volumeType, item => {
          return assign(item, {
            label: item.name,
            value: item.name,
            isLeaf: true
          });
        });
      });

    this.setVaild();
  }

  getOptions(subType, params, node?) {
    const extParams = {
      conditions: JSON.stringify(params)
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        if (subType === ResourceType.OPENSTACK_CONTAINER) {
          this.serverTreeData = map(resource, item => {
            return assign(item, {
              label: item.name,
              disabled: true,
              children: [],
              isLeaf: false,
              expanded: false
            });
          });
        } else {
          each(resource, item => {
            const isOpenStackCloudServer =
              item.subType === DataMap.Resource_Type.openStackCloudServer.value;
            node.children.push(
              assign(item, {
                label: item.name,
                disabled: !isOpenStackCloudServer,
                children: isOpenStackCloudServer ? null : [],
                isLeaf: isOpenStackCloudServer,
                expanded: false
              })
            );
          });
          this.serverTreeData = [...this.serverTreeData];
        }
      }
    );
  }

  getTreeData() {
    if (!isEmpty(this.serverTreeData)) {
      return;
    }
    this.getOptions(ResourceType.OPENSTACK_CONTAINER, {
      type: ResourceType.OpenStack,
      subType: ResourceType.OPENSTACK_CONTAINER
    });
  }

  expandedChange(node) {
    if (!node.expanded || node.children?.length) {
      return;
    }
    node.children = [];
    if (node.subType === ResourceType.OPENSTACK_CONTAINER) {
      this.getOptions(
        ResourceType.OpenStackDomain,
        {
          parentUuid: node.uuid,
          visible: ['1']
        },
        node
      );
    } else if (node.subType === ResourceType.OpenStackDomain) {
      this.getOptions(
        ResourceType.OpenStackProject,
        {
          parentUuid: node.uuid
        },
        node
      );
    } else {
      this.getOptions(
        ResourceType.OpenStackCloudServer,
        {
          path: [['=~'], `${node.path}/`],
          subType: [DataMap.Resource_Type.openStackCloudServer.value]
        },
        node
      );
    }
  }

  getLeftTableData() {
    const properties = JSON.parse(this.rowCopy.properties || '{}');
    let needRestoreDisks = properties?.volList;
    if (isEmpty(needRestoreDisks)) {
      needRestoreDisks = properties.extendInfo?.volList || [];
    }
    each(needRestoreDisks, item => {
      const capacity: string = this.capacityCalculateLabel
        .transform(item.volSizeInBytes, '1.0-0', CAPACITY_UNIT.BYTE, true)
        .replace(/,/g, '');
      assign(item, {
        nameId: `${item.name || '--'}(${item.uuid})`,
        volumeType: item.volume_type,
        id: item.uuid,
        diskType: `${item.attachments[0]?.device === this.diskSystemPath}`,
        size: item.volSizeInBytes / 1024 / 1024 / 1024,
        sizeDisplay: `${parseFloat(capacity)} ${trim(
          capacity.replace(/[0-9.]/g, '')
        )}`
      });
    });
    this.restoreTableData = {
      data: needRestoreDisks,
      total: size(needRestoreDisks)
    };
  }

  fiterDisk(value, item) {
    // 数据盘不能恢复到系统盘
    if (item.bootable === DataMap.Disk_Mode.false.value) {
      return (
        !includes(this.cacheSelectedDisk, value.id) &&
        +value.size >= +item.size &&
        value.bootable === DataMap.Disk_Mode.false.value &&
        value.device !== this.diskSystemPath
      );
    }
    return (
      !includes(this.cacheSelectedDisk, value.id) &&
      +value.size >= +item.size &&
      value.device !== this.diskSystemPath
    );
  }

  setVaild() {
    this.disk$.next(
      !isEmpty(this.targetTableData?.data) &&
        this.formGroup.valid &&
        every(this.targetTableData?.data, item => {
          return item.recoveryType === this.EXIST_DISK
            ? !isEmpty(item.targetDisk)
            : !isEmpty(item.targetDiskName) && !isEmpty(item.targetVolumeType);
        })
    );
  }

  diskNameChange() {
    this.setVaild();
  }

  diskDomainChange() {
    this.setVaild();
  }

  diskChange(_, disk) {
    this.cacheSelectedDisk = reject(
      map(this.targetTableData?.data, 'targetDisk'),
      item => isEmpty(item)
    );
    each(this.targetTableData?.data, item => {
      if (item.id === disk.id) {
        return;
      }
      item.targetDiskOptions = filter(
        this.targetDisksOptions,
        value => value.id === item.targetDisk || this.fiterDisk(value, item)
      );
    });
    this.setVaild();
  }

  getTargetPath() {
    return this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
      ? this.resourceProperties.path
      : this.formGroup.value.targetServer[0]?.path;
  }

  restore(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = {
        copyId: this.rowCopy.uuid,
        agents: this.formGroup.value.proxyHost,
        targetEnv:
          this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
            ? this.resourceProperties?.environment_uuid
            : this.formGroup.value.targetServer[0]?.rootUuid,
        restoreType: this.restoreType,
        targetLocation: this.formGroup.value.restoreTo,
        subObjects: map(this.targetTableData.data, item => {
          let curData;
          if (item.recoveryType === this.EXIST_DISK) {
            curData = pick(
              find(this.targetDisksOptions, {
                id: item.targetDisk
              }),
              ['id', 'size']
            );
          } else {
            curData = {
              name: item.targetDiskName,
              volumeTypeName: item.targetVolumeType,
              size: toNumber(item.size),
              isNewDisk: 'true'
            };
          }

          return JSON.stringify({
            uuid: item.id,
            extendInfo: {
              targetVolume: JSON.stringify(curData)
            }
          });
        }),
        targetObject:
          this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
            ? this.resourceProperties.uuid
            : this.formGroup.value.targetServer[0]?.uuid,
        extendInfo: {
          powerState: this.formGroup.value.restoreAutoPowerOn ? '1' : '0',
          copyVerify: this.formGroup.value.copyVerify ? 'true' : 'false',
          restoreLocation:
            this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
              ? this.resourceProperties.path
              : this.formGroup.value.targetServer[0]?.path
        }
      };
      if (this.rowCopy.status === DataMap.copydata_validStatus.invalid.value) {
        assign(params.extendInfo, {
          force_recovery: true
        });
      }
      this.restoreV2Service
        .CreateRestoreTask({ CreateRestoreTaskRequestBody: params })
        .subscribe(
          () => {
            observer.next();
            observer.complete();
          },
          err => {
            observer.error(err);
            observer.complete();
          }
        );
    });
  }
}
