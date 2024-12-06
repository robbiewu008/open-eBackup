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
import { DatePipe } from '@angular/common';
import {
  AfterViewInit,
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  Input,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { MessageService, ModalRef } from '@iux/live';
import { BackupsetRestoreComponent as ElasticsearchRestoreComponent } from 'app/business/protection/big-data/elasticSearch/backupset-restore/backupset-restore.component';
import { BackupSetRestoreComponent } from 'app/business/protection/big-data/hbase/backup-set/copy-data/backup-set-restore/backup-set-restore.component';
import { HdfsFilesetRestoreComponent } from 'app/business/protection/big-data/hdfs/filesets/copy-data/hdfs-fileset-restore/hdfs-fileset-restore.component';
import { BackupsetRestoreComponent as HiveBackupsetRestoreComponent } from 'app/business/protection/big-data/hive/backupset-restore/backupset-restore.component';
import { ClickHouseRestoreComponent } from 'app/business/protection/host-app/click-house/copy-data/click-house-restore/click-house-restore.component';
import { FilesetRestoreComponent } from 'app/business/protection/host-app/fileset/fileset-restore/fileset-restore.component';
import { ClusterRestoreComponent as DWSClusterRestoreComponent } from 'app/business/protection/host-app/gaussdb-dws/restore-cluster/restore-cluster.component';
import { DatabaseRestoreComponent as DWSDatabaseRestoreComponent } from 'app/business/protection/host-app/gaussdb-dws/restore-database/restore-database.component';
import { TableRestoreComponent as DWSTableRestoreComponent } from 'app/business/protection/host-app/gaussdb-dws/restore-table/restore-table.component';
import { OceanBaseRestoreComponent } from 'app/business/protection/host-app/ocean-base/ocean-base-restore/ocean-base-restore.component';
import { SQLServerAlwaysOnComponent as SQLServerGroupRestoreComponent } from 'app/business/protection/host-app/sql-server/alwayson-restore/alwayson-restore.component';
import { InstanceRestoreComponent as SQLServerInstanceRestoreComponent } from 'app/business/protection/host-app/sql-server/instance-restore/instance-restore.component';
import { SQLServerRestoreComponent as SQLServerDatabaseRestoreComponent } from 'app/business/protection/host-app/sql-server/sql-server-restore/sql-server-restore.component';
import { TidbRestoreComponent } from 'app/business/protection/host-app/tidb/tidb-restore/tidb-restore.component';
import { LocalFileSystemRestoreComponent } from 'app/business/protection/storage/local-file-system/local-file-system-restore/local-file-system-restore.component';
import { ObjectRestoreComponent } from 'app/business/protection/storage/object/object-service/object-restore/object-restore.component';
import { MODAL_COMMON } from 'app/shared';
import {
  ApiStorageBackupPluginService,
  CopyControllerService,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  RestoreApiV2Service,
  RestoreManagerService as RestoreService
} from 'app/shared/api/services';
import {
  CAPACITY_UNIT,
  CommonConsts,
  DataMap,
  LANGUAGE,
  ResourceType,
  RestoreFileType,
  RestoreLocationType,
  RestoreV2LocationType,
  RestoreV2Type,
  SYSTEM_TIME
} from 'app/shared/consts';
import { I18NService } from 'app/shared/services';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { InfoMessageService } from 'app/shared/services/info-message.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  cloneDeep,
  each,
  filter,
  find,
  findIndex,
  first,
  get,
  includes,
  intersectionWith,
  isArray,
  isEmpty,
  isNumber,
  isString,
  join,
  last,
  map,
  reject,
  set,
  size,
  slice,
  split,
  startsWith,
  toNumber,
  trim,
  unionBy,
  uniqueId
} from 'lodash';
import { combineLatest, Observable, Observer } from 'rxjs';
import { DoradoNasRestoreComponent } from '../dorado-nas-restore/dorado-nas-restore.component';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from '../pro-table';

