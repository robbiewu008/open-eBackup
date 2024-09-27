import { Component, OnInit, TemplateRef, ViewChild } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  AppService,
  BaseUtilService,
  CAPACITY_UNIT,
  CommonConsts,
  CopiesService,
  DataMap,
  DataMapService,
  I18NService,
  LiveMountApiService,
  ProtectedResourceApiService,
  ResourceType,
  StorageLocation
} from 'app/shared';
import { TableConfig, TableData } from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  cloneDeep,
  defer,
  each,
  every,
  first,
  includes,
  isEmpty,
  isNumber,
  map,
  reject,
  size
} from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';

@Component({
  selector: 'aui-live-mount-migrate',
  templateUrl: './live-mount-migrate.component.html',
  styleUrls: ['./live-mount-migrate.component.less']
})
export class LiveMountMigrateComponent implements OnInit {
  item;
  formGroup: FormGroup;
  storageLocation = StorageLocation;
  unitconst = CAPACITY_UNIT;
  tableData: TableData;
  tableConfig: TableConfig;
  diffTableConfig: TableConfig;

  serverTreeData = [];
  targetDatastoreOptions = [];
  cacheSelectedDatastore = [];
  preallocationOptions = this.dataMapService
    .toArray('preallocationType')
    .filter(item => {
      return (item.isLeaf = true);
    });

  diskValid$ = new Subject<boolean>();

  @ViewChild('preallocationTpl', { static: true })
  preallocationTpl: TemplateRef<any>;
  @ViewChild('sizeTpl', { static: true }) sizeTpl: TemplateRef<any>;
  @ViewChild('storagePoolTpl', { static: true }) storagePoolTpl: TemplateRef<
    any
  >;

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private appService: AppService,
    private dataMapService: DataMapService,
    private baseUtilService: BaseUtilService,
    private appUtilsService: AppUtilsService,
    private copiesApiService: CopiesService,
    private liveMountApiService: LiveMountApiService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit(): void {
    this.initConfig();
    this.listenForm();
    this.getDisk();
    this.getEnvironment();
    this.getOriginalStoragePool();
  }

  listenForm() {
    this.formGroup.get('targetPool').valueChanges.subscribe(res => {
      if (res === StorageLocation.Same) {
        this.formGroup
          .get('targetRecoveryPool')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup.get('computerLocation').clearValidators();
      } else {
        this.formGroup.get('targetRecoveryPool').clearValidators();
        this.formGroup
          .get('computerLocation')
          .setValidators([this.baseUtilService.VALID.required()]);
      }
      this.formGroup.get('targetRecoveryPool').updateValueAndValidity();
      this.formGroup.get('computerLocation').updateValueAndValidity();
    });
    this.formGroup.get('computerLocation').valueChanges.subscribe(res => {
      if (isEmpty(res)) {
        return;
      }
      defer(() => {
        if (this.formGroup.value.targetPool === StorageLocation.Same) {
          return;
        }
        this.getResourceAgents(first(res));
      });
    });
    this.formGroup.valueChanges.subscribe(() => this.validCheck());
  }