@Component({
  selector: 'aui-file-level-restore',
  templateUrl: './file-level-restore.component.html',
  styleUrls: ['./file-level-restore.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush,
  providers: [DatePipe]
})
export class FileLevelRestoreComponent implements OnInit, AfterViewInit {
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  @Input() restoreLevel;

  @ViewChild('dateTpl', { static: true }) dateTpl: TemplateRef<any>;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  _get = get;
  isDameng = true;
  isOceanBase = true;
  selectTips = this.i18n.get('protection_target_input_tips_label');
  subObjects = [];
  selectionAssociate = false;
  damengTargetLocation1;
  tableConfig: TableConfig;
  tableData: TableData;
  selectionData = [];
  unitconst = CAPACITY_UNIT;
  restoreFileType = RestoreFileType;
  restoreV2LocationType = RestoreV2LocationType;
  restoreLocationType = RestoreLocationType;
  dataMap = DataMap;
  language = LANGUAGE;
  timeZone = SYSTEM_TIME.timeZone;
  originalFileData = [];
  originalSelection = [];
  total = 0;
  selectFileData = [];
  mountedSelection;
  targetParams;
  inputTarget = '';
  fileSystemMountId;
  selectedLength = 0;
  fileLevelRestoreTips = '';
  databaseOptions = [];
  clusterOptions = [];
  tenantOptions = [];
  targetResourcePoolOptions = [];
  database = '';
  cluster = '';
  tenant = '';
  version;
  targetName = '';
  resourcePool = '';
  showTenantError: boolean = false;
  name;
  dwsNotAllowedSchemaOption = []; // 用于新位置判断重名且不可恢复schema
  newNameErrorTip = {
    invalidName: this.i18n.get('protection_dws_new_name_error_tips_label'),
    invalidSpecialName: this.i18n.get(
      'protection_dws_new_special_name_error_tips_label'
    ),
    invalidNotAllowedName: this.i18n.get(
      'protection_dws_not_allowed_schema_name_label'
    )
  };

  isOceanProtect = !includes(
    [
      DataMap.Deploy_Type.cloudbackup2.value,
      DataMap.Deploy_Type.cloudbackup.value,
      DataMap.Deploy_Type.hyperdetect.value,
      DataMap.Deploy_Type.cyberengine.value
    ],
    this.i18n.get('deploy_type')
  );

  // 文件系统类
  isFileSystemApp = false;
  modeMap = {
    fromTree: '1',
    fromTag: '2'
  };
  pathMode = this.modeMap.fromTree;
  manualInputPath = [];

  rowCopyResPro;
  @ViewChild('searchPopover', { static: false }) searchPopover;

  constructor(
    private datePipe: DatePipe,
    private modal: ModalRef,
    public i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private messageService: MessageService,
    private restoreService: RestoreService,
    private drawModalService: DrawModalService,
    private restoreV2Service: RestoreApiV2Service,
    private infoMessageService: InfoMessageService,
    private copyControllerService: CopyControllerService,
    private appUtilsService: AppUtilsService,
    private virtualScroll: VirtualScrollService,
    private apiStorageBackupPluginService: ApiStorageBackupPluginService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.rowCopyResPro = JSON.parse(this.rowCopy.resource_properties || '{}');
    this.selectionAssociate = includes(
      [
        DataMap.Resource_Type.DWS_Cluster.value,
        DataMap.Resource_Type.DWS_Database.value,
        DataMap.Resource_Type.DWS_Schema.value,
        DataMap.Resource_Type.DWS_Table.value,
        DataMap.Resource_Type.NASShare.value,
        DataMap.Resource_Type.NASFileSystem.value,
        DataMap.Resource_Type.ndmp.value,
        DataMap.Resource_Type.fileset.value
      ],
      this.childResType
    );
    this.isFileSystemApp = includes(
      [
        DataMap.Resource_Type.NASShare.value,
        DataMap.Resource_Type.NASFileSystem.value,
        DataMap.Resource_Type.ndmp.value,
        DataMap.Resource_Type.fileset.value,
        DataMap.Resource_Type.volume.value,
        DataMap.Resource_Type.HDFSFileset.value
      ],
      this.childResType
    );
    this.getfileLevelRestoreTips();
    this.isDameng = includes(
      [
        DataMap.Resource_Type.Dameng_singleNode.value,
        DataMap.Resource_Type.Dameng_cluster.value
      ],
      this.childResType
    );
    this.isOceanBase = includes(
      [DataMap.Resource_Type.OceanBaseCluster.value],
      this.childResType
    );
    if (
      includes(
        [DataMap.Resource_Type.OceanBaseCluster.value],
        this.childResType
      )
    ) {
      const tenantArray = JSON.parse(this.rowCopy.properties).tenant_list;
      this.tenantOptions = map(tenantArray, item => {
        return {
          key: item.id,
          value: item.id,
          label: item.name,
          isLeaf: true,
          ...item
        };
      });
      this.getClusterOptions();
    }
    if (this.isDameng) {
      this.damengTargetLocation1 = [{ name: this.rowCopy.resource_name }];
      this.inputTarget = this.rowCopy.resource_name;
      this.initForm();
    } else if (this.isOceanBase) {
      this.getSchema(`/${get(this.tenant, 'name')}`);
    } else {
      this.getOriginalPath();
    }

    if (
      includes([DataMap.Resource_Type.DWS_Cluster.value], this.childResType)
    ) {
      this.getDatabaseOptions();
    }
  }

  getDataCompareWithKey(resource_sub_type) {
    switch (resource_sub_type) {
      case DataMap.Resource_Type.ElasticsearchBackupSet.value:
      case DataMap.Resource_Type.SQLServerGroup.value:
      case DataMap.Resource_Type.SQLServerInstance.value:
      case DataMap.Resource_Type.SQLServerClusterInstance.value:
        return 'name';
      case DataMap.Resource_Type.tidbCluster.value:
      case DataMap.Resource_Type.tidbDatabase.value:
      case DataMap.Resource_Type.DWS_Cluster.value:
      case DataMap.Resource_Type.DWS_Table.value:
      case DataMap.Resource_Type.DWS_Schema.value:
        return 'rootPath';
      default:
        return null;
    }
  }

  getTargetResourcePoolOptions(envId) {
    if (!envId) {
      return;
    }
    this.targetResourcePoolOptions = [];
    this.resourcePool = '';
    this.modal.getInstance().lvOkDisabled = true;
    const extParams = {
      envId,
      conditions: JSON.stringify({ queryType: 'pool' })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params =>
        this.protectedEnvironmentApiService.ListEnvironmentResource(params),
      resource => {
        let poolInfo = [];
        const clusterInfo = get(resource[0], 'extendInfo.clusterInfo');
        if (clusterInfo) {
          poolInfo = get(JSON.parse(clusterInfo), 'pools');
        }
        this.targetResourcePoolOptions = map(poolInfo, data => {
          return {
            value: data.resource_pool_id,
            key: data.resource_pool_id,
            label: data.resource_pool_name,
            isLeaf: true,
            disabled: false
          };
        });
        this.cdr.detectChanges();
      }
    );
  }

  getDatabaseOptions(recordsTemp?, startPage?) {
    this.copyControllerService
      .ListCopyCatalogs({
        pageNo: startPage || CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE * 10,
        copyId: this.rowCopy.uuid,
        parentPath: `/${get(this.rowCopyResPro, 'name')}`
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
          startPage ===
            Math.ceil(res.totalCount / (CommonConsts.PAGE_SIZE * 10)) ||
          res.totalCount === 0
        ) {
          if (res.totalCount !== 0) {
            this.databaseOptions = map(recordsTemp, item => {
              return {
                ...item,
                label: item.path,
                key: item.path,
                value: `/${get(this.rowCopyResPro, 'name')}/${item.path}`,
                isLeaf: true
              };
            });
            this.database = first(this.databaseOptions)?.value;
            this.cdr.detectChanges();
            this.getSchema(this.database);
          }
          return;
        }
        this.getDatabaseOptions(recordsTemp, startPage);
      });
  }

  getClusterOptions(recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        subType: DataMap.Resource_Type.OceanBaseCluster.value
      })
    };
    this.protectedResourceApiService.ListResources(params).subscribe(res => {
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
          Math.ceil(res.totalCount / (CommonConsts.PAGE_SIZE * 10)) ||
        res.totalCount === 0
      ) {
        recordsTemp = recordsTemp.filter(
          item =>
            item.linkStatus === DataMap.resource_LinkStatus_Special.normal.value
        );
        this.clusterOptions = map(recordsTemp, item => {
          return assign(item, {
            value: item.uuid,
            key: item.uuid,
            label: item.name,
            isLeaf: true
          });
        });
        this.cdr.detectChanges();
        return;
      }
      this.getClusterOptions(recordsTemp, startPage);
    });
  }

  getSchema(opt, isSearch = false, recordsTemp?, startPage?) {
    this.name = '';
    this.copyControllerService
      .ListCopyCatalogs({
        pageNo: startPage || CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE * 10,
        copyId: this.rowCopy.uuid,
        parentPath: opt
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
          startPage ===
            Math.ceil(res.totalCount / (CommonConsts.PAGE_SIZE * 10)) ||
          res.totalCount === 0
        ) {
          each(recordsTemp, item => {
            assign(item, {
              name: get(item, 'path'),
              isLeaf: false,
              children: [],
              disabled: true,
              rootPath: `${opt}/${get(item, 'path')}`,
              type: RestoreFileType.Directory,
              icon: 'aui-icon-dws-schema'
            });
          });

          if (
            this.targetParams?.restore_location === RestoreLocationType.NEW ||
            this.targetParams?.restoreLocation === RestoreV2LocationType.NEW
          ) {
            this.selectFileData = [];
          }
          if (!isSearch) {
            this.originalSelection = [];
            this.selectedLength = 0;
          }
          this.modal.getInstance().lvOkDisabled = true;
          this.originalFileData = [...recordsTemp];
          this.total = res.totalCount;
          this.cdr.detectChanges();
          return;
        }
        this.getSchema(opt, isSearch, recordsTemp, startPage);
      });
  }

  getNewLocationSchema() {
    const extParams = {
      conditions: JSON.stringify({
        subType: DataMap.Resource_Type.DWS_Schema.value,
        environment: { name: [['~~'], this.targetParams.resource.name] }
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        each(resource, item => {
          if (get(item, 'extendInfo.isAllowRestore', 'false') === 'false') {
            this.dwsNotAllowedSchemaOption.push(
              ...item.extendInfo.table.split(',')
            );
          }
        });
      }
    );
  }

  getfileLevelRestoreTips() {
    let restoreType = this.i18n.get('common_files_label');

    switch (this.childResType) {
      case DataMap.Resource_Type.ElasticsearchBackupSet.value:
        restoreType = this.i18n.get('common_index_label');
        break;
      case DataMap.Resource_Type.HBaseBackupSet.value:
      case DataMap.Resource_Type.HiveBackupSet.value:
      case DataMap.Resource_Type.ClickHouse.value:
      case DataMap.Resource_Type.DWS_Cluster.value:
      case DataMap.Resource_Type.DWS_Database.value:
      case DataMap.Resource_Type.DWS_Schema.value:
      case DataMap.Resource_Type.DWS_Table.value:
      case DataMap.Resource_Type.tidbDatabase.value:
      case DataMap.Resource_Type.tidbCluster.value:
      case DataMap.Resource_Type.OceanBaseCluster.value:
        restoreType = this.i18n.get('common_table_label');
        break;
      case DataMap.Resource_Type.SQLServerClusterInstance.value:
      case DataMap.Resource_Type.SQLServerInstance.value:
      case DataMap.Resource_Type.SQLServerGroup.value:
        restoreType = this.i18n.get('common_database_label');
        break;
      case DataMap.Resource_Type.ObjectSet.value:
        restoreType = this.i18n.get('protection_object_bucket_label');
        break;
      default:
        restoreType = this.i18n.get('common_files_label');
    }

    // 特殊提示单独处理
    if (
      includes(
        [DataMap.Resource_Type.Dameng_singleNode.value],
        this.childResType
      )
    ) {
      this.fileLevelRestoreTips = this.i18n.get(
        'protection_dameng_filelevel_restore_tip_label'
      );
      return this.i18n.get('common_file_table_level_label');
    }

    this.fileLevelRestoreTips = this.i18n.get(
      'protection_file_level_restore_tip_label',
      [restoreType]
    );
    return restoreType;
  }

  initForm() {
    const cols: TableCols[] = [
      {
        key: 'path',
        name: this.i18n.get('common_name_label'),
        cellRender: this.dateTpl
      }
    ];
    this.tableConfig = {
      pagination: {
        mode: 'simple'
      },
      table: {
        columns: cols,
        compareWith: 'path',
        virtualScroll: true,
        scrollFixed: true,
        scroll: {
          y: '684px'
        },
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        fetchData: (filter: Filters, args) => {
          this.getData(filter, args);
        },
        colDisplayControl: false,
        selectionChange: selection => {
          this.selectionData = selection;
          this.disabledOkbtn();
        },
        trackByFn: (index, item) => {
          return item.uuid;
        }
      }
    };
  }
  ngAfterViewInit(): void {
    if (this.isDameng) {
      this.dataTable.fetchData();
    }
  }
  getData(filters: Filters, args: { isAutoPolling: any }) {
    this.copyControllerService
      .ListCopyCatalogs({
        pageNo: filters.paginator.pageIndex,
        pageSize: filters.paginator.pageSize,
        copyId: this.rowCopy.uuid,
        parentPath: '/'
      })
      .subscribe((res: any) => {
        this.subObjects = map(res.records, record => ({
          name: record.path
        }));
        this.tableData = {
          data: res.records,
          total: res.totalCount
        };
        this.cdr.detectChanges();
      });
  }

  turnPage(filter) {
    if (
      includes(
        [
          DataMap.Resource_Type.Hive.value,
          DataMap.Resource_Type.Elasticsearch.value
        ],
        this.rowCopy.resource_type
      ) ||
      includes(
        [
          DataMap.Resource_Type.DWS_Database.value,
          DataMap.Resource_Type.DWS_Schema.value,
          DataMap.Resource_Type.DWS_Table.value,
          DataMap.Resource_Type.ClickHouse.value,
          DataMap.Resource_Type.tidbCluster.value,
          DataMap.Resource_Type.tidbDatabase.value,
          DataMap.Resource_Type.SQLServerInstance.value,
          DataMap.Resource_Type.SQLServerClusterInstance.value,
          DataMap.Resource_Type.SQLServerGroup.value
        ],
        this.rowCopy.resource_sub_type
      )
    ) {
      this.getTables(filter.pageIndex);
    }
  }

  getOriginalPath() {
    if (
      includes(
        [
          DataMap.Resource_Type.Elasticsearch.value,
          DataMap.Resource_Type.Hive.value
        ],
        this.rowCopy.resource_type
      ) ||
      includes(
        [
          DataMap.Resource_Type.DWS_Database.value,
          DataMap.Resource_Type.DWS_Schema.value,
          DataMap.Resource_Type.DWS_Table.value,
          DataMap.Resource_Type.ClickHouse.value,
          DataMap.Resource_Type.SQLServerGroup.value,
          DataMap.Resource_Type.SQLServerClusterInstance.value,
          DataMap.Resource_Type.SQLServerInstance.value,
          DataMap.Resource_Type.tidbDatabase.value,
          DataMap.Resource_Type.tidbCluster.value
        ],
        this.rowCopy.resource_sub_type
      )
    ) {
      this.getTables();
    } else if (
      includes(
        [DataMap.Resource_Type.DWS_Cluster.value],
        this.rowCopy.resource_sub_type
      )
    ) {
      this.originalFileData = [
        {
          children: [],
          hasChildren: true,
          name: this.rowCopy.resource_name,
          parent_uuid: null,
          path: '/',
          rootPath: '/',
          disabled: true,
          icon: 'aui-icon-nas-file'
        }
      ];
    } else {
      this.originalFileData = [
        {
          children: [],
          hasChildren: true,
          name: this.rowCopy.resource_name,
          parent_uuid: null,
          path: '/',
          rootPath: '/',
          icon:
            this.rowCopy.resource_sub_type ===
            DataMap.Resource_Type.HDFSFileset.value
              ? 'aui-icon-hdfs-file'
              : this.rowCopy.resource_sub_type ===
                DataMap.Resource_Type.HBaseBackupSet.value
              ? 'aui-icon-cluster'
              : includes(
                  [
                    DataMap.Resource_Type.fileset.value,
                    DataMap.Resource_Type.volume.value
                  ],
                  this.rowCopy.resource_sub_type
                )
              ? 'aui-icon-directory'
              : 'aui-icon-nas-file'
        }
      ];
    }
  }

  getTables(startPage?) {
    let path = '/';
    if (
      includes(
        [DataMap.Resource_Type.DWS_Database.value],
        this.rowCopy.resource_sub_type
      )
    ) {
      path = `/${this.rowCopyResPro['parent_name']}/${this.rowCopyResPro['name']}`;
    } else if (
      includes(
        [
          DataMap.Resource_Type.DWS_Schema.value,
          DataMap.Resource_Type.DWS_Table.value
        ],
        this.rowCopy.resource_sub_type
      )
    ) {
      path = `/${this.rowCopyResPro['environment_name']}/${this.rowCopyResPro['parent_name']}`;
    } else if (
      [DataMap.Resource_Type.ClickHouse.value].includes(
        this.rowCopy.resource_sub_type
      )
    ) {
      if (this.rowCopy.resource_type === ResourceType.DATABASE) {
        path = `/${this.rowCopyResPro['name']}`;
      } else {
        path = `/${this.rowCopyResPro['parent_name']}`;
      }
    } else if (
      [DataMap.Resource_Type.tidbCluster.value].includes(
        this.rowCopy.resource_sub_type
      )
    ) {
      path = `/${this.rowCopyResPro.extendInfo.clusterName}`;
    } else if (
      [DataMap.Resource_Type.tidbDatabase.value].includes(
        this.rowCopy.resource_sub_type
      )
    ) {
      const tmp = this.rowCopyResPro.extendInfo;
      path = `/${tmp.clusterName}/${tmp.databaseName}`;
    }

    this.copyControllerService
      .ListCopyCatalogs({
        pageNo: startPage || CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE * 10,
        copyId: this.rowCopy.uuid,
        parentPath: path
      })
      .subscribe(res => {
        each(res.records, item => {
          assign(item, {
            name:
              this.childResType ===
                DataMap.Resource_Type.ElasticsearchBackupSet.value &&
              JSON.parse(get(item, 'extendInfo'))['TYPE'] === 'DATA_STREAM'
                ? `${get(item, 'path')}(${this.i18n.get(
                    'protection_data_stream_label'
                  )})`
                : get(item, 'path'),
            isLeaf: !(
              includes(
                ['cluster', 'database', 'schema', 'file'],
                item['type']
              ) && this.restoreLevel !== 'schema'
            ),
            children:
              includes(['cluster', 'database', 'schema'], item['type']) &&
              this.restoreLevel !== 'schema'
                ? []
                : null,
            disabled:
              includes(['cluster', 'database', 'schema'], item['type']) &&
              this.restoreLevel !== 'schema',
            rootPath: includes(
              [
                DataMap.Resource_Type.DWS_Database.value,
                DataMap.Resource_Type.DWS_Schema.value,
                DataMap.Resource_Type.DWS_Table.value,
                DataMap.Resource_Type.tidbCluster.value,
                DataMap.Resource_Type.tidbDatabase.value
              ],
              this.rowCopy.resource_sub_type
            )
              ? `${path}/${get(item, 'path')}`
              : '/',
            type: includes(
              [
                DataMap.Resource_Type.DWS_Database.value,
                DataMap.Resource_Type.DWS_Schema.value,
                DataMap.Resource_Type.ClickHouse.value,
                DataMap.Resource_Type.DWS_Table.value,
                DataMap.Resource_Type.tidbCluster.value
              ],
              this.rowCopy.resource_sub_type
            )
              ? RestoreFileType.Directory
              : RestoreFileType.File,
            icon: includes(
              [
                DataMap.Resource_Type.DWS_Database.value,
                DataMap.Resource_Type.DWS_Schema.value,
                DataMap.Resource_Type.DWS_Table.value
              ],
              this.rowCopy.resource_sub_type
            )
              ? 'aui-icon-dws-schema'
              : this.rowCopy.resource_sub_type ===
                DataMap.Resource_Type.tidbCluster.value
              ? 'aui-icon-database'
              : 'aui-icon-file'
          });
        });
        this.originalFileData = [...res.records];
        this.total = res.totalCount;
        this.cdr.detectChanges();
        // 切换分页后，回显选中的数据，手动模拟lvCompareWith="path"
        if (
          includes(
            [
              DataMap.Resource_Type.HiveBackupSet.value,
              DataMap.Resource_Type.ElasticsearchBackupSet.value,
              DataMap.Resource_Type.ClickHouse.value
            ],
            this.rowCopy.resource_sub_type
          )
        ) {
          each(this.originalSelection, item => {
            const table = find(
              this.originalFileData,
              child => child.path === item.path
            );

            if (!!table) {
              const index = findIndex(
                this.originalSelection,
                child => get(child, 'path') === item.path
              );
              this.originalSelection[index] = table;
            }
          });
          this.selectionChange();
        }
      });
  }

  searchSource(e) {
    if (
      ![
        DataMap.Resource_Type.ElasticsearchBackupSet.value,
        DataMap.Resource_Type.tidbCluster.value,
        DataMap.Resource_Type.tidbDatabase.value,
        DataMap.Resource_Type.DWS_Cluster.value,
        DataMap.Resource_Type.DWS_Schema.value,
        DataMap.Resource_Type.DWS_Table.value
      ].includes(this.rowCopy.resource_sub_type)
    ) {
      this.originalSelection = [];
      this.selectedLength = 0;
    }
    if (
      [DataMap.Resource_Type.ElasticsearchBackupSet.value].includes(
        this.rowCopy.resource_sub_type
      )
    ) {
      // 搜索只能搜索单个文件，搜索时不应该展示分页器
      this.total = 0;
    }
    if (!e) {
      if (
        includes([DataMap.Resource_Type.DWS_Cluster.value], this.childResType)
      ) {
        this.getSchema(this.database, true);
      } else {
        this.getTables();
      }
    } else {
      this.searchPopover.hide();
      let path = '/';
      if (
        includes(
          [DataMap.Resource_Type.DWS_Database.value],
          this.rowCopy.resource_sub_type
        )
      ) {
        path = `/${this.rowCopyResPro['parent_name']}/${this.rowCopyResPro['name']}`;
      } else if (
        includes(
          [
            DataMap.Resource_Type.DWS_Schema.value,
            DataMap.Resource_Type.DWS_Table.value
          ],
          this.rowCopy.resource_sub_type
        )
      ) {
        path = `/${this.rowCopyResPro['environment_name']}/${this.rowCopyResPro['parent_name']}`;
      } else if (
        includes(
          [DataMap.Resource_Type.ElasticsearchBackupSet.value],
          this.rowCopy.resource_sub_type
        )
      ) {
        path = '/';
      } else if (
        includes(
          [DataMap.Resource_Type.tidbCluster.value],
          this.rowCopy.resource_sub_type
        )
      ) {
        path = `/${
          JSON.parse(this.rowCopy.resource_properties || '{}')?.extendInfo
            ?.clusterName
        }`;
      } else if (
        [DataMap.Resource_Type.tidbDatabase.value].includes(
          this.rowCopy.resource_sub_type
        )
      ) {
        const tmp = this.rowCopyResPro.extendInfo;
        path = `/${tmp.clusterName}/${tmp.databaseName}`;
      } else {
        const database = find(
          this.databaseOptions,
          val => val.value === this.database
        );
        path = `/${get(this.rowCopyResPro, 'name')}/${database.path}`;
      }
      this.copyControllerService
        .ListCopyCatalogsByName({
          parentPath: path,
          copyId: this.rowCopy.uuid,
          name: e
        })
        .subscribe((res: any) => {
          each(res.records, item => {
            assign(item, {
              name:
                this.childResType ===
                  DataMap.Resource_Type.ElasticsearchBackupSet.value &&
                JSON.parse(get(item, 'extendInfo'))['TYPE'] === 'DATA_STREAM'
                  ? `${get(item, 'path')}(${this.i18n.get(
                      'protection_data_stream_label'
                    )})`
                  : get(item, 'path'),
              isLeaf: !(
                includes(
                  ['cluster', 'database', 'schema', 'file'],
                  item['type']
                ) && this.restoreLevel !== 'schema'
              ),
              children:
                includes(['cluster', 'database', 'schema'], item['type']) ||
                item.type === RestoreFileType.Directory
                  ? item?.children || []
                  : null,
              disabled:
                includes(['cluster', 'database', 'schema'], item['type']) &&
                this.restoreLevel !== 'schema',
              rootPath: includes(
                [
                  DataMap.Resource_Type.DWS_Database.value,
                  DataMap.Resource_Type.DWS_Schema.value,
                  DataMap.Resource_Type.DWS_Table.value,
                  DataMap.Resource_Type.tidbCluster.value,
                  DataMap.Resource_Type.tidbDatabase.value
                ],
                this.rowCopy.resource_sub_type
              )
                ? `${path}/${get(item, 'path')}`
                : '/',
              type: includes(
                [
                  DataMap.Resource_Type.DWS_Database.value,
                  DataMap.Resource_Type.DWS_Schema.value,
                  DataMap.Resource_Type.DWS_Table.value,
                  DataMap.Resource_Type.tidbCluster.value
                ],
                this.rowCopy.resource_sub_type
              )
                ? RestoreFileType.Directory
                : RestoreFileType.File,
              icon: includes(
                [
                  DataMap.Resource_Type.DWS_Database.value,
                  DataMap.Resource_Type.DWS_Schema.value,
                  DataMap.Resource_Type.DWS_Table.value
                ],
                this.rowCopy.resource_sub_type
              )
                ? 'aui-icon-dws-schema'
                : this.rowCopy.resource_sub_type ===
                  DataMap.Resource_Type.tidbCluster.value
                ? 'aui-icon-database'
                : 'aui-icon-file'
            });
            if (item?.children && !!item.children?.items?.length) {
              each(item.children.items, val => {
                assign(val, {
                  name: get(val, 'path'),
                  isLeaf: true,
                  children: null,
                  disabled: false,
                  rootPath: `${path}/${get(item, 'path')}/${get(val, 'path')}`,
                  icon: 'aui-icon-file'
                });
              });
              const tmpChildren = item.children.items;
              set(item, 'children', tmpChildren);
              set(item, 'expanded', true);
            }
          });
          this.originalFileData = [...res.records];
          this.cdr.detectChanges();
        });
    }
  }

  getCopySourceTree(node, startPage?) {
    if (!node.expanded || !!size(node.children)) {
      return;
    }
    this.getCopySourceNode(node, startPage);
  }

  getCopySourceNode(node, startPage?) {
    this.copyControllerService
      .ListCopyCatalogs({
        memberEsn: this.rowCopy?.device_esn || '',
        pageNo: startPage || CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE * 10,
        copyId: this.rowCopy.uuid,
        parentPath: node.rootPath || '/'
      })
      .subscribe(res => {
        if (
          includes(
            [
              DataMap.Resource_Type.DWS_Cluster.value,
              DataMap.Resource_Type.DWS_Database.value,
              DataMap.Resource_Type.DWS_Schema.value,
              DataMap.Resource_Type.DWS_Table.value
            ],
            this.childResType
          ) &&
          !!size(res.records) &&
          node.type === RestoreFileType.Directory
        ) {
          node.disabled = false;
        }
        this.updataChildren(res, node, true);
        this.originalFileData = [...this.originalFileData];
        this.cdr.detectChanges();
      });
  }

  getTargetTree() {
    if (
      includes(
        [
          DataMap.Resource_Type.DWS_Cluster.value,
          DataMap.Resource_Type.DWS_Schema.value
        ],
        this.rowCopy.resource_sub_type
      ) &&
      (this.targetParams?.restore_location === RestoreLocationType.NEW ||
        this.targetParams?.restoreLocation === RestoreV2LocationType.NEW)
    ) {
      this.mountedSelection = [this.targetParams.resource];
      this.inputTarget = this.targetParams.resource.name;
      this.selectFileData = [];
      each(this.originalSelection, item => {
        if (!item.isLeaf || item?.isMoreBtn) {
          return;
        }

        assign(item, {
          isLeaf: true,
          newName: '',
          invalid: false,
          errorTips: '',
          type: RestoreFileType.Directory
        });

        this.selectFileData.push(item);
      });
      // dws集群和schema如果新位置恢复，需要去检测填写的schema是不是重名且不允许恢复
      this.dwsNotAllowedSchemaOption = [];
      this.getNewLocationSchema();
      this.disabledOkbtn();
      this.cdr.detectChanges();
      return;
    }

    if (
      includes(
        [DataMap.Resource_Type.DWS_Table.value],
        this.rowCopy.resource_sub_type
      )
    ) {
      this.mountedSelection = null;
      this.inputTarget = this.targetParams.resource.name;
      this.getDatabase(this.targetParams.resource.value);
      this.disabledOkbtn();
      return;
    }

    if (
      includes(
        [
          DataMap.Resource_Type.SQLServerClusterInstance.value,
          DataMap.Resource_Type.SQLServerInstance.value,
          DataMap.Resource_Type.SQLServerGroup.value
        ],
        this.rowCopy.resource_sub_type
      )
    ) {
      this.mountedSelection = null;
      this.inputTarget = this.targetParams.resource.name;
      this.getInstance(this.targetParams.resource.value);
      this.disabledOkbtn();
      return;
    }

    if (
      this.targetParams.restore_location === RestoreLocationType.ORIGIN ||
      this.targetParams.restoreLocation === RestoreV2LocationType.ORIGIN
    ) {
      this.inputTarget = this.rowCopy.resource_name;
      this.selectFileData = [
        {
          children: null,
          hasChildren: false,
          name: this.rowCopy.resource_name,
          label: this.rowCopy.resource_name,
          parent_uuid: null,
          path: '/',
          rootPath: '/',
          icon: includes([DataMap.Resource_Type.ndmp.value], this.childResType)
            ? 'aui-icon-nas-file'
            : 'aui-icon-cluster'
        }
      ];
    } else {
      this.inputTarget =
        this.childResType === DataMap.Resource_Type.HBaseBackupSet.value
          ? this.targetParams.resource?.label
          : this.targetParams.resource?.name;
      this.selectFileData = [
        {
          children: includes(
            [
              DataMap.Resource_Type.HDFSFileset.value,
              DataMap.Resource_Type.HBaseBackupSet.value,
              DataMap.Resource_Type.ClickHouse.value,
              DataMap.Resource_Type.HiveBackupSet.value,
              DataMap.Resource_Type.fileset.value,
              DataMap.Resource_Type.volume.value,
              DataMap.Resource_Type.tidbCluster.value,
              DataMap.Resource_Type.tidbDatabase.value,
              DataMap.Resource_Type.ObjectSet.value,
              DataMap.Resource_Type.ndmp.value
            ],
            this.childResType
          )
            ? null
            : [],
          hasChildren: !includes(
            [
              DataMap.Resource_Type.HDFSFileset.value,
              DataMap.Resource_Type.HBaseBackupSet.value,
              DataMap.Resource_Type.ClickHouse.value,
              DataMap.Resource_Type.HiveBackupSet.value,
              DataMap.Resource_Type.fileset.value,
              DataMap.Resource_Type.volume.value,
              DataMap.Resource_Type.tidbCluster.value,
              DataMap.Resource_Type.tidbDatabase.value,
              DataMap.Resource_Type.ObjectSet.value,
              DataMap.Resource_Type.ndmp.value
            ],
            this.childResType
          ),
          isLeaf: !![DataMap.Resource_Type.ClickHouse.value].includes(
            this.childResType
          ),
          name: this.targetParams.resource?.name,
          label: this.targetParams.resource?.label,
          parent_uuid: null,
          path:
            this.childResType === DataMap.Resource_Type.LocalFileSystem.value &&
            this.targetParams.resource?.path
              ? this.targetParams.resource?.path
              : '/',
          rootPath:
            this.childResType === DataMap.Resource_Type.LocalFileSystem.value &&
            this.targetParams.resource?.path
              ? this.targetParams.resource?.path
              : '/',
          icon: includes(
            [
              DataMap.Resource_Type.HDFSFileset.value,
              DataMap.Resource_Type.HBaseBackupSet.value,
              DataMap.Resource_Type.HiveBackupSet.value,
              DataMap.Resource_Type.tidbCluster.value,
              DataMap.Resource_Type.tidbDatabase.value
            ],
            this.childResType
          )
            ? 'aui-icon-cluster'
            : 'aui-icon-nas-file'
        }
      ];
    }

    if (
      this.rowCopy.resource_sub_type ===
        DataMap.Resource_Type.tidbDatabase.value &&
      this.targetParams.restoreLocation === RestoreV2LocationType.ORIGIN
    ) {
      this.inputTarget = this.rowCopy.resource_environment_name;
      this.selectFileData = [
        {
          children: null,
          hasChildren: false,
          name: this.rowCopy.resource_environment_name,
          label: this.rowCopy.resource_environment_name,
          parent_uuid: null,
          path: '/',
          rootPath: '/',
          icon: 'aui-icon-nas-file'
        }
      ];
    }
    this.mountedSelection = this.selectFileData[0];
    this.disabledOkbtn();
    this.cdr.detectChanges();
  }

  validNewName(item) {
    const value = item.newName;

    if (!trim(value)) {
      item.invalid = false;
      this.disabledOkbtn();
      return;
    }

    const resProperties = JSON.parse(this.rowCopy.resource_properties);
    let tmpParentName =
      this.childResType === DataMap.Resource_Type.DWS_Schema.value
        ? resProperties.parent_name
        : find(this.databaseOptions, { value: this.database }).label;

    if (
      this.dwsNotAllowedSchemaOption.some(
        val => val === `${tmpParentName}/${item.newName}`
      )
    ) {
      item.invalid = true;
      item.errorTips = this.newNameErrorTip.invalidNotAllowedName;
      this.disabledOkbtn();
      return;
    }

    const reg = /^[a-zA-Z\_]{1}[a-zA-Z0-9\_\$\#]{0,62}$/;

    if (startsWith(trim(value), 'PG_')) {
      item.invalid = true;
      item.errorTips = this.newNameErrorTip.invalidSpecialName;
    } else if (!reg.test(value)) {
      item.invalid = true;
      item.errorTips = this.newNameErrorTip.invalidName;
    } else {
      item.invalid = false;
    }
    this.disabledOkbtn();
  }

  getDatabase(uuid, recordsTemp?: any[], startPage?: number) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        subType: DataMap.Resource_Type.DWS_Database.value,
        parentUuid: [['=='], uuid]
      })
    };
    this.protectedResourceApiService.ListResources(params).subscribe(res => {
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
        this.selectFileData = map(recordsTemp, item => {
          return {
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: item.name || '',
            isLeaf: true
          };
        });

        const resourceData = isString(this.rowCopy.resource_properties)
          ? this.rowCopyResPro
          : {};
        if (
          resourceData?.ext_parameters?.backup_tool_type ===
          DataMap.Backup_Type.Roach.value
        ) {
          const originalDatabase = filter(this.selectFileData, item => {
            return item.name === resourceData?.parent_name;
          });
          this.selectFileData = !!size(originalDatabase)
            ? originalDatabase
            : this.selectionData;
        }
        this.cdr.detectChanges();
        return;
      }
      this.getDatabase(uuid, recordsTemp, startPage);
    });
  }

  getInstance(uuid, recordsTemp?: any[], startPage?: number) {
    const conditions = {
      subType: [
        DataMap.Resource_Type.SQLServerInstance.value,
        DataMap.Resource_Type.SQLServerClusterInstance.value
      ],
      environment: {
        uuid: uuid
      }
    };

    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify(conditions)
    };

    this.protectedResourceApiService.ListResources(params).subscribe(res => {
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
        if (res.totalCount === 0) {
          this.getClusterInstance();
          return;
        }
        this.selectFileData = map(recordsTemp, item => {
          return {
            ...item,
            key: item.uuid,
            value: item.uuid,
            name: `${item.name}(${
              item.subType ===
              DataMap.Resource_Type.SQLServerClusterInstance.value
                ? item.path
                : item.environment?.endpoint
            })`,
            isLeaf: true
          };
        });
        this.cdr.detectChanges();
        return;
      }
      this.getInstance(uuid, recordsTemp, startPage);
    });
  }

  getClusterInstance(recordsTemp?: any[], startPage?: number) {
    const conditions = {
      subType: [
        DataMap.Resource_Type.SQLServerInstance.value,
        DataMap.Resource_Type.SQLServerClusterInstance.value
      ],
      path: [['=='], this.targetParams.resource.endpoint]
    };

    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify(conditions)
    };

    this.protectedResourceApiService.ListResources(params).subscribe(res => {
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
        this.selectFileData = map(recordsTemp, item => {
          return {
            ...item,
            key: item.uuid,
            value: item.uuid,
            name: `${item.name}(${
              item.subType ===
              DataMap.Resource_Type.SQLServerClusterInstance.value
                ? item.path
                : item.environment?.endpoint
            })`,
            isLeaf: true
          };
        });
        this.cdr.detectChanges();
        return;
      }
      this.getClusterInstance(recordsTemp, startPage);
    });
  }

  updataChildren(res, node, isCopySource?) {
    each(res.records, item => {
      assign(item, {
        name:
          DataMap.Resource_Type.ObjectSet.value === this.childResType &&
          item.type === this.restoreFileType.File
            ? item.extendInfo.split(',').slice(-1)
            : item.path,
        rootPath:
          node.rootPath === '/'
            ? `/${item.path}`
            : `${node.rootPath}/${item.path}`,
        isLeaf:
          item.type === RestoreFileType.Link ||
          item.type === RestoreFileType.File ||
          item.type === 'table' ||
          item.type === DataMap.Resource_Type.SQLServerDatabase.value,
        children:
          item.type === RestoreFileType.Directory ||
          includes(
            [
              'cluster',
              'database',
              'schema',
              DataMap.Resource_Type.SQLServerInstance.value,
              DataMap.Resource_Type.SQLServerClusterInstance.value,
              DataMap.Resource_Type.SQLServerGroup.value
            ],
            item['type']
          )
            ? []
            : null,
        disabled:
          includes(
            [
              'cluster',
              'database',
              'schema',
              DataMap.Resource_Type.SQLServerInstance.value,
              DataMap.Resource_Type.SQLServerClusterInstance.value,
              DataMap.Resource_Type.SQLServerGroup.value
            ],
            item['type']
          ) && this.restoreLevel !== 'schema',
        icon:
          item.type === RestoreFileType.Directory
            ? this.rowCopy.resource_sub_type ===
              DataMap.Resource_Type.HBaseBackupSet.value
              ? 'aui-icon-directory-namespace'
              : this.rowCopy.resource_sub_type ===
                  DataMap.Resource_Type.volume.value &&
                node.name === this.rowCopyResPro['name']
              ? 'aui-icon-volume-small'
              : 'aui-icon-directory'
            : item.type === 'schema'
            ? 'aui-icon-dws-schema'
            : 'aui-icon-file'
      });

      const format = get(JSON.parse(this.rowCopy.properties), 'format');
      if (
        format === 1 &&
        includes(
          [
            DataMap.Resource_Type.NASShare.value,
            DataMap.Resource_Type.fileset.value,
            DataMap.Resource_Type.volume.value,
            DataMap.Resource_Type.ObjectSet.value
          ],
          this.rowCopy.resource_sub_type
        ) &&
        (this.rowCopy.resource_sub_type ===
          DataMap.Resource_Type.NASShare.value ||
          item.modifyTime !== '0')
      ) {
        item.modifyTime =
          this.datePipe.transform(
            toNumber(item.modifyTime) * 1000,
            'yyyy-MM-dd HH:mm:ss',
            this.timeZone
          ) || item.modifyTime;
      }
    });

    if (isArray(node.children) && !isEmpty(node.children)) {
      node.children = [
        ...reject(node.children, n => {
          return n.isMoreBtn;
        }),
        ...res.records
      ];
    } else {
      node.children.push(...res.records);
    }
    if (res.totalCount > size(node.children)) {
      const moreClickNode = {
        name: `${this.i18n.get('common_more_label')}...`,
        isMoreBtn: true,
        hasChildren: false,
        isLeaf: true,
        children: null,
        parent: node,
        startPage: Math.floor(size(node.children) / 200)
      };
      node.children = [...node.children, moreClickNode];
    }
    if (find(this.originalSelection, node) && isCopySource) {
      this.originalSelection = [...this.originalSelection, ...res.records];
    }

    if (
      this.rowCopy.resource_sub_type === DataMap.Resource_Type.DWS_Cluster.value
    ) {
      this.originalSelection = unionBy(this.originalSelection, 'rootPath');
      const schemaIndex = findIndex(
        this.originalSelection,
        child => get(child, 'rootPath') === node.rootPath
      );
      if (schemaIndex !== -1) {
        this.originalSelection[schemaIndex] = node;
      }
      each(this.originalSelection, item => {
        const table = find(
          node.children,
          child => child.rootPath === item.rootPath
        );

        if (!!table) {
          const index = findIndex(
            this.originalSelection,
            child => get(child, 'rootPath') === item.rootPath
          );
          this.originalSelection[index] = table;
        }
      });
      this.selectionChange();
    }
  }

  getExpandedTreeData(node, startPage?, isAddInput?) {
    if (!node.expanded || !!size(node.children)) {
      return;
    }
    this.getNodeData(node, startPage, isAddInput);
  }

  getNodeData(node?, startPage?, isAddInput?) {
    const postParams = this.targetParams.mountRequestParams;
    if (
      includes(
        [
          DataMap.Resource_Type.NASShare.value,
          DataMap.Resource_Type.NASFileSystem.value
        ],
        this.childResType
      ) &&
      postParams &&
      postParams.location === 'NEW' &&
      postParams.CreateMountRequestBody?.sharedProtocol === 'CIFS'
    ) {
      const params = {
        akOperationTips: false,
        listRestoreTargetFilesRequestBody: {
          cifsAuth: postParams.CreateMountRequestBody?.cifsAuth,
          filesystemName: postParams.CreateMountRequestBody?.filesystemName,
          name: postParams.CreateMountRequestBody?.sharedName,
          protocol: 'CIFS',
          shareIp: postParams.CreateMountRequestBody?.shareIp || '192.168.1.1',
          targetType: postParams.targetType || 'HOMOGENEOUS',
          location: 'NEW'
        },
        path: node && node.rootPath ? node.rootPath : '/',
        pageNo: startPage || CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE * 10,
        envId: postParams.envId
      };
      this.apiStorageBackupPluginService
        .ListRestoreTargetFiles(params)
        .subscribe(res => {
          this.updataChildren(res, node);
          if (isAddInput) {
            node.children = [this.createTreeInput(), ...node.children];
          }
          this.selectFileData = [...this.selectFileData];
          this.cdr.detectChanges();
        });
    } else {
      const params = {
        akOperationTips: false,
        mountPointId: this.fileSystemMountId,
        pageNo: startPage || CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE * 10,
        parentPath: node && node.rootPath ? node.rootPath : '/'
      };
      this.apiStorageBackupPluginService
        .ListRestoreFiles(params)
        .subscribe(res => {
          this.updataChildren(res, node);
          if (isAddInput) {
            node.children = [this.createTreeInput(), ...node.children];
          }
          this.selectFileData = [...this.selectFileData];
          this.cdr.detectChanges();
        });
    }
  }

  excludeParentFileSelection(node) {
    if (node.parent) {
      this.originalSelection = reject(this.originalSelection, item => {
        return item.rootPath === node.parent.rootPath;
      });
      if (find(node.parent?.children, { isMoreBtn: true })) {
        this.originalSelection = reject(this.originalSelection, item => {
          return item.isMoreBtn;
        });
      }
      this.excludeParentFileSelection(node.parent);
    }
  }

  includeChildrenFileSelection(node) {
    if (!isEmpty(node.children)) {
      if (
        find(this.originalSelection, item => {
          return item.rootPath === node.rootPath;
        })
      ) {
        this.originalSelection = unionBy(
          this.originalSelection,
          node.children,
          'rootPath'
        );
      } else {
        this.originalSelection = reject(this.originalSelection, item => {
          return includes(map(node.children, 'rootPath'), item.rootPath);
        });
      }
      each(node.children, item => {
        this.includeChildrenFileSelection(item);
      });
    }
  }

  lvCheck(node) {
    // 取消父
    this.excludeParentFileSelection(node);
    // 关联子
    this.includeChildrenFileSelection(node);
    this.selectedLength = this.getPath(
      cloneDeep(this.originalSelection)
    ).length;
    if (
      includes(
        [
          DataMap.Resource_Type.DWS_Cluster.value,
          DataMap.Resource_Type.DWS_Schema.value
        ],
        this.rowCopy.resource_sub_type
      )
    ) {
      this.selectedLength = size(
        filter(this.originalSelection, item => item?.isLeaf)
      );
    }
    this.disabledOkbtn();
  }

  rowClick(item) {
    if (item.isCreate || item.isMoreBtn || item.type === RestoreFileType.File) {
      return;
    }
    this.mountedSelection = item;

    if (
      includes(
        [
          DataMap.Resource_Type.DWS_Cluster.value,
          DataMap.Resource_Type.DWS_Schema.value
        ],
        this.childResType
      )
    ) {
      return;
    }

    if (this.childResType === DataMap.Resource_Type.DWS_Table.value) {
      this.inputTarget = `${this.targetParams.resource.name}/${item.name}`;
    } else {
      this.inputTarget =
        this.childResType === DataMap.Resource_Type.HBaseBackupSet.value
          ? item.label
          : item.name;
    }
    this.disabledOkbtn();
  }

  rowChecked(item) {
    if (this.childResType === DataMap.Resource_Type.DWS_Table.value) {
      return item.name === split(this.inputTarget, '/')[1];
    } else if (
      includes(
        [
          DataMap.Resource_Type.DWS_Cluster.value,
          DataMap.Resource_Type.DWS_Schema.value
        ],
        this.childResType
      )
    ) {
      return false;
    } else if (
      includes(
        [
          DataMap.Resource_Type.SQLServerClusterInstance.value,
          DataMap.Resource_Type.SQLServerInstance.value,
          DataMap.Resource_Type.SQLServerGroup.value
        ],
        this.rowCopy.resource_sub_type
      )
    ) {
      return item.uuid === get(this.mountedSelection, 'uuid');
    } else {
      return (
        this.mountedSelection &&
        this.mountedSelection['rootPath'] === item.rootPath
      );
    }
  }

  selectionChange() {
    this.selectedLength = this.getPath(
      cloneDeep(this.originalSelection)
    ).length;
    if (
      includes(
        [
          DataMap.Resource_Type.DWS_Cluster.value,
          DataMap.Resource_Type.DWS_Schema.value
        ],
        this.rowCopy.resource_sub_type
      )
    ) {
      this.selectedLength = size(
        filter(this.originalSelection, item => item?.isLeaf)
      );
    }

    this.disabledOkbtn();

    if (
      includes(
        [DataMap.Resource_Type.DWS_Database.value],
        this.rowCopy.resource_sub_type
      ) &&
      this.restoreLevel === 'schema' &&
      !!size(this.originalSelection)
    ) {
      this.originalSelection = [last(this.originalSelection)];
    }

    if (
      includes(
        [
          DataMap.Resource_Type.DWS_Cluster.value,
          DataMap.Resource_Type.DWS_Schema.value
        ],
        this.rowCopy.resource_sub_type
      ) &&
      (this.targetParams?.restore_location === RestoreLocationType.NEW ||
        this.targetParams?.restoreLocation === RestoreV2LocationType.NEW)
    ) {
      this.selectFileData = [];
      each(this.originalSelection, item => {
        if (!item.isLeaf || item?.isMoreBtn) {
          return;
        }
        assign(item, {
          isLeaf: true,
          newName: '',
          errorTips: '',
          invalid: false,
          type: RestoreFileType.Directory
        });
        this.selectFileData.push(item);
      });
    }
  }

  getRestoreComponent() {
    let recoveryComponent: any;
    switch (this.childResType) {
      case DataMap.Resource_Type.NASShare.value:
      case DataMap.Resource_Type.NASFileSystem.value:
      case DataMap.Resource_Type.ndmp.value:
        recoveryComponent = DoradoNasRestoreComponent;
        break;
      case DataMap.Resource_Type.LocalFileSystem.value:
        recoveryComponent = LocalFileSystemRestoreComponent;
        break;
      case DataMap.Resource_Type.HDFSFileset.value:
        recoveryComponent = HdfsFilesetRestoreComponent;
        break;
      case DataMap.Resource_Type.HBaseBackupSet.value:
        recoveryComponent = BackupSetRestoreComponent;
        break;
      case DataMap.Resource_Type.HiveBackupSet.value:
        recoveryComponent = HiveBackupsetRestoreComponent;
        break;
      case DataMap.Resource_Type.DWS_Cluster.value:
        recoveryComponent = DWSClusterRestoreComponent;
        break;
      case DataMap.Resource_Type.DWS_Database.value:
        recoveryComponent = DWSDatabaseRestoreComponent;
        break;
      case DataMap.Resource_Type.ElasticsearchBackupSet.value:
        recoveryComponent = ElasticsearchRestoreComponent;
        break;
      case DataMap.Resource_Type.fileset.value:
      case DataMap.Resource_Type.volume.value:
        recoveryComponent = FilesetRestoreComponent;
        break;
      case DataMap.Resource_Type.DWS_Schema.value:
        recoveryComponent = DWSTableRestoreComponent;
        break;
      case DataMap.Resource_Type.DWS_Table.value:
        recoveryComponent = DWSTableRestoreComponent;
        break;
      case DataMap.Resource_Type.ClickHouse.value:
        recoveryComponent = ClickHouseRestoreComponent;
        break;
      case DataMap.Resource_Type.SQLServerInstance.value:
      case DataMap.Resource_Type.SQLServerClusterInstance.value:
        recoveryComponent = SQLServerInstanceRestoreComponent;
        break;
      case DataMap.Resource_Type.SQLServerDatabase.value:
        recoveryComponent = SQLServerDatabaseRestoreComponent;
        break;
      case DataMap.Resource_Type.SQLServerGroup.value:
        recoveryComponent = SQLServerGroupRestoreComponent;
        break;
      case DataMap.Resource_Type.tidbDatabase.value:
      case DataMap.Resource_Type.tidbCluster.value:
        recoveryComponent = TidbRestoreComponent;
        break;
      case DataMap.Resource_Type.OceanBaseCluster.value:
        recoveryComponent = OceanBaseRestoreComponent;
        break;
      case DataMap.Resource_Type.ObjectSet.value:
        recoveryComponent = ObjectRestoreComponent;
        break;
      default:
        recoveryComponent = LocalFileSystemRestoreComponent;
    }
    return recoveryComponent;
  }

  selectRecoveryTarget() {
    const recoveryComponent = this.getRestoreComponent();
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvWidth:
        this.childResType === DataMap.Resource_Type.volume.value
          ? MODAL_COMMON.xLargeWidth
          : MODAL_COMMON.normalWidth + 100,
      lvHeader: this.i18n.get('protection_select_restore_target_label'),
      lvOkDisabled: false,
      lvContent: recoveryComponent,
      lvAfterOpen: modal => {
        const content = modal.getContentComponent();
        const modalIns = modal.getInstance();
        if (this.childResType === DataMap.Resource_Type.HDFSFileset.value) {
          const combinedValid: any = combineLatest(
            content.formGroup.statusChanges,
            content.fileValid$
          );
          combinedValid.subscribe(res => {
            const [formGroupStatus, fileValid] = res;
            modalIns.lvOkDisabled = !(formGroupStatus === 'VALID' && fileValid);
          });
          if (
            content.targetParams &&
            content.targetParams.restoreTo === RestoreV2LocationType.NEW
          ) {
            content.formGroup.updateValueAndValidity();
            content.fileValid$.next(false);
          }
        } else {
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res !== 'VALID';
          });
        }
      },
      lvComponentParams: {
        rowCopy: this.rowCopy,
        restoreType: this.restoreType,
        targetParams: this.targetParams
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent();
          this.targetParams = content.getTargetParams();
          this.version = content?.version;
          if (
            includes(
              [
                DataMap.Resource_Type.HDFSFileset.value,
                DataMap.Resource_Type.HBaseBackupSet.value,
                DataMap.Resource_Type.HiveBackupSet.value,
                DataMap.Resource_Type.ElasticsearchBackupSet.value,
                DataMap.Resource_Type.DWS_Database.value,
                DataMap.Resource_Type.DWS_Cluster.value,
                DataMap.Resource_Type.DWS_Schema.value,
                DataMap.Resource_Type.DWS_Table.value,
                DataMap.Resource_Type.ClickHouse.value,
                DataMap.Resource_Type.fileset.value,
                DataMap.Resource_Type.volume.value,
                DataMap.Resource_Type.SQLServerClusterInstance.value,
                DataMap.Resource_Type.SQLServerInstance.value,
                DataMap.Resource_Type.SQLServerGroup.value,
                DataMap.Resource_Type.tidbDatabase.value,
                DataMap.Resource_Type.tidbCluster.value,
                DataMap.Resource_Type.OceanBaseCluster.value,
                DataMap.Resource_Type.ObjectSet.value
              ],
              this.childResType
            )
          ) {
            resolve(true);
            this.getTargetTree();
          }
          if (this.childResType === DataMap.Resource_Type.ndmp.value) {
            resolve(true);
            this.getTargetTree();
            return;
          }
          if (
            this.targetParams.restore_location !== RestoreLocationType.ORIGIN &&
            this.targetParams.restoreLocation !== RestoreV2LocationType.ORIGIN
          ) {
            this.fileSystemMount(resolve);
          } else {
            resolve(true);
            this.getTargetTree();
          }
        });
      }
    });
  }

  fileSystemMount(resolve) {
    const postParams = this.targetParams.mountRequestParams;
    if (
      includes(
        [
          DataMap.Resource_Type.NASShare.value,
          DataMap.Resource_Type.NASFileSystem.value
        ],
        this.childResType
      ) &&
      postParams.location === 'NEW' &&
      postParams.CreateMountRequestBody?.sharedProtocol === 'CIFS'
    ) {
      resolve(true);
      this.getTargetTree();
    } else {
      this.apiStorageBackupPluginService.CreateMount(postParams).subscribe({
        next: res => {
          resolve(true);
          this.fileSystemMountId = res.mountPointId;
          this.cdr.detectChanges();
          this.getTargetTree();
        },
        error: err => resolve(false)
      });
    }
  }

  createTreeInput() {
    return {
      id: uniqueId(),
      children: [],
      hasChildren: true,
      expanded: true,
      isCreate: true,
      name: '',
      parent_uuid: null,
      parent: this.mountedSelection,
      path: '',
      icon: 'aui-icon-directory'
    };
  }

  createFolder() {
    if (this.mountedSelection) {
      if (!this.mountedSelection.expanded) {
        this.mountedSelection.expanded = true;
        if (!!size(this.mountedSelection.children)) {
          this.mountedSelection.children.unshift(this.createTreeInput());
        } else {
          this.getExpandedTreeData(
            this.mountedSelection,
            CommonConsts.PAGE_START,
            true
          );
        }
      } else {
        this.mountedSelection.children.unshift(this.createTreeInput());
      }
    }
  }

  saveCreateFolder(item) {
    this.apiStorageBackupPluginService
      .CreateFileSystemFolder({
        parentPath: item.parent.rootPath,
        mountPointId: this.fileSystemMountId,
        folderName: item.inputName
      })
      .subscribe(res => {
        item.parent.children = [];
        this.getNodeData(item.parent);
        this.cdr.detectChanges();
      });
  }

  cancleCreateFolder(item) {
    item.parent.children = reject(item.parent.children, v => {
      return v.id && v.id === item.id;
    });
  }

  deleteFolder() {
    this.infoMessageService.create({
      content: this.i18n.get('protection_delete_folder_tip_label', [
        this.mountedSelection.name
      ]),
      onOK: () => {
        this.apiStorageBackupPluginService
          .DeleteFileSystemFolder({
            mountPointId: this.fileSystemMountId,
            filePath: this.mountedSelection.rootPath
          })
          .subscribe(res => {
            this.mountedSelection.parent.children = [];
            this.getNodeData(this.mountedSelection.parent);
            this.mountedSelection = null;
            this.inputTarget = '';
            this.disabledOkbtn();
            this.cdr.detectChanges();
          });
      }
    });
  }

  disabledOkbtn(data?) {
    if (this.childResType === DataMap.Resource_Type.Dameng_singleNode.value) {
      this.modal.getInstance().lvOkDisabled = !size(this.selectionData);
    } else if (
      includes(
        [
          DataMap.Resource_Type.DWS_Cluster.value,
          DataMap.Resource_Type.DWS_Schema.value
        ],
        this.rowCopy.resource_sub_type
      ) &&
      (this.targetParams?.restore_location === RestoreLocationType.NEW ||
        this.targetParams?.restoreLocation === RestoreV2LocationType.NEW)
    ) {
      this.modal.getInstance().lvOkDisabled =
        (this.rowCopy.isSearchRestore
          ? !this.mountedSelection
          : !size(this.originalSelection) || !this.mountedSelection) ||
        !!find(this.selectFileData, item => item.invalid);
    } else if (
      includes(
        [DataMap.Resource_Type.DWS_Database.value],
        this.rowCopy.resource_sub_type
      ) &&
      this.restoreLevel === 'schema'
    ) {
      this.modal.getInstance().lvOkDisabled = this.rowCopy.isSearchRestore
        ? !this.mountedSelection
        : !size(this.originalSelection) ||
          !this.mountedSelection ||
          size(this.originalSelection) > 1;
    } else if (
      includes(
        [DataMap.Resource_Type.OceanBaseCluster.value],
        this.rowCopy.resource_sub_type
      )
    ) {
      const selection = filter(
        this.originalSelection,
        item => item.isLeaf && !item.isMoreBtn
      );
      this.modal.getInstance().lvOkDisabled =
        !size(selection) ||
        isEmpty(this.cluster) ||
        isEmpty(this.resourcePool) ||
        this.validTenantName(this.targetName) ||
        size(selection) > 256;
      if (size(selection) > 256) {
        this.messageService.error(
          this.i18n.get('protection_table_level_restore_max_table_tips_label', [
            256
          ]),
          {
            lvShowCloseButton: true,
            lvMessageKey: 'oceanBaseMaxTableKey'
          }
        );
      }
      if (data) {
        this.showTenantError = this.validTenantName(this.targetName);
      }
    } else if (
      includes(
        [
          DataMap.Resource_Type.tidbCluster.value,
          DataMap.Resource_Type.tidbDatabase.value
        ],
        this.rowCopy.resource_sub_type
      )
    ) {
      this.modal.getInstance().lvOkDisabled = this.rowCopy.isSearchRestore
        ? !this.mountedSelection
        : !size(this.originalSelection) ||
          size(this.originalSelection) > 256 ||
          !this.mountedSelection;
    } else {
      this.modal.getInstance().lvOkDisabled = this.rowCopy.isSearchRestore
        ? !this.mountedSelection
        : this.originalSelectionInvalid() || !this.mountedSelection;
    }
  }

  originalSelectionInvalid() {
    return this.isFileSystemApp && this.pathMode === this.modeMap.fromTag
      ? !size(this.manualInputPath)
      : !size(this.originalSelection);
  }

  getOriginalSelection() {
    return this.isFileSystemApp && this.pathMode === this.modeMap.fromTag
      ? this.manualInputPath
      : this.getPath(cloneDeep(this.originalSelection));
  }

  pathModeChange() {
    this.manualInputPath = [];
    this.disabledOkbtn();
  }

  validTenantName(name: string) {
    const regex = /^[a-zA-Z_][a-zA-Z0-9_]{0,127}$/;
    return !regex.test(name) || isEmpty(name);
  }

  getPath(paths) {
    let filterPaths = [];
    let childPaths = [];
    paths = filter(paths, item => {
      return !isEmpty(item.rootPath);
    });
    each(paths, item => {
      if (!!size(item.children)) {
        childPaths = unionBy(childPaths, item.children, 'rootPath');
      }
    });
    filterPaths = reject(paths, item => {
      return !isEmpty(find(childPaths, { rootPath: item.rootPath }));
    });
    if (
      includes(
        [
          DataMap.Resource_Type.HiveBackupSet.value,
          DataMap.Resource_Type.ClickHouse.value,
          DataMap.Resource_Type.ElasticsearchBackupSet.value,
          DataMap.Resource_Type.SQLServerInstance.value,
          DataMap.Resource_Type.SQLServerClusterInstance.value,
          DataMap.Resource_Type.SQLServerGroup.value
        ],
        this.childResType
      )
    ) {
      return map(filterPaths, item => {
        return item.path;
      });
    } else {
      return map(filterPaths, item => {
        if (
          includes(
            [
              DataMap.Resource_Type.NASShare.value,
              DataMap.Resource_Type.NASFileSystem.value,
              DataMap.Resource_Type.ndmp.value,
              DataMap.Resource_Type.HDFSFileset.value,
              DataMap.Resource_Type.LocalFileSystem.value,
              DataMap.Resource_Type.fileset.value,
              DataMap.Resource_Type.volume.value,
              DataMap.Resource_Type.ObjectSet.value
            ],
            this.childResType
          ) &&
          item.type === RestoreFileType.Directory
        ) {
          assign(item, {
            rootPath: item.rootPath + '/'
          });
        }
        return item.rootPath;
      });
    }
  }

  pathChange(path) {
    this.manualInputPath = [...path];
    this.disabledOkbtn();
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      let tables = cloneDeep(this.originalSelection);
      tables = filter(tables, item => {
        return size(split(item.rootPath, '/')) > 2;
      });
      if (
        DataMap.Resource_Type.HBaseBackupSet.value === this.childResType &&
        this.targetParams?.namespace &&
        size(
          intersectionWith(tables, (a, b) => {
            return a['name'] === b['name'];
          })
        ) !== size(tables) &&
        size(tables) >= 2
      ) {
        this.messageService.error(
          this.i18n.get('common_same_table_to_namespace_label'),
          {
            lvShowCloseButton: true,
            lvMessageKey: 'resSameTableMesageKey'
          }
        );
        observer.error('');
        observer.complete();
        return;
      }
      const params = this.targetParams?.requestParams || {};
      if (
        includes(
          [
            DataMap.Resource_Type.NASShare.value,
            DataMap.Resource_Type.NASFileSystem.value,
            DataMap.Resource_Type.ndmp.value,
            DataMap.Resource_Type.HDFSFileset.value,
            DataMap.Resource_Type.HBaseBackupSet.value,
            DataMap.Resource_Type.HiveBackupSet.value,
            DataMap.Resource_Type.ElasticsearchBackupSet.value,
            DataMap.Resource_Type.DWS_Cluster.value,
            DataMap.Resource_Type.DWS_Database.value,
            DataMap.Resource_Type.DWS_Schema.value,
            DataMap.Resource_Type.DWS_Table.value,
            DataMap.Resource_Type.fileset.value,
            DataMap.Resource_Type.volume.value,
            DataMap.Resource_Type.SQLServerClusterInstance.value,
            DataMap.Resource_Type.SQLServerInstance.value,
            DataMap.Resource_Type.SQLServerGroup.value,
            DataMap.Resource_Type.tidbCluster.value,
            DataMap.Resource_Type.tidbDatabase.value,
            DataMap.Resource_Type.ObjectSet.value
          ],
          this.childResType
        )
      ) {
        if (
          this.targetParams.restoreLocation !== RestoreV2LocationType.ORIGIN
        ) {
          if (
            includes(
              [
                DataMap.Resource_Type.NASShare.value,
                DataMap.Resource_Type.NASFileSystem.value
              ],
              this.childResType
            )
          ) {
            assign(params.extendInfo, {
              targetPath:
                this.mountedSelection.type === RestoreFileType.Directory
                  ? this.mountedSelection.rootPath + '/'
                  : this.mountedSelection.rootPath
            });
          }
        }
        if (
          includes([DataMap.Resource_Type.DWS_Table.value], this.childResType)
        ) {
          const selection = filter(
            this.originalSelection,
            item => item.isLeaf && !item.isMoreBtn
          );
          assign(params, {
            subObjects: this.rowCopy.isSearchRestore
              ? [this.rowCopy.searchRestorePath]
              : map(this.getPath(cloneDeep(selection)), val => {
                  const path = slice(split(val, '/'), 2);
                  return JSON.stringify({
                    name: `${split(this.inputTarget, '/')[1] || path[0]}/${
                      path[1]
                    }/${path[2]}`,
                    type: 'Database',
                    subType:
                      size(path) === 0
                        ? DataMap.Resource_Type.DWS_Cluster.value
                        : size(path) === 1
                        ? DataMap.Resource_Type.DWS_Database.value
                        : size(path) === 2
                        ? DataMap.Resource_Type.DWS_Schema.value
                        : DataMap.Resource_Type.DWS_Table.value,
                    extendInfo: {
                      oldName: join(path, '/')
                    }
                  });
                })
          });
        } else if (
          includes(
            [
              DataMap.Resource_Type.DWS_Cluster.value,
              DataMap.Resource_Type.DWS_Schema.value,
              DataMap.Resource_Type.DWS_Database.value
            ],
            this.childResType
          )
        ) {
          const selection = filter(
            this.originalSelection,
            item => item.isLeaf && !item.isMoreBtn
          );
          assign(params, {
            subObjects: this.rowCopy.isSearchRestore
              ? [this.rowCopy.searchRestorePath]
              : map(this.getPath(cloneDeep(selection)), val => {
                  const path = slice(split(val, '/'), 2);
                  return JSON.stringify({
                    name: join(path, '/'),
                    type: 'Database',
                    subType:
                      size(path) === 0
                        ? DataMap.Resource_Type.DWS_Cluster.value
                        : size(path) === 1
                        ? DataMap.Resource_Type.DWS_Database.value
                        : size(path) === 2
                        ? DataMap.Resource_Type.DWS_Schema.value
                        : DataMap.Resource_Type.DWS_Table.value,
                    extendInfo: {
                      oldName: join(path, '/')
                    }
                  });
                })
          });
        } else {
          assign(params, {
            subObjects: this.rowCopy.isSearchRestore
              ? [this.rowCopy.searchRestorePath]
              : this.getOriginalSelection()
          });
        }

        if (
          includes(
            [
              DataMap.Resource_Type.tidbCluster.value,
              DataMap.Resource_Type.tidbDatabase.value
            ],
            this.childResType
          )
        ) {
          const selection = filter(
            this.originalSelection,
            item => item.isLeaf && !item.isMoreBtn
          );
          assign(params, {
            subObjects: this.rowCopy.isSearchRestore
              ? [this.rowCopy.searchRestorePath]
              : map(this.getPath(cloneDeep(selection)), val => {
                  const path = slice(split(val, '/'), 1);
                  return JSON.stringify({
                    name: join(path, '/'),
                    type: 'Database',
                    subType:
                      size(path) === 3
                        ? DataMap.Resource_Type.tidbTable.value
                        : DataMap.Resource_Type.tidbDatabase.value,
                    extendInfo: {
                      oldName: join(path, '/')
                    }
                  });
                })
          });
        }

        if (
          includes(
            [
              DataMap.Resource_Type.SQLServerClusterInstance.value,
              DataMap.Resource_Type.SQLServerInstance.value,
              DataMap.Resource_Type.SQLServerGroup.value
            ],
            this.childResType
          ) &&
          this.targetParams.restoreLocation === RestoreV2LocationType.NEW
        ) {
          set(params, 'targetObject', this.mountedSelection?.uuid);
        }

        // 多集群场景：NARS文件系统和NARS文件共享下恢复要向所选节点转发
        let memberEsn = '';
        if (
          this.isOceanProtect &&
          includes(
            [
              DataMap.Resource_Type.NASShare.value,
              DataMap.Resource_Type.NASFileSystem.value
            ],
            this.childResType
          )
        ) {
          memberEsn = get(
            this.targetParams,
            'mountRequestParams.memberEsn',
            ''
          );
        }
        if (
          (includes(
            [
              DataMap.Resource_Type.HDFSFileset.value,
              DataMap.Resource_Type.HBaseBackupSet.value
            ],
            this.childResType
          ) &&
            this.targetParams.restoreTo === RestoreV2LocationType.NEW) ||
          (includes(
            [DataMap.Resource_Type.HiveBackupSet.value],
            this.childResType
          ) &&
            this.targetParams.restoreLocation === RestoreV2LocationType.NEW)
        ) {
          this.beforeRestoreShowTips(params, memberEsn, observer);
        } else {
          this.createFileLevelRestoreTask(params, memberEsn, observer);
        }
      }
      if (this.childResType === DataMap.Resource_Type.ClickHouse.value) {
        const resource = this.rowCopyResPro;
        if (this.targetParams.restoreTo === 'new') {
          assign(params, {
            copyId: this.rowCopy.uuid,
            targetEnv: this.targetParams?.targetEnv,
            restoreType: RestoreV2Type.FileRestore,
            targetLocation: RestoreV2LocationType.NEW,
            targetObject: this.targetParams.targetObject,
            subObjects: this.getPath(cloneDeep(this.originalSelection))
          });
        } else {
          assign(params, {
            copyId: this.rowCopy.uuid,
            targetEnv: resource.environment_uuid,
            restoreType: RestoreV2Type.FileRestore,
            targetLocation: RestoreV2LocationType.ORIGIN,
            targetObject: resource.uuid,
            subObjects: this.getPath(cloneDeep(this.originalSelection))
          });
        }
        this.restoreV2Service
          .CreateRestoreTask({ CreateRestoreTaskRequestBody: params })
          .subscribe({
            next: res => {
              observer.next();
              observer.complete();
            },
            error: err => {
              observer.error(err);
              observer.complete();
            }
          });
      }
      if (this.childResType === DataMap.Resource_Type.Dameng_singleNode.value) {
        const resource = this.rowCopyResPro;
        assign(params, {
          copyId: this.rowCopy.uuid,
          targetEnv: resource.environment_uuid,
          restoreType: RestoreV2Type.FileRestore,
          targetLocation: RestoreV2LocationType.ORIGIN,
          targetObject: resource.uuid,
          subObjects: [map(this.selectionData, item => item.path).join(';')]
        });
        this.restoreV2Service
          .CreateRestoreTask({ CreateRestoreTaskRequestBody: params })
          .subscribe({
            next: res => {
              observer.next();
              observer.complete();
            },
            error: err => {
              observer.error(err);
              observer.complete();
            }
          });
      }
      if (this.childResType === DataMap.Resource_Type.OceanBaseCluster.value) {
        const selection = filter(
          this.originalSelection,
          item => item.isLeaf && !item.isMoreBtn
        );
        assign(params, {
          copyId: this.rowCopy.uuid,
          restoreType: RestoreV2Type.FileRestore,
          targetEnv: get(this.cluster, 'uuid'),
          targetLocation: RestoreV2LocationType.NEW,
          targetObject: get(this.cluster, 'uuid'),
          extendInfo: {
            tenantInfos: JSON.stringify([
              {
                originalName: get(this.tenant, 'label'),
                originalId: get(this.tenant, 'value'),
                targetName: trim(this.targetName),
                resourcePoolId: get(this.resourcePool, 'value'),
                resourcePoolName: get(this.resourcePool, 'label')
              }
            ])
          },
          subObjects: map(this.getPath(cloneDeep(selection)), item => {
            const path = slice(split(item, '/'), 2);
            return join(path, '.');
          })
        });
        this.restoreV2Service
          .CreateRestoreTask({ CreateRestoreTaskRequestBody: params })
          .subscribe({
            next: res => {
              observer.next();
              observer.complete();
            },
            error: err => {
              observer.error(err);
              observer.complete();
            }
          });
      }
      if (this.childResType === DataMap.Resource_Type.LocalFileSystem.value) {
        assign(params.target, {
          restore_target: this.mountedSelection.rootPath
        });
        assign(params, {
          restore_objects: this.rowCopy.isSearchRestore
            ? [this.rowCopy.searchRestorePath]
            : this.getPath(cloneDeep(this.originalSelection))
        });
        this.restoreService
          .createRestoreV1RestoresPost({ body: params })
          .subscribe({
            next: res => {
              observer.next();
              observer.complete();
            },
            error: err => {
              observer.error(err);
              observer.complete();
            }
          });
      }
    });
  }

  private beforeRestoreShowTips(
    params,
    memberEsn: string,
    observer: Observer<void>
  ) {
    let nameSpace = this.targetParams?.namespace || '';
    let tips = isEmpty(nameSpace)
      ? this.i18n.get('protection_hbase_restore_no_backup_task_tips_label')
      : this.i18n.get(
          'protection_hbase_restore_target_namespace_no_backup_task_tips_label',
          [nameSpace.startsWith('/') ? nameSpace.substring(1) : nameSpace]
        );
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvModalKey: 'file-level-restore-tips-info',
      ...{
        lvType: 'dialog',
        lvDialogIcon: 'lv-icon-popup-danger-48',
        lvHeader: this.i18n.get(
          'protection_hbase_restore_no_backup_task_header_label'
        ),
        lvContent: tips,
        lvWidth: 500,
        lvOkType: 'primary',
        lvCancelType: 'default',
        lvOkDisabled: false,
        lvFocusButtonId: 'cancel',
        lvCloseButtonDisplay: true,
        lvOk: () => {
          this.createFileLevelRestoreTask(params, memberEsn, observer);
        },
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
  }

  private createFileLevelRestoreTask(
    params,
    memberEsn: string,
    observer: Observer<void>
  ) {
    this.restoreV2Service
      .CreateRestoreTask({
        CreateRestoreTaskRequestBody: params,
        memberEsn: memberEsn
      })
      .subscribe({
        next: res => {
          observer.next();
          observer.complete();
        },
        error: err => {
          observer.error(err);
          observer.complete();
        }
      });
  }

  trackByIndex(index) {
    return index;
  }

  getTargetPath() {
    return {
      tips: this.getfileLevelRestoreTips(),
      targetPath: this.inputTarget
    };
  }
}