  initConfig() {
    this.formGroup = this.fb.group({
      targetPool: new FormControl(StorageLocation.Same),
      targetRecoveryPool: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      computerLocation: new FormControl([])
    });
    const pagination: any = {
      mode: 'simple',
      showPageSizeOptions: false,
      winTablePagination: true,
      showTotal: true,
      pageSize: CommonConsts.PAGE_SIZE_SMALL
    };
    const baseCols = [
      {
        key: 'name',
        name: this.i18n.get('protection_fc_disk_name_label')
      },
      {
        key: 'type',
        name: this.i18n.get('common_bus_type_label')
      },
      {
        key: 'volSizeInBytes',
        name: this.i18n.get('protection_fc_disk_capacity_label'),
        cellRender: this.sizeTpl
      }
    ];
    this.tableConfig = {
      table: {
        async: false,
        columns: [
          ...baseCols,
          {
            key: 'recoveryPreallocation',
            name: this.i18n.get('protection_target_preallocation_label'),
            width: 200,
            cellRender: this.preallocationTpl
          }
        ],
        compareWith: 'uuid',
        colDisplayControl: false
      },
      pagination: pagination
    };
    this.diffTableConfig = {
      table: {
        async: false,
        columns: [
          ...baseCols,
          {
            key: 'recoveryPool',
            width: 360,
            name: this.i18n.get('protection_target_storage_pool_label'),
            cellRender: this.storagePoolTpl
          },
          {
            key: 'recoveryPreallocation',
            name: this.i18n.get('protection_target_preallocation_label'),
            width: 200,
            cellRender: this.preallocationTpl
          }
        ],
        compareWith: 'uuid',
        colDisplayControl: false
      },
      pagination: pagination
    };
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

  getDisk() {
    const params = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        uuid: this.item?.copy_id,
        resource_sub_type: DataMap.Resource_Type.cNwareVm.value
      })
    };
    this.copiesApiService.queryResourcesV1CopiesGet(params).subscribe(res => {
      const copy = first(res.items);
      if (!copy) {
        return;
      }
      const properties = JSON.parse(copy.properties);
      let needRestoreDisks = properties?.volList;
      if (isEmpty(needRestoreDisks)) {
        needRestoreDisks = properties.extendInfo?.volList || [];
      }
      each(needRestoreDisks, item => {
        assign(item, {
          datastoreOptions: []
        });
      });
      this.tableData = {
        data: needRestoreDisks,
        total: size(needRestoreDisks)
      };
    });
  }

  getOriginalStoragePool() {
    this.protectedResourceApiService
      .ShowResource({ resourceId: this.item?.target_resource_id })
      .subscribe(host => this.getResourceAgents(host));
  }

  getResourceAgents(host) {
    this.protectedResourceApiService
      .ListResources({
        pageNo: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        queryDependency: true,
        conditions: JSON.stringify({
          uuid: host.rootUuid
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
          this.getStoragePool(host.rootUuid, agentsId);
        }
      });
  }

  setTableDatastore() {
    if (!isEmpty(this.tableData?.data)) {
      each(this.tableData?.data, item => {
        item.recoveryDatastore = '';
      });
      this.setDiskDatastoreOptions();
    }
  }

  getStoragePool(envId, agentsId, recordsTemp?: any[], startPage?: number) {
    const params = {
      agentId: agentsId,
      envId: envId,
      resourceIds: [this.item?.target_resource_id],
      pageNo: startPage || 1,
      pageSize: 200,
      conditions: JSON.stringify({
        resourceType: 'StoragePool',
        uuid: this.item?.target_resource_id
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
        this.targetDatastoreOptions = datastores;
        this.cacheSelectedDatastore = [];
        this.setTableDatastore();
        this.validCheck();
        return;
      }
      startPage++;
      this.getStoragePool(envId, agentsId, recordsTemp, startPage);
    });
  }

  validCheck() {
    this.diskValid$.next(
      every(this.tableData?.data, item => {
        if (this.formGroup.value.targetPool === StorageLocation.Same) {
          return !isEmpty(item.recoveryPreallocation);
        }
        return (
          !isEmpty(item.recoveryDatastore) &&
          !isEmpty(item.recoveryPreallocation)
        );
      })
    );
  }

  setDiskDatastoreOptions() {
    each(this.tableData?.data, item => {
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

  preallocationChange() {
    this.validCheck();
  }

  calcDisk(item) {
    if (includes(this.cacheSelectedDatastore, item.uuid)) {
      let usedCapacity = 0;
      each(this.tableData?.data, v => {
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
  }

  calcCapacity() {
    each(this.targetDatastoreOptions, item => this.calcDisk(item));
  }

  datastoreChange() {
    this.cacheSelectedDatastore = reject(
      map(this.tableData?.data, 'recoveryDatastore'),
      item => isEmpty(item)
    );
    this.calcCapacity();
    this.setDiskDatastoreOptions();
    this.validCheck();
  }

  onOK(): Observable<void> {
    return new Observable<any>((observer: Observer<void>) => {
      const disks = map(this.tableData?.data, item => {
        return {
          datastore: {
            moId:
              this.formGroup.value.targetPool === StorageLocation.Same
                ? this.formGroup.value.targetRecoveryPool
                : item.recoveryDatastore
          },
          preallocation: item.recoveryPreallocation,
          diskId: item.uuid
        };
      });
      const params = {
        liveMountId: this.item.id,
        liveMountMigrateRequest: {
          vmWareMigrateParam: {
            diskDatastoreType: this.formGroup.value.targetPool,
            disk: disks
          }
        }
      };
      this.liveMountApiService.migrateUsingPUT(params).subscribe(
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
