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
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  EventEmitter,
  Input,
  OnDestroy,
  OnInit,
  Output,
  ViewChild
} from '@angular/core';
import { FormBuilder } from '@angular/forms';
import { Router } from '@angular/router';
import { MessageboxService, MessageService } from '@iux/live';
import { RegisterDatabaseComponent as RegisterSaphanaDatabaseComponent } from 'app/business/protection/application/saphana/register-database/register-database.component';
import { RegisterInstanceComponent as RegisterSaphanaInstanceComponent } from 'app/business/protection/application/saphana/register-instance/register-instance.component';
import { BackupSetComponent } from 'app/business/protection/big-data/hbase/backup-set/backup-set.component';
import { ClustersComponent } from 'app/business/protection/big-data/hbase/clusters/clusters.component';
import { FilesetsComponent } from 'app/business/protection/big-data/hdfs/filesets/filesets.component';
import { HuaWeiStackListComponent } from 'app/business/protection/cloud/huawei-stack/stack-list/huawei-stack-list.component';
import { OpenstackListComponent } from 'app/business/protection/cloud/openstack/openstack-list/openstack-list.component';
import { RegisterComponent as RegisterAntDBInstanceComponent } from 'app/business/protection/database/ant-db/register/register.component';
import { ClusterComponent as ClickHouseClusterComponent } from 'app/business/protection/host-app/click-house/cluster/cluster.component';
import { DatabaseComponent as ClickHouseDatabaseComponent } from 'app/business/protection/host-app/click-house/database/database.component';
import { TabelSetComponent as ClickHouseTablesetComonent } from 'app/business/protection/host-app/click-house/tabel-set/tabel-set.component';
import { DamengComponent } from 'app/business/protection/host-app/dameng/dameng.component';
import { DatabaseTemplateComponent } from 'app/business/protection/host-app/database-template/database-template.component';
import { RegisterGroupComponent } from 'app/business/protection/host-app/exchange/availabilty-group/register-group/register-group.component';
import { FilesetComponent } from 'app/business/protection/host-app/fileset/fileset.component';
import { InstanceDatabaseComponent } from 'app/business/protection/host-app/gaussdb-dws/instance-database/instance-database.component';
import { TableTemplateComponent } from 'app/business/protection/host-app/general-database/table-template/table-template.component';
import { HostComponent } from 'app/business/protection/host-app/host/host.component';
import { RegisterClusterComponent as RegisterInformixServiceComponent } from 'app/business/protection/host-app/informix/register-cluster/register-cluster.component';
import { RegisterInstanceComponent as RegisterInformixInstanceComponent } from 'app/business/protection/host-app/informix/register-instance/register-instance.component';
import { KingBaseClusterComponent } from 'app/business/protection/host-app/king-base/cluster/king-base-cluster.component';
import { KingBaseInstanceDatabaseComponent } from 'app/business/protection/host-app/king-base/instance-database/king-base-instance-database.component';
import { RegisterComponent as RegisterLightCloudGuassdbComponent } from 'app/business/protection/host-app/light-cloud-gaussdb/register/register.component';
import { MongodbComponent } from 'app/business/protection/host-app/mongodb/mongodb.component';
import { ClusterComponent as MySQLClusterComponent } from 'app/business/protection/host-app/mysql/cluster/cluster.component';
import { InstanceDatabaseComponent as MySQLListComponent } from 'app/business/protection/host-app/mysql/instance-database/instance-database.component';
import { RegisterClusterComponent as RegisterOceanBaseComponent } from 'app/business/protection/host-app/ocean-base/register-cluster/register-cluster.component';
import { RegisterTenantComponent } from 'app/business/protection/host-app/ocean-base/register-tenant/register-tenant.component';
import { SummaryClusterComponent as OceanBaseClusterComponent } from 'app/business/protection/host-app/ocean-base/summary-cluster/summary-cluster.component';
import { BaseTemplateComponent as OpenGaussComponent } from 'app/business/protection/host-app/opengauss/base-template/base-template.component';
import { ClusterComponent as OpenGaussClusterComponent } from 'app/business/protection/host-app/opengauss/cluster/cluster.component';
import { DatabaseListComponent as OracleDatabaseListComponent } from 'app/business/protection/host-app/oracle/database-list/database-list.component';
import { PostgreClusterComponent } from 'app/business/protection/host-app/postgre-sql/cluster/postgre-cluster.component';
import { PostgreInstanceDatabaseComponent } from 'app/business/protection/host-app/postgre-sql/instance-database/postgre-instance-database.component';
import { RedisShowComponent } from 'app/business/protection/host-app/redis/redis-show/redis-show.component';
import { RegisterDistributedInstanceComponent } from 'app/business/protection/host-app/tdsql/dirstibuted-instance/register-distributed-instance/register-distributed-instance.component';
import { RegisterClusterComponent } from 'app/business/protection/host-app/tdsql/register-cluster/register-cluster.component';
import { RegisterInstanceComponent } from 'app/business/protection/host-app/tdsql/register-instance/register-instance.component';
import { SummaryClusterComponent as TDSQLClusterComponent } from 'app/business/protection/host-app/tdsql/summary-cluster/summary-cluster.component';
import { SummaryComponent as TDSQLInstanceComponent } from 'app/business/protection/host-app/tdsql/summary-instance/summary.component';
import { RegisterClusterComponent as RegisterTidbClusterComponent } from 'app/business/protection/host-app/tidb/register-cluster/register-cluster.component';
import { RegisterDatabaseComponent as RegisterTidbDatabaseComponent } from 'app/business/protection/host-app/tidb/register-database/register-database.component';
import { RegisterTableComponent as RegisterTidbTableComponent } from 'app/business/protection/host-app/tidb/register-table/register-table.component';
import { CreateVolumeComponent } from 'app/business/protection/host-app/volume/create-volume/create-volume.component';
import { DoradoFileSystemComponent } from 'app/business/protection/storage/dorado-file-system/dorado-file-system.component';
import { NasSharedComponent } from 'app/business/protection/storage/nas-shared/nas-shared.component';
import { RegisterNasShareComponent } from 'app/business/protection/storage/nas-shared/register-nas-share/register-nas-share.component';
import { RegisterObjectComponent } from 'app/business/protection/storage/object/object-service/register-object/register-object.component';
import { ObjectStorageComponent } from 'app/business/protection/storage/object/object-storage/object-storage.component';
import { FusionListComponent } from 'app/business/protection/virtualization/fusion-compute/fusion-list/fusion-list.component';
import { KubernetesContainerComponent } from 'app/business/protection/virtualization/kubernetes-container/kubernetes-container.component';
import { BaseTemplateComponent } from 'app/business/protection/virtualization/kubernetes/base-template/base-template.component';
import { ClusterComponent } from 'app/business/protection/virtualization/kubernetes/cluster/cluster.component';
import { BaseTableComponent } from 'app/business/protection/virtualization/virtualization-base/base-table/base-table.component';
import { VmListComponent } from 'app/business/protection/virtualization/vmware/vm-list/vm-list.component';
import {
  ApiExportFilesApiService as ExportFileApiService,
  BaseUtilService,
  ClientManagerApiService,
  CommonConsts,
  ComponentRestApiService,
  CookieService,
  DatabasesService,
  DataMap,
  DataMapService,
  DATE_PICKER_MODE,
  EnvironmentsService,
  ExceptionService,
  extendSlaInfo,
  getLabelList,
  getPermissionMenuItem,
  getTableOptsItems,
  GlobalService,
  HcsResourceServiceService,
  HostService,
  I18NService,
  JobAPIService,
  MODAL_COMMON,
  OperateItems,
  OpHcsServiceApiService,
  ProjectedObjectApiService,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ProtectResourceAction,
  ProtectResourceCategory,
  ResourceService,
  ResourceType,
  SearchRange,
  SearchResource,
  SnmpApiService,
  SwitchService,
  VirtualResourceService,
  WarningMessageService
} from 'app/shared';
import { WarningBatchConfirmsService } from 'app/shared/components/warning-batch-confirm/warning-batch-confirm.component';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { InfoMessageService } from 'app/shared/services/info-message.service';
import { ProtectService } from 'app/shared/services/protect.service';
import { RegisterService } from 'app/shared/services/register.service';
import { RememberColumnsService } from 'app/shared/services/remember-columns.service';
import { ResourceDetailService } from 'app/shared/services/resource-detail.service';
import { SetResourceTagService } from 'app/shared/services/set-resource-tag.service';
import { SlaService } from 'app/shared/services/sla.service';
import { TakeManualBackupService } from 'app/shared/services/take-manual-backup.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  cloneDeep,
  each,
  first,
  get,
  has,
  includes,
  isEmpty,
  isNil,
  map as _map,
  mapValues,
  omit,
  set,
  size,
  trim,
  values
} from 'lodash';
import { Subscription } from 'rxjs';
import { map } from 'rxjs/operators';
import { GetLabelOptionsService } from '../../../shared/services/get-labels.service';
import { SaponoracleRegisterDatabaseComponent } from '../../protection/application/saponoracle/register-database/register-database.component';

@Component({
  selector: 'aui-resource-list',
  templateUrl: './resource-list.component.html',
  styleUrls: ['./resource-list.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class ResourceListComponent implements OnInit, OnDestroy {
  slaName;
  orderBy;
  orderType;
  tableData = [];
  filterParams = {} as any;
  resourceTypes = [];
  dataMap = DataMap;
  hostComponent: HostComponent;
  oracleComponent: OracleDatabaseListComponent;
  vmListComponent: VmListComponent;
  doradoFileSystemComponent: DoradoFileSystemComponent;
  nasSharedComponent: NasSharedComponent;
  hdfsFilesetsComponent: FilesetsComponent;
  hbaseBackupSetComponent: BackupSetComponent;
  kubernetesClusterComponent: ClusterComponent;
  kubernetesComponent: BaseTemplateComponent;
  redisClusterComponent: RedisShowComponent;
  postgreClusterComponent: PostgreClusterComponent;
  postgreInstanceComponent: PostgreInstanceDatabaseComponent;
  kingBaseClusterComponent: KingBaseClusterComponent;
  kingBaseInstanceComponent: KingBaseInstanceDatabaseComponent;
  fcListComponent: FusionListComponent;
  hcsListComponent: HuaWeiStackListComponent;
  filesetListComponent: FilesetComponent;
  mysqlClusterComponent: MySQLClusterComponent;
  tdsqlInstanceComponent: TDSQLInstanceComponent;
  tdsqlClusterComponent: TDSQLClusterComponent;
  oceanbaseClusterComponent: OceanBaseClusterComponent;
  mysqlListComponent: MySQLListComponent;
  clickhouseDatabaseComponent: ClickHouseDatabaseComponent;
  clickHouseTablesetComonent: ClickHouseTablesetComonent;
  clickHouseClusterComonent: ClickHouseClusterComponent;
  instanceDatabaseComponent: InstanceDatabaseComponent;
  openGaussClusterComponent: OpenGaussClusterComponent;
  openGaussComponent: OpenGaussComponent;
  damengComponent: DamengComponent;
  clustersComponent: ClustersComponent;
  openstackListComponent: OpenstackListComponent;
  generalDatabaseComponent: TableTemplateComponent;
  databaseTemplateComponent: DatabaseTemplateComponent;
  mongodbComponent: MongodbComponent;
  kubernetesContainerComponent: KubernetesContainerComponent;
  objectStorageComponent: ObjectStorageComponent;
  baseTableComponent: BaseTableComponent;
  resourceStore$: Subscription = new Subscription();
  resourceRefreshStore$: Subscription = new Subscription();
  pageNo = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE_OPTIONS[1];
  total = CommonConsts.PAGE_TOTAL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  columns = [
    {
      label: this.i18n.get('common_name_label'),
      key: 'name'
    },
    {
      label: this.i18n.get('common_type_label'),
      key: 'sub_type'
    },
    {
      label: this.i18n.get('common_location_label'),
      key: 'path'
    },
    {
      label: this.i18n.get('common_sla_label'),
      key: 'sla_name'
    },
    {
      label: this.i18n.get('common_sla_compliance_label'),
      key: 'sla_compliance'
    },
    {
      label: this.i18n.get('protection_protected_status_label'),
      key: 'protection_status'
    },
    {
      key: 'labelList',
      label: this.i18n.get('common_tag_label'),
      isHidden: includes(
        [
          DataMap.Deploy_Type.cloudbackup.value,
          DataMap.Deploy_Type.cloudbackup2.value
        ],
        this.i18n.get('deploy_type')
      )
    }
  ];

  @Input() resourceTypeValues;
  @Input() searchType;
  @Output() calcTimeChange = new EventEmitter<any>();
  @ViewChild('slaNamePopover', { static: false }) slaNamePopover;

  constructor(
    private fb: FormBuilder,
    public i18n: I18NService,
    private router: Router,
    private message: MessageService,
    private messageBox: MessageboxService,
    private slaService: SlaService,
    private protectService: ProtectService,
    private switchService: SwitchService,
    private detailService: ResourceDetailService,
    private globalService: GlobalService,
    private drawModalService: DrawModalService,
    private dataMapService: DataMapService,
    private warningMessageService: WarningMessageService,
    private projectedObjectApiService: ProjectedObjectApiService,
    private takeManualBackupService: TakeManualBackupService,
    private batchOperateService: BatchOperateService,
    private cookieService: CookieService,
    private hostApiService: HostService,
    private environmentsApiService: EnvironmentsService,
    private databasesService: DatabasesService,
    private virtualResourceService: VirtualResourceService,
    private resourcesService: ResourceService,
    private rememberColumnsService: RememberColumnsService,
    public virtualScroll: VirtualScrollService,
    private cdr: ChangeDetectorRef,
    private snmpApiService: SnmpApiService,
    private infoMessageService: InfoMessageService,
    private componentRestApiService: ComponentRestApiService,
    private warningBatchConfirmsService: WarningBatchConfirmsService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    public baseUtilService: BaseUtilService,
    public messageService: MessageService,
    private exceptionService: ExceptionService,
    private exportFilesApi: ExportFileApiService,
    private clientManagerApiService: ClientManagerApiService,
    private registerService: RegisterService,
    private hcsResourceService: HcsResourceServiceService,
    private opHcsServiceApiService: OpHcsServiceApiService,
    private setResourceTagService: SetResourceTagService,
    private getLabelOptionsService: GetLabelOptionsService,
    private appUtilsService?: AppUtilsService,
    private jobApiService?: JobAPIService
  ) {}

  ngOnDestroy() {
    this.resourceStore$.unsubscribe();
    this.resourceRefreshStore$.unsubscribe();
  }

  ngOnInit() {
    this.getStore();
    this.getComponent();
  }

  getResources() {
    const params = {
      pageNo: this.pageNo,
      pageSize: this.pageSize
    };

    each(this.filterParams, (value, key) => {
      if (isEmpty(trim(value))) {
        delete this.filterParams[key];
      }

      if (key === 'searchKey') {
        if (this.searchType === SearchRange.RESOURCES) {
          this.filterParams['name'] = this.filterParams[key];
        } else {
          this.filterParams['labelName'] = this.filterParams[key];
        }
        delete this.filterParams[key];
      }

      if (
        key === 'sla_compliance' &&
        includes(this.filterParams[key], true) &&
        includes(this.filterParams[key], false) &&
        size(this.filterParams[key]) === 2
      ) {
        delete this.filterParams[key];
      }

      if (
        key === 'sub_type' &&
        size(this.filterParams[key]) === size(this.resourceTypeValues)
      ) {
        delete this.filterParams[key];
      }
    });

    if (!isEmpty(this.filterParams)) {
      assign(params, {
        conditions: JSON.stringify(
          omit(this.filterParams, ['clusterId', 'clusterType'])
        )
      });
    }

    this.resourcesService
      .queryResourcesV1ResourceActionSearchGet({
        ...params,
        ...{
          clustersId: this.filterParams?.clusterId,
          clustersType: this.filterParams?.clusterType
        }
      })
      .pipe(
        map(res => {
          each(res.items, item => {
            assign(item, {
              globalSearch: this,
              subType: item.sub_type,
              resourceType: this.getResourceType(item)
            });
          });
          return res;
        })
      )
      .subscribe(
        res => {
          each(res.items, item => {
            // 获取标签数据
            const { showList, hoverList } = getLabelList(item);
            assign(item, {
              showLabelList: showList,
              hoverLabelList: hoverList
            });
            if (
              includes(
                [
                  DataMap.Resource_Type.HCSCloudHost.value,
                  DataMap.Resource_Type.HCSProject.value,
                  DataMap.Resource_Type.FusionCompute.value,
                  DataMap.Resource_Type.fusionOne.value,
                  DataMap.Resource_Type.KubernetesStatefulset.value,
                  DataMap.Resource_Type.KubernetesNamespace.value,
                  DataMap.Resource_Type.OpenGauss.value,
                  DataMap.Resource_Type.OpenGauss_instance.value,
                  DataMap.Resource_Type.OpenGauss_database.value,
                  DataMap.Resource_Type.MySQLCluster.value,
                  DataMap.Resource_Type.MySQLClusterInstance.value,
                  DataMap.Resource_Type.MySQLInstance.value,
                  DataMap.Resource_Type.MySQLDatabase.value,
                  DataMap.Resource_Type.SQLServerCluster.value,
                  DataMap.Resource_Type.SQLServerClusterInstance.value,
                  DataMap.Resource_Type.SQLServerInstance.value,
                  DataMap.Resource_Type.DWS_Cluster.value,
                  DataMap.Resource_Type.DWS_Database.value,
                  DataMap.Resource_Type.DWS_Schema.value,
                  DataMap.Resource_Type.DWS_Table.value,
                  DataMap.Resource_Type.goldendbCluter.value,
                  DataMap.Resource_Type.goldendbInstance.value,
                  DataMap.Resource_Type.generalDatabase.value,
                  DataMap.Resource_Type.dbTwoCluster.value,
                  DataMap.Resource_Type.dbTwoClusterInstance.value,
                  DataMap.Resource_Type.dbTwoInstance.value,
                  DataMap.Resource_Type.dbTwoDatabase.value,
                  DataMap.Resource_Type.dbTwoTableSet.value,
                  DataMap.Resource_Type.HiveBackupSet.value,
                  DataMap.Resource_Type.ElasticsearchBackupSet.value,
                  DataMap.Resource_Type.fileset.value,
                  DataMap.Resource_Type.Hive.value,
                  DataMap.Resource_Type.HBase.value,
                  DataMap.Resource_Type.Elasticsearch.value,
                  DataMap.Resource_Type.informixService.value,
                  DataMap.Resource_Type.informixInstance.value,
                  DataMap.Resource_Type.informixClusterInstance.value,
                  DataMap.Resource_Type.OceanBaseCluster.value,
                  DataMap.Resource_Type.OceanBaseTenant.value,
                  DataMap.Resource_Type.tidbCluster.value,
                  DataMap.Resource_Type.tidbDatabase.value,
                  DataMap.Resource_Type.tidbTable.value,
                  DataMap.Resource_Type.kubernetesNamespaceCommon.value,
                  DataMap.Resource_Type.kubernetesDatasetCommon.value,
                  DataMap.Resource_Type.LocalFileSystem.value,
                  DataMap.Resource_Type.ObjectStorage.value,
                  DataMap.Resource_Type.ObjectSet.value,
                  DataMap.Resource_Type.APSCloudServer.value,
                  DataMap.Resource_Type.APSZone.value,
                  DataMap.Resource_Type.APSResourceSet.value,
                  DataMap.Resource_Type.cNwareVm.value,
                  DataMap.Resource_Type.lightCloudGaussdbInstance.value,
                  DataMap.Resource_Type.openStackCloudServer.value,
                  DataMap.Resource_Type.hyperVVm.value,
                  DataMap.Resource_Type.ExchangeSingle.value,
                  DataMap.Resource_Type.ExchangeGroup.value,
                  DataMap.Resource_Type.ExchangeDataBase.value,
                  DataMap.Resource_Type.ExchangeEmail.value,
                  DataMap.Resource_Type.volume.value,
                  DataMap.Resource_Type.nutanixVm.value
                ],
                item.sub_type
              )
            ) {
              this.protectedResourceApiService
                .ShowResource({
                  resourceId: item.uuid
                })
                .subscribe(resource => {
                  if (
                    includes(
                      [
                        DataMap.Resource_Type.OpenGauss_instance.value,
                        DataMap.Resource_Type.OpenGauss_database.value
                      ],
                      item.sub_type
                    )
                  ) {
                    this.getOpenGaussItem(resource);
                  }

                  if (
                    DataMap.Resource_Type.HCSProject.value === item.sub_type
                  ) {
                    assign(resource, {
                      cloudHostCount: +resource.extendInfo?.cloudHostCount || 0,
                      projectCount: +resource.extendInfo?.projectCount || 0
                    });
                  }

                  if (
                    includes(
                      [
                        DataMap.Resource_Type.FusionCompute.value,
                        DataMap.Resource_Type.fusionOne.value
                      ],
                      item.sub_type
                    )
                  ) {
                    assign(resource, {
                      vmNumber: +resource?.extendInfo?.vmNumber || 0
                    });
                  }

                  if (
                    [
                      DataMap.Resource_Type.lightCloudGaussdbInstance.value
                    ].includes(item.sub_type)
                  ) {
                    assign(resource, {
                      isAllowRestore: get(
                        resource,
                        'extendInfo.isAllowRestore',
                        'true'
                      )
                    });
                  }

                  if (
                    [
                      DataMap.Resource_Type.DWS_Database.value,
                      DataMap.Resource_Type.DWS_Schema.value,
                      DataMap.Resource_Type.DWS_Table.value,
                      DataMap.Resource_Type.DWS_Cluster.value
                    ].includes(item.sub_type)
                  ) {
                    assign(resource, {
                      isAllowRestore: get(
                        resource,
                        'extendInfo.isAllowRestore',
                        'false'
                      )
                    });
                  }

                  if (
                    includes(
                      [
                        DataMap.Resource_Type.tidbDatabase.value,
                        DataMap.Resource_Type.tidbTable.value,
                        DataMap.Resource_Type.AntDBInstance.value,
                        DataMap.Resource_Type.AntDBClusterInstance.value
                      ],
                      item.sub_type
                    )
                  ) {
                    assign(resource, {
                      linkStatus: resource.extendInfo.linkStatus
                    });
                  }
                  assign(item, resource);
                  this.total = res.total;
                  this.tableData = res.items;
                  this.tableData = _map(res.items as any, item =>
                    omit(item, ['ext_parameters'])
                  );
                  this.cdr.detectChanges();
                });
            }
          });
          this.total = res.total;
          this.tableData = _map(res.items as any, item =>
            omit(item, ['ext_parameters'])
          );
          this.cdr.detectChanges();
          this.calcTimeChange.emit({
            clusterId: this.filterParams?.clusterId,
            total: this.total,
            endTime: new Date().getTime(),
            isSearched: true
          });
        },
        () => {
          this.total = 0;
          this.tableData = [];
          this.cdr.detectChanges();
          this.calcTimeChange.emit({
            clusterId: this.filterParams?.clusterId,
            total: this.total,
            endTime: new Date().getTime(),
            isSearched: true
          });
        }
      );
  }

  getOpenGaussItem(item) {
    if (
      item.environment.extendInfo.clusterState ===
      DataMap.opengauss_Clusterstate.unavailable.value
    ) {
      assign(item, {
        sub_type: item.subType,
        belong_cluster: item['environment']['name'],
        owned_instance: item.parentName,
        instanceState: DataMap.openGauss_InstanceStatus.offline.value
      });
    } else {
      assign(item, {
        sub_type: item.subType,
        belong_cluster: item['environment']['name'],
        owned_instance: item.parentName,
        instanceState:
          item.extendInfo.instanceState ===
          DataMap.openGauss_InstanceStatus.normal.value
            ? DataMap.openGauss_InstanceStatus.normal.value
            : DataMap.openGauss_InstanceStatus.offline.value
      });
    }
  }

  trackById = (_, item) => {
    return item.uuid;
  };

  getResourceType(item) {
    if (item.sub_type === DataMap.Resource_Type.ClickHouse.value) {
      const typeMap = {
        TableSet: DataMap.globalResourceType.clickhouseTableSet.value,
        Cluster: DataMap.globalResourceType.clickhouseCluster.value,
        Database: DataMap.globalResourceType.clickhouseDatabase.value,
        Table: DataMap.globalResourceType.clickhouseTable.value,
        Node: DataMap.globalResourceType.clickhouseNode.value
      };
      return typeMap[item.type];
    }
    return item.sub_type || item.subType;
  }

  sortChange(source) {
    this.orderBy = source.key;
    this.orderType = source.direction;
    this.getResources();
  }

  searchSlaName(event) {
    if (this.slaNamePopover) {
      this.slaNamePopover.hide();
    }
    assign(this.filterParams, {
      sla_name: trim(event)
    });
    this.getResources();
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.pageNo = page.pageIndex;
    this.getResources();
  }

  getStore() {
    this.resourceStore$ = this.globalService
      .getState(SearchRange.RESOURCES)
      .subscribe(params => {
        if (isEmpty(omit(params, ['clusterId', 'clusterType']))) {
          this.total = CommonConsts.PAGE_TOTAL;
          this.tableData = [];
          this.cdr.detectChanges();
          this.calcTimeChange.emit({
            total: 0,
            isSearched: false
          });
          return;
        }
        this.filterParams = !this.cookieService.isCloudBackup
          ? omit(params, ['resourceType'])
          : assign(params, {
              sub_type: [DataMap.Resource_Type.LocalFileSystem.value]
            });
        this.getResources();
      });
    this.resourceRefreshStore$ = this.globalService
      .getState(SearchResource.Refresh)
      .subscribe(() => this.getResources());
  }

  getSlaDetail(item) {
    if (!item.sla_id) {
      return;
    }
    this.slaService.getDetail({ uuid: item.sla_id, name: item.sla_name });
  }

  optsCallback = data => {
    return this.getOptItems(data);
  };

  getNasItems(item) {
    const opts = {
      protect: {
        id: 'protect',
        type: this.cookieService.isCloudBackup ? 'default' : 'primary',
        disableCheck: item => {
          return !isEmpty(item[0].sla_id);
        },
        label: this.i18n.get('common_protect_label'),
        onClick: data => this.protect([item], ProtectResourceAction.Create)
      },
      modifyProtect: {
        id: 'modifyProtect',
        disableCheck: item => {
          return isEmpty(item[0].sla_id);
        },
        label: this.i18n.get('common_resource_protection_modify_label'),
        onClick: data => this.protect([item], ProtectResourceAction.Modify)
      },
      removeProtection: {
        id: 'removeProtection',
        divide: true,
        disableCheck: item => {
          return isEmpty(item[0].sla_id);
        },
        label: this.i18n.get('protection_remove_protection_label'),
        onClick: data => {
          this.protectService
            .removeProtection(_map([item], 'uuid'), _map([item], 'name'))
            .subscribe(res => this.getResources());
        }
      },
      activeProtection: {
        id: 'activeProtection',
        disableCheck: item => {
          return isEmpty(item[0].sla_id) || item[0].sla_status;
        },
        disabledTips: this.i18n.get(
          'protection_partial_resources_active_label'
        ),
        label: this.i18n.get('protection_active_protection_label'),
        onClick: data => {
          this.protectService
            .activeProtection(_map([item], 'uuid'))
            .subscribe(res => {
              this.getResources();
              if (
                includes(
                  mapValues(this.drawModalService.modals, 'key'),
                  'detail-modal'
                ) &&
                size([item]) === 1
              ) {
                this.getResourceDetail(first([item]));
              }
            });
        }
      },
      deactiveProtection: {
        id: 'deactiveProtection',
        divide: true,
        disableCheck: item => {
          return isEmpty(item[0].sla_id) || !item[0].sla_status;
        },
        disabledTips: this.i18n.get(
          'protection_partial_resources_deactive_label'
        ),
        label: this.i18n.get('protection_deactive_protection_label'),
        onClick: data => {
          this.protectService
            .deactiveProtection(_map([item], 'uuid'), _map([item], 'name'))
            .subscribe(res => {
              this.getResources();
              if (
                includes(
                  mapValues(this.drawModalService.modals, 'key'),
                  'detail-modal'
                ) &&
                size([item]) === 1
              ) {
                this.getResourceDetail(first([item]));
              }
            });
        }
      },
      recovery: {
        id: 'recovery',
        disableCheck: () => {
          return false;
        },
        label: this.i18n.get('common_restore_label'),
        onClick: data =>
          this.getResourceDetail({
            ...item,
            activeId: 'copydata',
            datePickerMode: DATE_PICKER_MODE.DATE
          })
      },
      manualBackup: {
        id: 'manualBackup',
        disableCheck: item => {
          return isEmpty(item[0].sla_id);
        },
        label: this.i18n.get('common_manual_backup_label'),
        onClick: data => {
          this.manualBackup([item]);
        }
      }
    };

    if (
      item.sub_type === DataMap.Resource_Type.NASShare.value ||
      item.subType === DataMap.Resource_Type.NASShare.value
    ) {
      assign(opts, {
        modify: {
          id: 'modify',
          permission: OperateItems.RegisterNasShare,
          label: this.i18n.get('common_modify_label'),
          onClick: data => {
            this.modifyNas(item);
          },
          disableCheck: data => {
            return false;
          }
        }
      });
    }

    return opts;
  }

  getNasDetails(item) {
    this.protectedResourceApiService
      .ShowResource({
        resourceId: item.uuid
      })
      .subscribe(res => {
        if (!res || isEmpty(res)) {
          this.messageService.error(
            this.i18n.get('common_resource_not_exist_label'),
            {
              lvShowCloseButton: true,
              lvMessageKey: 'resNotExistMesageKey'
            }
          );
          return;
        }

        const data = assign(res, {
          sub_type: res.subType,
          protocol: res.extendInfo?.protocol,
          sla_id: res.protectedObject?.slaId
        });

        if (item.activeId) {
          set(data, 'activeId', 'copydata');
          set(data, 'datePickerMode', DATE_PICKER_MODE.DATE);
        }

        if (
          includes(
            mapValues(this.drawModalService.modals, 'key'),
            'slaDetailModalKey'
          )
        ) {
          this.drawModalService.destroyModal('slaDetailModalKey');
        }
        extendSlaInfo(data);
        this.detailService.openDetailModal(data.subType, {
          data: assign(
            data,
            {
              optItems: getTableOptsItems(
                cloneDeep(
                  getPermissionMenuItem(values(this.getNasItems(item)))
                ),
                data,
                this
              )
            },
            {
              optItemsFn: v => {
                return getTableOptsItems(
                  cloneDeep(
                    getPermissionMenuItem(values(this.getNasItems(item)))
                  ),
                  v,
                  this
                );
              }
            }
          )
        });
      });
  }

  modifyNas(datas) {
    this.protectedResourceApiService
      .ShowResource({
        resourceId: datas.uuid
      })
      .subscribe(item => {
        if (!isEmpty(item)) {
          assign(item, {
            filters: item['extendInfo'].filters
              ? JSON.parse(item['extendInfo'].filters)
              : '[]'
          });
        }
        this.drawModalService.create(
          assign({}, MODAL_COMMON.generateDrawerOptions(), {
            lvModalKey: 'reigster-nas-shared',
            lvWidth: MODAL_COMMON.normalWidth + 100,
            lvHeader: isEmpty(item)
              ? this.i18n.get('common_register_label')
              : this.i18n.get('common_modify_label'),
            lvContent: RegisterNasShareComponent,
            lvOkDisabled: true,
            lvComponentParams: {
              items: [],
              item: {
                ...item,
                sub_type: DataMap.Resource_Type.NASShare.value
              }
            },
            lvAfterOpen: modal => {
              const content = modal.getContentComponent() as RegisterNasShareComponent;
              const modalIns = modal.getInstance();
              content.formGroup.statusChanges.subscribe(res => {
                modalIns.lvOkDisabled = res !== 'VALID';
              });
              content.formGroup.updateValueAndValidity();
            },
            lvOk: modal => {
              return new Promise(resolve => {
                const content = modal.getContentComponent() as RegisterNasShareComponent;
                content.onOK().subscribe({
                  next: res => {
                    resolve(true);
                    if (
                      !isEmpty(item) &&
                      includes(
                        mapValues(this.drawModalService.modals, 'key'),
                        'detail-modal'
                      )
                    ) {
                      this.getResourceDetail(content.item);
                    } else {
                      this.getResources();
                    }
                  },
                  error: () => resolve(false)
                });
              });
            },
            lvCancel: modal => {
              const content = modal.getContentComponent() as RegisterNasShareComponent;
              if (
                !isEmpty(item) &&
                includes(
                  mapValues(this.drawModalService.modals, 'key'),
                  'detail-modal'
                )
              ) {
                this.getResourceDetail(content.item);
              }
            }
          })
        );
      });
  }

  getOptItems(item) {
    if (
      includes(
        [
          DataMap.Resource_Type.FusionCompute.value,
          DataMap.Resource_Type.fusionOne.value
        ],
        item.sub_type
      )
    ) {
      switch (item.type) {
        case DataMap.Resource_Type.fusionComputeCNA.value:
          this.fcListComponent.tab = {
            id: ResourceType.HOST,
            type: ResourceType.HOST
          };
          return this.fcListComponent.getOptsItems(item);
        case DataMap.Resource_Type.fusionComputeVirtualMachine.value:
          this.fcListComponent.tab = {
            id: ResourceType.VM,
            type: ResourceType.VM
          };
          return this.fcListComponent.getOptsItems(item);
        case DataMap.Resource_Type.fusionComputeCluster.value:
          this.fcListComponent.tab = {
            id: ResourceType.CLUSTER,
            type: ResourceType.CLUSTER
          };
          return this.fcListComponent.getOptsItems(item);
      }
    }
    switch (item.sub_type) {
      case DataMap.Resource_Type.ABBackupClient.value:
      case DataMap.Resource_Type.VMBackupAgent.value:
      case DataMap.Resource_Type.DBBackupAgent.value:
        return this.hostComponent.getOptItems(item);
      case DataMap.Resource_Type.oracle.value:
      case DataMap.Resource_Type.oracleCluster.value:
        return this.oracleComponent.getOptItems(item);
      case DataMap.Resource_Type.LocalFileSystem.value:
        this.doradoFileSystemComponent.initConfig();
        return this.doradoFileSystemComponent.optItems;
      case DataMap.Resource_Type.hostSystem.value:
        this.vmListComponent.tab = {
          id: ResourceType.HOST,
          resType: ResourceType.VM
        };
        return this.vmListComponent.getOptsItems(item);
      case DataMap.Resource_Type.virtualMachine.value:
        this.vmListComponent.tab = {
          id: ResourceType.VM,
          resType: ResourceType.VM
        };
        return this.vmListComponent.getOptsItems(item);
      case DataMap.Resource_Type.clusterComputeResource.value:
        this.vmListComponent.tab = {
          id: ResourceType.CLUSTER,
          resType: ResourceType.VM
        };
        return this.vmListComponent.getOptsItems(item);
      case DataMap.Resource_Type.NASFileSystem.value:
      case DataMap.Resource_Type.ndmp.value:
        return getTableOptsItems(
          cloneDeep(getPermissionMenuItem(values(this.getNasItems(item)))),
          item,
          this
        );
      case DataMap.Resource_Type.NASShare.value:
        return getTableOptsItems(
          cloneDeep(getPermissionMenuItem(values(this.getNasItems(item)))),
          item,
          this
        );
      case DataMap.Resource_Type.HDFSFileset.value:
        this.hdfsFilesetsComponent.initConfig();
        return getTableOptsItems(
          this.hdfsFilesetsComponent.optItems,
          item,
          this.hdfsFilesetsComponent
        );
      case DataMap.Resource_Type.HiveBackupSet.value:
      case DataMap.Resource_Type.ElasticsearchBackupSet.value:
      case DataMap.Resource_Type.HBaseBackupSet.value:
        this.hbaseBackupSetComponent.initConfig();
        return getTableOptsItems(
          this.hbaseBackupSetComponent.optItems,
          item,
          this.hbaseBackupSetComponent
        );
      case DataMap.Resource_Type.Kubernetes.value:
        this.kubernetesClusterComponent.initConfig();
        return getTableOptsItems(
          this.kubernetesClusterComponent.optItems,
          item,
          this.kubernetesClusterComponent
        );
      case DataMap.Resource_Type.KubernetesNamespace.value:
      case DataMap.Resource_Type.KubernetesStatefulset.value:
        this.kubernetesComponent.columns = [];
        this.kubernetesComponent.resourceSubType = item.sub_type;
        this.kubernetesComponent.initConfig();
        return getTableOptsItems(
          this.kubernetesComponent.optItems,
          item,
          this.kubernetesComponent
        );
      case DataMap.Resource_Type.kubernetesClusterCommon.value:
        this.kubernetesClusterComponent.subType =
          DataMap.Resource_Type.kubernetesClusterCommon.value;
        this.kubernetesClusterComponent.initConfig();
        return getTableOptsItems(
          this.kubernetesClusterComponent.optItems,
          item,
          this.kubernetesClusterComponent
        );
      case DataMap.Resource_Type.kubernetesNamespaceCommon.value:
      case DataMap.Resource_Type.kubernetesDatasetCommon.value:
        this.kubernetesComponent.columns = [];
        this.kubernetesComponent.resourceSubType = item.sub_type;
        this.kubernetesComponent.extraConfig = this.kubernetesContainerComponent.datasetExtraConfig;
        this.kubernetesComponent.initConfig();
        return getTableOptsItems(
          this.kubernetesComponent.optItems,
          item,
          this.kubernetesComponent
        );
      case DataMap.Resource_Type.Redis.value:
        this.redisClusterComponent.initConfig();
        return getTableOptsItems(
          this.redisClusterComponent.optItems,
          item,
          this.redisClusterComponent
        );
      case DataMap.Resource_Type.PostgreSQLCluster.value:
        this.postgreClusterComponent.initConfig();
        return getTableOptsItems(
          this.postgreClusterComponent.optItems,
          item,
          this.postgreClusterComponent
        );
      case DataMap.Resource_Type.PostgreSQLInstance.value:
      case DataMap.Resource_Type.PostgreSQLClusterInstance.value:
        this.postgreInstanceComponent.initConfig();
        return getTableOptsItems(
          this.postgreInstanceComponent.optItems,
          item,
          this.postgreInstanceComponent
        );
      case DataMap.Resource_Type.KingBaseCluster.value:
        this.kingBaseClusterComponent.initConfig();
        return getTableOptsItems(
          this.kingBaseClusterComponent.optItems,
          item,
          this.kingBaseClusterComponent
        );
      case DataMap.Resource_Type.KingBaseInstance.value:
      case DataMap.Resource_Type.KingBaseClusterInstance.value:
        this.kingBaseInstanceComponent.initConfig();
        return getTableOptsItems(
          this.kingBaseInstanceComponent.optItems,
          item,
          this.kingBaseInstanceComponent
        );
      case DataMap.Resource_Type.HCSTenant.value:
        this.hcsListComponent.tab = {
          id: ResourceType.TENANT,
          type: ResourceType.TENANT,
          sub_type: 'HCSTenant'
        };
        return this.hcsListComponent.getOptsItems(item);
      case DataMap.Resource_Type.HCSProject.value:
        this.hcsListComponent.tab = {
          id: ResourceType.PROJECT,
          type: ResourceType.PROJECT,
          sub_type: 'HCSProject'
        };
        return this.hcsListComponent.getOptsItems(item);
      case DataMap.Resource_Type.HCSCloudHost.value:
        this.hcsListComponent.tab = {
          id: ResourceType.CLOUD_HOST,
          type: ResourceType.CLOUD_HOST,
          sub_type: 'HCSCloudHost'
        };
        return this.hcsListComponent.getOptsItems(item);
      case DataMap.Resource_Type.fileset.value:
        this.filesetListComponent.getOptsItems(item);
        return getTableOptsItems(
          this.filesetListComponent.optItems,
          item,
          this.filesetListComponent
        );
      case DataMap.Resource_Type.MySQLCluster.value:
        this.mysqlClusterComponent.initConfig();
        return getTableOptsItems(
          this.mysqlClusterComponent.optItems,
          item,
          this.mysqlClusterComponent
        );
      case DataMap.Resource_Type.MySQLClusterInstance.value:
      case DataMap.Resource_Type.MySQLInstance.value:
        this.mysqlListComponent.activeIndex = 'instance';
        this.mysqlListComponent.initConfig();
        return getTableOptsItems(
          this.mysqlListComponent.optItems,
          item,
          this.mysqlListComponent
        );
      case DataMap.Resource_Type.MySQLDatabase.value:
        this.mysqlListComponent.activeIndex = 'database';
        this.mysqlListComponent.initConfig();
        return getTableOptsItems(
          this.mysqlListComponent.optItems,
          item,
          this.mysqlListComponent
        );
      case DataMap.Resource_Type.ClickHouse.value: {
        if (item.type === ResourceType.TABLE_SET) {
          this.clickHouseTablesetComonent.initConfig();
          return getTableOptsItems(
            this.clickHouseTablesetComonent.opts,
            item,
            this.clickHouseTablesetComonent
          );
        } else if (item.type === ResourceType.DATABASE) {
          this.clickhouseDatabaseComponent.initConfig();
          return getTableOptsItems(
            this.clickhouseDatabaseComponent.opts,
            item,
            this.clickhouseDatabaseComponent
          );
        } else if (item.type === ResourceType.CLUSTER) {
          this.clickHouseClusterComonent.initConfig();
          return getTableOptsItems(
            this.clickHouseClusterComonent.opts,
            item,
            this.clickHouseClusterComonent
          );
        } else {
          return;
        }
      }
      case DataMap.Resource_Type.AntDBInstance.value:
      case DataMap.Resource_Type.AntDBClusterInstance.value:
        this.databaseTemplateComponent.configParams = {
          activeIndex: DataMap.Resource_Type.AntDB.value,
          tableCols: [],
          tableOpts: [
            'register',
            'protect',
            'modifyProtect',
            'removeProtection',
            'activeProtection',
            'deactiveProtection',
            'recovery',
            'manualBackup',
            'connectivityTest',
            'modify',
            'deleteResource'
          ],
          registerComponent: RegisterAntDBInstanceComponent
        };
        this.databaseTemplateComponent.initConfig();
        return getTableOptsItems(
          this.databaseTemplateComponent.optItems,
          item,
          this.databaseTemplateComponent
        );
      case DataMap.Resource_Type.saphanaDatabase.value:
        this.databaseTemplateComponent.configParams = {
          activeIndex: DataMap.Resource_Type.saphanaDatabase.value,
          tableCols: [],
          tableOpts: [
            'register',
            'protect',
            'modifyProtect',
            'removeProtection',
            'activeProtection',
            'deactiveProtection',
            'recovery',
            'manualBackup',
            'connectivityTest',
            'modify',
            'deleteResource'
          ],
          registerComponent: RegisterSaphanaDatabaseComponent
        };
        this.databaseTemplateComponent.initConfig();
        return getTableOptsItems(
          this.databaseTemplateComponent.optItems,
          item,
          this.databaseTemplateComponent
        );
      case DataMap.Resource_Type.saphanaInstance.value:
        this.databaseTemplateComponent.configParams = {
          activeIndex: DataMap.Resource_Type.saphanaInstance.value,
          tableCols: [],
          tableOpts: [
            'register',
            'resourceAuth',
            'resourceReclaiming',
            'connectivityTest',
            'modify',
            'deleteResource'
          ],
          registerComponent: RegisterSaphanaInstanceComponent
        };
        this.databaseTemplateComponent.initConfig();
        return getTableOptsItems(
          this.databaseTemplateComponent.optItems,
          item,
          this.databaseTemplateComponent
        );
      case DataMap.Resource_Type.saponoracleDatabase.value:
        this.databaseTemplateComponent.configParams = {
          activeIndex: DataMap.Resource_Type.saponoracleDatabase.value,
          tableCols: [],
          tableOpts: [
            'register',
            'protect',
            'modifyProtect',
            'removeProtection',
            'activeProtection',
            'deactiveProtection',
            'recovery',
            'manualBackup',
            'connectivityTest',
            'modify',
            'deleteResource'
          ],
          registerComponent: SaponoracleRegisterDatabaseComponent
        };
        this.databaseTemplateComponent.initConfig();
        return getTableOptsItems(
          this.databaseTemplateComponent.optItems,
          item,
          this.databaseTemplateComponent
        );
      case DataMap.Resource_Type.informixService.value:
        this.databaseTemplateComponent.configParams = {
          activeIndex: DataMap.Resource_Type.informixService.value,
          tableCols: [],
          tableOpts: [
            'register',
            'resourceAuth',
            'resourceReclaiming',
            'connectivityTest',
            'modify',
            'deleteResource'
          ],
          registerComponent: RegisterInformixServiceComponent
        };
        this.databaseTemplateComponent.initConfig();
        return getTableOptsItems(
          this.databaseTemplateComponent.optItems,
          item,
          this.databaseTemplateComponent
        );
      case DataMap.Resource_Type.informixInstance.value:
      case DataMap.Resource_Type.informixClusterInstance.value:
        this.databaseTemplateComponent.configParams = {
          activeIndex: DataMap.Resource_Type.informixInstance.value,
          tableCols: [],
          tableOpts: [
            'register',
            'protect',
            'modifyProtect',
            'removeProtection',
            'activeProtection',
            'deactiveProtection',
            'recovery',
            'manualBackup',
            'connectivityTest',
            'modify',
            'deleteResource'
          ],
          registerComponent: RegisterInformixInstanceComponent
        };
        this.databaseTemplateComponent.initConfig();
        return getTableOptsItems(
          this.databaseTemplateComponent.optItems,
          item,
          this.databaseTemplateComponent
        );
      case DataMap.Resource_Type.tdsqlCluster.value:
        this.databaseTemplateComponent.configParams = {
          activeIndex: DataMap.Resource_Type.tdsqlCluster.value,
          tableCols: [],
          tableOpts: [
            'register',
            'resourceAuth',
            'resourceReclaiming',
            'connectivityTest',
            'modify',
            'deleteResource'
          ],
          registerComponent: RegisterClusterComponent
        };
        this.databaseTemplateComponent.initConfig();
        return getTableOptsItems(
          this.databaseTemplateComponent.optItems,
          item,
          this.databaseTemplateComponent
        );
      case DataMap.Resource_Type.tdsqlInstance.value:
        this.databaseTemplateComponent.configParams = {
          activeIndex: DataMap.Resource_Type.tdsqlInstance.value,
          tableCols: [],
          tableOpts: [
            'register',
            'protect',
            'modifyProtect',
            'removeProtection',
            'activeProtection',
            'deactiveProtection',
            'recovery',
            'manualBackup',
            'connectivityTest',
            'modify',
            'deleteResource'
          ],
          registerComponent: RegisterInstanceComponent
        };
        this.databaseTemplateComponent.initConfig();
        return getTableOptsItems(
          this.databaseTemplateComponent.optItems,
          item,
          this.databaseTemplateComponent
        );
      case DataMap.Resource_Type.tdsqlDistributedInstance.value:
        this.databaseTemplateComponent.configParams = {
          activeIndex: DataMap.Resource_Type.tdsqlDistributedInstance.value,
          tableCols: [],
          tableOpts: [
            'register',
            'protect',
            'modifyProtect',
            'removeProtection',
            'activeProtection',
            'deactiveProtection',
            'recovery',
            'manualBackup',
            'connectivityTest',
            'modify',
            'deleteResource'
          ],
          registerComponent: RegisterDistributedInstanceComponent
        };
        this.databaseTemplateComponent.initConfig();
        return getTableOptsItems(
          this.databaseTemplateComponent.optItems,
          item,
          this.databaseTemplateComponent
        );
      case DataMap.Resource_Type.OceanBaseCluster.value:
        this.databaseTemplateComponent.configParams = {
          activeIndex: DataMap.Resource_Type.OceanBaseCluster.value,
          tableCols: [],
          tableOpts: [
            'protect',
            'modifyProtect',
            'removeProtection',
            'activeProtection',
            'deactiveProtection',
            'recovery',
            'manualBackup',
            'connectivityTest',
            'modify',
            'deleteResource'
          ],
          registerComponent: RegisterOceanBaseComponent
        };
        this.databaseTemplateComponent.initConfig();
        return getTableOptsItems(
          this.databaseTemplateComponent.optItems,
          item,
          this.databaseTemplateComponent
        );
      case DataMap.Resource_Type.OceanBaseTenant.value:
        this.databaseTemplateComponent.configParams = {
          activeIndex: DataMap.Resource_Type.OceanBaseTenant.value,
          tableCols: [],
          tableOpts: [
            'protect',
            'modifyProtect',
            'removeProtection',
            'activeProtection',
            'deactiveProtection',
            'recovery',
            'manualBackup',
            'modify',
            'deleteResource'
          ],
          registerComponent: RegisterTenantComponent
        };
        this.databaseTemplateComponent.initConfig();
        return getTableOptsItems(
          this.databaseTemplateComponent.optItems,
          item,
          this.databaseTemplateComponent
        );
      case DataMap.Resource_Type.tidbCluster.value:
        this.databaseTemplateComponent.configParams = {
          activeIndex: DataMap.Resource_Type.tidbCluster.value,
          tableCols: [],
          tableOpts: [
            'protect',
            'modifyProtect',
            'removeProtection',
            'activeProtection',
            'deactiveProtection',
            'recovery',
            'manualBackup',
            'connectivityTest',
            'modify',
            'deleteResource'
          ],
          registerComponent: RegisterTidbClusterComponent
        };
        this.databaseTemplateComponent.initConfig();
        return getTableOptsItems(
          this.databaseTemplateComponent.optItems,
          item,
          this.databaseTemplateComponent
        );
      case DataMap.Resource_Type.tidbDatabase.value:
        this.databaseTemplateComponent.configParams = {
          activeIndex: DataMap.Resource_Type.tidbDatabase.value,
          tableCols: [],
          tableOpts: [
            'protect',
            'modifyProtect',
            'removeProtection',
            'activeProtection',
            'deactiveProtection',
            'recovery',
            'manualBackup',
            'connectivityTest',
            'modify',
            'deleteResource'
          ],
          registerComponent: RegisterTidbDatabaseComponent
        };
        this.databaseTemplateComponent.initConfig();
        return getTableOptsItems(
          this.databaseTemplateComponent.optItems,
          item,
          this.databaseTemplateComponent
        );
      case DataMap.Resource_Type.tidbTable.value:
        this.databaseTemplateComponent.configParams = {
          activeIndex: DataMap.Resource_Type.tidbTable.value,
          tableCols: [],
          tableOpts: [
            'protect',
            'modifyProtect',
            'removeProtection',
            'activeProtection',
            'deactiveProtection',
            'recovery',
            'manualBackup',
            'connectivityTest',
            'modify',
            'deleteResource'
          ],
          registerComponent: RegisterTidbTableComponent
        };
        this.databaseTemplateComponent.initConfig();
        return getTableOptsItems(
          this.databaseTemplateComponent.optItems,
          item,
          this.databaseTemplateComponent
        );
      case DataMap.Resource_Type.ExchangeSingle.value:
      case DataMap.Resource_Type.ExchangeGroup.value:
        this.databaseTemplateComponent.configParams = {
          activeIndex: DataMap.Resource_Type.Exchange.value,
          tableCols: [],
          tableOpts: [
            'protect',
            'modifyProtect',
            'removeProtection',
            'activeProtection',
            'deactiveProtection',
            'recovery',
            'manualBackup',
            'rescan',
            'connectivityTest',
            'modify',
            'deleteResource'
          ],
          registerComponent: RegisterGroupComponent
        };
        this.databaseTemplateComponent.initConfig();
        return getTableOptsItems(
          this.databaseTemplateComponent.optItems,
          item,
          this.databaseTemplateComponent
        );
      case DataMap.Resource_Type.ExchangeDataBase.value:
        this.databaseTemplateComponent.configParams = {
          activeIndex: DataMap.Resource_Type.ExchangeDataBase.value,
          tableCols: [],
          tableOpts: [
            'protect',
            'modifyProtect',
            'removeProtection',
            'activeProtection',
            'deactiveProtection',
            'recovery',
            'manualBackup'
          ]
        };
        this.databaseTemplateComponent.initConfig();
        return getTableOptsItems(
          this.databaseTemplateComponent.optItems,
          item,
          this.databaseTemplateComponent
        );
      case DataMap.Resource_Type.ExchangeEmail.value:
        this.databaseTemplateComponent.configParams = {
          activeIndex: DataMap.Resource_Type.ExchangeEmail.value,
          tableCols: [],
          tableOpts: [
            'protect',
            'modifyProtect',
            'removeProtection',
            'activeProtection',
            'deactiveProtection',
            'recovery',
            'manualBackup'
          ]
        };
        this.databaseTemplateComponent.initConfig();
        return getTableOptsItems(
          this.databaseTemplateComponent.optItems,
          item,
          this.databaseTemplateComponent
        );
      case DataMap.Resource_Type.gaussdbForOpengauss.value:
        this.instanceDatabaseComponent.activeIndex =
          DataMap.Resource_Type.gaussdbForOpengauss.value;
        this.instanceDatabaseComponent.initConfig();
        return getTableOptsItems(
          this.instanceDatabaseComponent.optItems,
          item,
          this.instanceDatabaseComponent
        );
      case DataMap.Resource_Type.gaussdbForOpengaussProject.value:
        this.instanceDatabaseComponent.activeIndex =
          DataMap.Resource_Type.gaussdbForOpengaussProject.value;
        this.instanceDatabaseComponent.initConfig();
        return getTableOptsItems(
          this.instanceDatabaseComponent.optItems,
          item,
          this.instanceDatabaseComponent
        );
      case DataMap.Resource_Type.SQLServerCluster.value:
        this.instanceDatabaseComponent.activeIndex =
          DataMap.Resource_Type.SQLServerCluster.value;
        this.instanceDatabaseComponent.initConfig();
        return getTableOptsItems(
          this.instanceDatabaseComponent.optItems,
          item,
          this.instanceDatabaseComponent
        );
      case DataMap.Resource_Type.SQLServerClusterInstance.value:
      case DataMap.Resource_Type.SQLServerInstance.value:
        this.instanceDatabaseComponent.activeIndex =
          DataMap.Resource_Type.SQLServerInstance.value;
        this.instanceDatabaseComponent.initConfig();
        return getTableOptsItems(
          this.instanceDatabaseComponent.optItems,
          item,
          this.instanceDatabaseComponent
        );
      case DataMap.Resource_Type.SQLServerGroup.value:
        this.instanceDatabaseComponent.activeIndex =
          DataMap.Resource_Type.SQLServerGroup.value;
        this.instanceDatabaseComponent.initConfig();
        return getTableOptsItems(
          this.instanceDatabaseComponent.optItems,
          item,
          this.instanceDatabaseComponent
        );
      case DataMap.Resource_Type.SQLServerDatabase.value:
        this.instanceDatabaseComponent.activeIndex =
          DataMap.Resource_Type.SQLServerDatabase.value;
        this.instanceDatabaseComponent.initConfig();
        return getTableOptsItems(
          this.instanceDatabaseComponent.optItems,
          item,
          this.instanceDatabaseComponent
        );
      case DataMap.Resource_Type.DWS_Cluster.value:
        this.instanceDatabaseComponent.subType =
          DataMap.Resource_Type.DWS_Cluster.value;
        this.instanceDatabaseComponent.activeIndex =
          DataMap.Resource_Type.DWS_Cluster.value;
        this.instanceDatabaseComponent.initConfig();
        return getTableOptsItems(
          this.instanceDatabaseComponent.optItems,
          item,
          this.instanceDatabaseComponent
        );
      case DataMap.Resource_Type.DWS_Database.value:
        this.instanceDatabaseComponent.subType =
          DataMap.Resource_Type.DWS_Database.value;
        this.instanceDatabaseComponent.activeIndex =
          DataMap.Resource_Type.DWS_Database.value;
        this.instanceDatabaseComponent.initConfig();
        return getTableOptsItems(
          this.instanceDatabaseComponent.optItems,
          item,
          this.instanceDatabaseComponent
        );
      case DataMap.Resource_Type.DWS_Schema.value:
        this.instanceDatabaseComponent.subType =
          DataMap.Resource_Type.DWS_Schema.value;
        this.instanceDatabaseComponent.activeIndex =
          DataMap.Resource_Type.DWS_Schema.value;
        this.instanceDatabaseComponent.initConfig();
        return getTableOptsItems(
          this.instanceDatabaseComponent.optItems,
          item,
          this.instanceDatabaseComponent
        );
      case DataMap.Resource_Type.DWS_Table.value:
        this.instanceDatabaseComponent.subType =
          DataMap.Resource_Type.DWS_Table.value;
        this.instanceDatabaseComponent.activeIndex =
          DataMap.Resource_Type.DWS_Table.value;
        this.instanceDatabaseComponent.initConfig();
        return getTableOptsItems(
          this.instanceDatabaseComponent.optItems,
          item,
          this.instanceDatabaseComponent
        );
      case DataMap.Resource_Type.goldendbCluter.value:
        this.instanceDatabaseComponent.subType =
          DataMap.Resource_Type.goldendbCluter.value;
        this.instanceDatabaseComponent.activeIndex =
          DataMap.Resource_Type.goldendbCluter.value;
        this.instanceDatabaseComponent.initConfig();
        return getTableOptsItems(
          this.instanceDatabaseComponent.optItems,
          item,
          this.instanceDatabaseComponent
        );
      case DataMap.Resource_Type.goldendbInstance.value:
        this.instanceDatabaseComponent.subType =
          DataMap.Resource_Type.goldendbInstance.value;
        this.instanceDatabaseComponent.activeIndex =
          DataMap.Resource_Type.goldendbInstance.value;
        this.instanceDatabaseComponent.initConfig();
        return getTableOptsItems(
          this.instanceDatabaseComponent.optItems,
          item,
          this.instanceDatabaseComponent
        );
      case DataMap.Resource_Type.generalDatabase.value:
        this.generalDatabaseComponent.sourceType =
          DataMap.Resource_Type.generalDatabase.value;
        this.generalDatabaseComponent.initConfig();
        return getTableOptsItems(
          this.generalDatabaseComponent.optItems,
          item,
          this.generalDatabaseComponent
        );
      case DataMap.Resource_Type.dbTwoCluster.value:
        this.instanceDatabaseComponent.subType =
          DataMap.Resource_Type.dbTwoCluster.value;
        this.instanceDatabaseComponent.activeIndex =
          DataMap.Resource_Type.dbTwoCluster.value;
        this.instanceDatabaseComponent.initConfig();
        return getTableOptsItems(
          this.instanceDatabaseComponent.optItems,
          item,
          this.instanceDatabaseComponent
        );
      case DataMap.Resource_Type.dbTwoClusterInstance.value:
      case DataMap.Resource_Type.dbTwoInstance.value:
        this.instanceDatabaseComponent.subType =
          DataMap.Resource_Type.dbTwoCluster.value;
        this.instanceDatabaseComponent.activeIndex =
          DataMap.Resource_Type.dbTwoInstance.value;
        this.instanceDatabaseComponent.initConfig();
        return getTableOptsItems(
          this.instanceDatabaseComponent.optItems,
          item,
          this.instanceDatabaseComponent
        );
      case DataMap.Resource_Type.dbTwoDatabase.value:
        this.instanceDatabaseComponent.subType =
          DataMap.Resource_Type.dbTwoCluster.value;
        this.instanceDatabaseComponent.activeIndex =
          DataMap.Resource_Type.dbTwoDatabase.value;
        this.instanceDatabaseComponent.initConfig();
        return getTableOptsItems(
          this.instanceDatabaseComponent.optItems,
          item,
          this.instanceDatabaseComponent
        );
      case DataMap.Resource_Type.dbTwoTableSet.value:
        this.instanceDatabaseComponent.subType =
          DataMap.Resource_Type.dbTwoCluster.value;
        this.instanceDatabaseComponent.activeIndex =
          DataMap.Resource_Type.dbTwoTableSet.value;
        this.instanceDatabaseComponent.initConfig();
        return getTableOptsItems(
          this.instanceDatabaseComponent.optItems,
          item,
          this.instanceDatabaseComponent
        );
      case DataMap.Resource_Type.OpenGauss.value:
        this.openGaussClusterComponent.initConfig();
        return getTableOptsItems(
          this.openGaussClusterComponent.optsConfigSearch,
          item,
          this.openGaussClusterComponent
        );
      case DataMap.Resource_Type.OpenGauss_instance.value:
        this.openGaussComponent.initConfig();
        this.openGaussComponent.resourceSubType =
          DataMap.Resource_Type.OpenGauss_instance.value;
        return getTableOptsItems(
          this.openGaussComponent.optItems,
          item,
          this.openGaussComponent
        );
      case DataMap.Resource_Type.OpenGauss_database.value:
        this.openGaussComponent.initConfig();
        this.openGaussComponent.resourceSubType =
          DataMap.Resource_Type.OpenGauss_database.value;
        return getTableOptsItems(
          this.openGaussComponent.optItems,
          item,
          this.openGaussComponent
        );
      case DataMap.Resource_Type.Dameng_cluster.value:
      case DataMap.Resource_Type.Dameng_singleNode.value:
        this.damengComponent.initConfig();
        return getTableOptsItems(
          this.damengComponent.opts,
          item,
          this.damengComponent
        );
      case DataMap.Resource_Type.gaussdbTSingle.value:
      case DataMap.Resource_Type.GaussDB_T.value:
        this.nasSharedComponent.subType = DataMap.Resource_Type.GaussDB_T.value;
        this.nasSharedComponent.initConfig();
        return getTableOptsItems(
          this.nasSharedComponent.optItems,
          item,
          this.nasSharedComponent
        );
      case DataMap.Resource_Type.Hive.value:
        this.clustersComponent.resSubType = DataMap.Resource_Type.Hive.value;
        this.clustersComponent.initConfig();
        return getTableOptsItems(
          this.clustersComponent.optItems,
          item,
          this.clustersComponent
        );
      case DataMap.Resource_Type.HBase.value:
        this.clustersComponent.resSubType = DataMap.Resource_Type.HBase.value;
        this.clustersComponent.initConfig();
        return getTableOptsItems(
          this.clustersComponent.optItems,
          item,
          this.clustersComponent
        );
      case DataMap.Resource_Type.Elasticsearch.value:
        this.clustersComponent.resSubType =
          DataMap.Resource_Type.Elasticsearch.value;
        this.clustersComponent.initConfig();
        return getTableOptsItems(
          this.clustersComponent.optItems,
          item,
          this.clustersComponent
        );
      case DataMap.Resource_Type.openStackProject.value:
      case DataMap.Resource_Type.openStackCloudServer.value:
        this.openstackListComponent.resType = item.sub_type;
        this.openstackListComponent.initConfig();
        return getTableOptsItems(
          this.openstackListComponent.optItems,
          item,
          this.openstackListComponent
        );
      case DataMap.Resource_Type.MongodbSingleInstance.value:
      case DataMap.Resource_Type.MongodbClusterInstance.value:
        this.kubernetesComponent.columns = [];
        this.kubernetesComponent.resourceSubType =
          DataMap.Resource_Type.MongoDB.value;
        this.kubernetesComponent.extraConfig = this.mongodbComponent.extraConfig;
        this.kubernetesComponent.initConfig();
        return getTableOptsItems(
          this.kubernetesComponent.optItems,
          item,
          this.kubernetesComponent
        );
      case DataMap.Resource_Type.lightCloudGaussdbProject.value:
        this.databaseTemplateComponent.configParams = {
          activeIndex: DataMap.Resource_Type.lightCloudGaussdbProject.value,
          tableCols: [],
          tableOpts: [
            'register',
            'resourceAuth',
            'resourceReclaiming',
            'rescan',
            'connectivityTest',
            'allowRecovery',
            'disableRecovery',
            'modify',
            'deleteResource'
          ],
          registerComponent: RegisterLightCloudGuassdbComponent
        };
        this.databaseTemplateComponent.initConfig();
        return getTableOptsItems(
          this.databaseTemplateComponent.optItems,
          item,
          this.databaseTemplateComponent
        );
      case DataMap.Resource_Type.lightCloudGaussdbInstance.value:
        this.databaseTemplateComponent.configParams = {
          activeIndex: DataMap.Resource_Type.lightCloudGaussdbInstance.value,
          tableCols: [],
          tableOpts: [
            'protect',
            'modifyProtect',
            'removeProtection',
            'activeProtection',
            'deactiveProtection',
            'allowRecovery',
            'disableRecovery',
            'recovery',
            'manualBackup'
          ]
        };
        this.databaseTemplateComponent.initConfig();
        return getTableOptsItems(
          this.databaseTemplateComponent.optItems,
          item,
          this.databaseTemplateComponent
        );
      case DataMap.Resource_Type.ObjectStorage.value:
        this.objectStorageComponent.initConfig();
        return getTableOptsItems(
          this.objectStorageComponent.optItems,
          item,
          this.objectStorageComponent
        );
      case DataMap.Resource_Type.ObjectSet.value:
        this.databaseTemplateComponent.configParams = {
          activeIndex: DataMap.Resource_Type.ObjectSet.value,
          tableCols: [],
          tableOpts: [
            'register',
            'protect',
            'modifyProtect',
            'removeProtection',
            'activeProtection',
            'deactiveProtection',
            'recovery',
            'manualBackup',
            'deleteResource',
            'modify'
          ],
          registerComponent: RegisterObjectComponent
        };
        this.databaseTemplateComponent.initConfig();
        return getTableOptsItems(
          this.databaseTemplateComponent.optItems,
          item,
          this.databaseTemplateComponent
        );
      case DataMap.Resource_Type.volume.value:
        this.databaseTemplateComponent.configParams = {
          activeIndex: DataMap.Resource_Type.volume.value,
          tableCols: [],
          tableOpts: [
            'register',
            'protect',
            'modifyProtect',
            'removeProtection',
            'activeProtection',
            'deactiveProtection',
            'recovery',
            'manualBackup',
            'modify',
            'deleteResource'
          ],
          registerComponent: CreateVolumeComponent
        };
        this.databaseTemplateComponent.initConfig();
        return getTableOptsItems(
          this.databaseTemplateComponent.optItems,
          item,
          this.databaseTemplateComponent
        );
      case DataMap.Resource_Type.APSCloudServer.value:
      case DataMap.Resource_Type.APSResourceSet.value:
      case DataMap.Resource_Type.APSZone.value:
      case DataMap.Resource_Type.cNwareCluster.value:
      case DataMap.Resource_Type.cNwareHost.value:
      case DataMap.Resource_Type.cNwareVm.value:
      case DataMap.Resource_Type.hyperVCluster.value:
      case DataMap.Resource_Type.hyperVHost.value:
      case DataMap.Resource_Type.hyperVVm.value:
      case DataMap.Resource_Type.nutanixCluster.value:
      case DataMap.Resource_Type.nutanixHost.value:
      case DataMap.Resource_Type.nutanixVm.value:
        this.baseTableComponent.subType = item.sub_type;
        this.baseTableComponent.initConfig();
        return getTableOptsItems(
          this.baseTableComponent.optItems,
          item,
          this.baseTableComponent
        );
    }
  }

  protect(datas, action: ProtectResourceAction, header?: string, refreshData?) {
    this.protectedResourceApiService
      .ShowResource({
        resourceId: datas[0].uuid
      })
      .subscribe(item => {
        if (
          item.subType === DataMap.Resource_Type.NASShare.value &&
          !item.extendInfo?.ip
        ) {
          this.messageService.error(
            this.i18n.get('protection_global_search_fqdn_ip_desc_label'),
            {
              lvShowCloseButton: true,
              lvMessageKey: 'nasIpNotExistMesageKey'
            }
          );
          return;
        }

        const data = assign(item, {
          sub_type: item.subType,
          protocol: item.extendInfo?.protocol,
          sla_id: item.protectedObject?.slaId
        });

        this.protectService.openProtectModal(
          item.subType === DataMap.Resource_Type.NASShare.value
            ? ProtectResourceCategory.NASShare
            : item.subType,
          action,
          {
            width: 780,
            data,
            onOK: () => {
              this.getResources();
            },
            restoreWidth: params => this.getResourceDetail(params)
          }
        );
      });
  }

  manualBackup(datas) {
    if (size(datas) > 1) {
      each(datas, item => {
        assign(item, {
          host_ip: item.environment_endpoint,
          resource_id: item.uuid,
          resource_type: datas[0].sub_type
        });
      });
      this.takeManualBackupService.batchExecute(datas, () =>
        this.getResources()
      );
    } else {
      assign(datas[0], {
        host_ip: datas[0].environment_endpoint,
        resource_id: datas[0].uuid,
        resource_type: datas[0].sub_type
      });
      this.takeManualBackupService.execute(datas[0], () => this.getResources());
    }
  }

  getResourceDetail(item) {
    this.protectedResourceApiService
      .ShowResource({
        resourceId: item.uuid
      })
      .subscribe(res => {
        extendSlaInfo(res);
        const params = {
          pageSize: CommonConsts.PAGE_SIZE,
          pageNo: CommonConsts.PAGE_START,
          conditions: JSON.stringify({ uuid: item.uuid })
        };

        let optItems = [];
        switch (item.sub_type) {
          case DataMap.Resource_Type.UBackupAgent.value:
          case DataMap.Resource_Type.ABBackupClient.value:
          case DataMap.Resource_Type.VMBackupAgent.value:
          case DataMap.Resource_Type.DBBackupAgent.value:
            this.clientManagerApiService
              .queryAgentListInfoUsingGET(params)
              .subscribe(res => {
                const host: any = first(res.records) || {};
                if (!isEmpty(host)) {
                  const _trustworthiness = get(host, [
                    'extendInfo',
                    'trustworthiness'
                  ]);
                  assign(host, {
                    sub_type: host.subType,
                    protection_status: host.protectionStatus,
                    link_status: Number(host.linkStatus),
                    os_type: host.osType,
                    authorized_user: host.authorizedUser,
                    trustworthiness: isNil(_trustworthiness)
                      ? false
                      : JSON.parse(_trustworthiness)
                  });
                  optItems = this.hostComponent.getOptItems(host);
                }
                this.openDetail(host, optItems);
              });
            break;
          case DataMap.Resource_Type.oracle.value:
          case DataMap.Resource_Type.oracleCluster.value:
            this.protectedResourceApiService
              .ShowResource({ resourceId: item.uuid })
              .subscribe(res => {
                extendSlaInfo(res);
                assign(res, {
                  sub_type: res.subType,
                  link_status: res.extendInfo?.linkStatus,
                  verify_status: res.extendInfo?.verify_status === 'true',
                  ip: item.path || item.endpoint
                });
                optItems = this.oracleComponent.getOptItems(res || {});
                this.openDetail(res, optItems);
              });
            break;
          case DataMap.Resource_Type.LocalFileSystem.value:
            this.doradoFileSystemComponent.initConfig();
            optItems = this.doradoFileSystemComponent.optItems;
            this.detailService.openDetailModal(res.subType, {
              data: { ...res, optItems }
            });
            break;
          case DataMap.Resource_Type.clusterComputeResource.value:
            this.vmListComponent.tab = {
              id: ResourceType.CLUSTER,
              resType: ResourceType.VM
            };
            this.vmListComponent.viewDetail(item);
            break;
          case DataMap.Resource_Type.hostSystem.value:
            this.vmListComponent.tab = {
              id: ResourceType.HOST,
              resType: ResourceType.VM
            };
            this.vmListComponent.viewDetail(item);
            break;
          case DataMap.Resource_Type.virtualMachine.value:
            this.virtualResourceService
              .queryResourcesV1VirtualResourceGet(params)
              .subscribe(res => {
                this.vmListComponent.tab = {
                  id: ResourceType.VM,
                  resType: ResourceType.VM
                };
                this.vmListComponent.viewDetail(first(res.items) || {});
              });
            break;
          case DataMap.Resource_Type.msHostSystem.value:
            this.vmListComponent.tab = {
              id: ResourceType.HOST,
              resType: ResourceType.HYPERV
            };
            this.vmListComponent.viewDetail(item);
            break;
          case DataMap.Resource_Type.NASFileSystem.value:
          case DataMap.Resource_Type.ndmp.value:
            this.getNasDetails(item);
            break;
          case DataMap.Resource_Type.NASShare.value:
            this.getNasDetails(item);
            break;
          case DataMap.Resource_Type.HDFSFileset.value:
            this.hdfsFilesetsComponent.getResourceDetail(item);
            break;
          case DataMap.Resource_Type.HBaseBackupSet.value:
            this.hbaseBackupSetComponent.getResourceDetail(item);
            break;
          case DataMap.Resource_Type.Kubernetes.value:
            this.kubernetesClusterComponent.getResourceDetail(item);
            break;
          case DataMap.Resource_Type.KubernetesNamespace.value:
          case DataMap.Resource_Type.KubernetesStatefulset.value:
            assign(res, {
              sub_type: res.subType
            });
            this.kubernetesComponent.resourceSubType = res.subType;
            this.kubernetesComponent.columns = [];
            this.kubernetesComponent.initConfig();
            this.kubernetesComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.kubernetesClusterCommon.value:
            this.kubernetesClusterComponent.subType =
              DataMap.Resource_Type.kubernetesClusterCommon.value;
            this.kubernetesClusterComponent.getResourceDetail(item);
            break;
          case DataMap.Resource_Type.kubernetesNamespaceCommon.value:
          case DataMap.Resource_Type.kubernetesDatasetCommon.value:
            assign(res, {
              sub_type: res.subType
            });
            this.kubernetesComponent.resourceSubType = res.subType;
            this.kubernetesComponent.columns = [];
            this.kubernetesComponent.initConfig();
            this.kubernetesComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.Redis.value:
            assign(res, {
              sub_type: res.subType
            });
            this.redisClusterComponent.initConfig();
            this.redisClusterComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.PostgreSQLCluster.value:
            assign(res, {
              sub_type: res.subType
            });
            this.postgreClusterComponent.initConfig();
            this.postgreClusterComponent.getDetail(res);
            break;
          case DataMap.Resource_Type.PostgreSQLInstance.value:
          case DataMap.Resource_Type.PostgreSQLClusterInstance.value:
            assign(res, {
              sub_type: res.subType
            });
            this.postgreInstanceComponent.initConfig();
            this.postgreInstanceComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.KingBaseCluster.value:
            assign(res, {
              sub_type: res.subType
            });
            this.kingBaseClusterComponent.initConfig();
            this.kingBaseClusterComponent.getDetail(res);
            break;
          case DataMap.Resource_Type.KingBaseInstance.value:
          case DataMap.Resource_Type.KingBaseClusterInstance.value:
            assign(res, {
              sub_type: res.subType
            });
            this.kingBaseInstanceComponent.initConfig();
            this.kingBaseInstanceComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.FusionCompute.value:
          case DataMap.Resource_Type.fusionOne.value:
            assign(res, {
              sub_type: res.subType
            });
            this.fcListComponent.viewDetail(res);
            break;
          case DataMap.Resource_Type.HCSCloudHost.value:
          case DataMap.Resource_Type.HCSProject.value:
          case DataMap.Resource_Type.HCSTenant.value:
            assign(res, {
              sub_type: res.subType
            });
            this.hcsListComponent.viewDetail(res);
            break;
          case DataMap.Resource_Type.fileset.value:
            this.filesetListComponent.getOptsItems;
            optItems = this.filesetListComponent.optItems;
            this.detailService.openDetailModal(res.subType, {
              data: { ...res, optItems }
            });
            break;
          case DataMap.Resource_Type.ElasticsearchBackupSet.value:
          case DataMap.Resource_Type.HiveBackupSet.value:
            this.hbaseBackupSetComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.MySQLCluster.value:
            assign(item, { clusterType: item.extendInfo?.clusterType });
            this.mysqlClusterComponent.getDetail(item);
            break;
          case DataMap.Resource_Type.MySQLClusterInstance.value:
          case DataMap.Resource_Type.MySQLInstance.value:
            assign(item, { auth_status: item.extendInfo?.linkStatus });
            this.mysqlListComponent.getResourceDetail(item);
            break;
          case DataMap.Resource_Type.MySQLDatabase.value:
            assign(item, { auth_status: item.extendInfo?.authStatus });
            this.mysqlListComponent.getResourceDetail(item);
            break;
          case DataMap.Resource_Type.ClickHouse.value:
            if (item.type === ResourceType.TABLE_SET) {
              this.clickHouseTablesetComonent.getResourceDetail(res);
            } else if (item.type === ResourceType.DATABASE) {
              this.clickhouseDatabaseComponent.getResourceDetail(res);
            } else if (item.type === ResourceType.CLUSTER) {
              this.clickHouseClusterComonent.getResourceDetail(res);
            }
            break;
          case DataMap.Resource_Type.ExchangeSingle.value:
          case DataMap.Resource_Type.ExchangeGroup.value:
            assign(res, {
              sub_type: res.subType
            });
            this.databaseTemplateComponent.configParams = {
              activeIndex: DataMap.Resource_Type.Exchange.value,
              tableCols: [],
              tableOpts: [
                'protect',
                'modifyProtect',
                'removeProtection',
                'activeProtection',
                'deactiveProtection',
                'recovery',
                'manualBackup',
                'rescan',
                'connectivityTest',
                'modify',
                'deleteResource'
              ],
              registerComponent: RegisterGroupComponent
            };
            this.databaseTemplateComponent.initConfig();
            this.databaseTemplateComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.ExchangeDataBase.value:
          case DataMap.Resource_Type.ExchangeEmail.value:
            assign(res, {
              sub_type: res.subType
            });
            if (item.sub_type === DataMap.Resource_Type.ExchangeEmail.value) {
              assign(res, {
                address: res.extendInfo?.PrimarySmtpAddress
              });
            }
            this.databaseTemplateComponent.configParams = {
              activeIndex: item.sub_type,
              tableCols: [],
              tableOpts: [
                'protect',
                'modifyProtect',
                'removeProtection',
                'activeProtection',
                'deactiveProtection',
                'recovery',
                'manualBackup'
              ]
            };
            this.databaseTemplateComponent.initConfig();
            this.databaseTemplateComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.AntDBInstance.value:
          case DataMap.Resource_Type.AntDBClusterInstance.value:
            this.databaseTemplateComponent.configParams = {
              activeIndex: DataMap.Resource_Type.AntDB.value,
              tableCols: [],
              tableOpts: [
                'register',
                'protect',
                'modifyProtect',
                'removeProtection',
                'activeProtection',
                'deactiveProtection',
                'recovery',
                'manualBackup',
                'connectivityTest',
                'modify',
                'deleteResource'
              ],
              registerComponent: RegisterAntDBInstanceComponent
            };
            this.databaseTemplateComponent.initConfig();
            this.databaseTemplateComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.saphanaInstance.value:
            this.databaseTemplateComponent.configParams = {
              activeIndex: DataMap.Resource_Type.saphanaInstance.value,
              tableCols: [],
              tableOpts: [
                'register',
                'resourceAuth',
                'resourceReclaiming',
                'connectivityTest',
                'modify',
                'deleteResource'
              ],
              registerComponent: RegisterSaphanaInstanceComponent
            };
            this.databaseTemplateComponent.initConfig();
            this.databaseTemplateComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.saphanaDatabase.value:
            this.databaseTemplateComponent.configParams = {
              activeIndex: DataMap.Resource_Type.saphanaDatabase.value,
              tableCols: [],
              tableOpts: [
                'register',
                'protect',
                'modifyProtect',
                'removeProtection',
                'activeProtection',
                'deactiveProtection',
                'recovery',
                'manualBackup',
                'connectivityTest',
                'modify',
                'deleteResource'
              ],
              registerComponent: RegisterSaphanaDatabaseComponent
            };
            this.databaseTemplateComponent.initConfig();
            this.databaseTemplateComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.saponoracleDatabase.value:
            this.databaseTemplateComponent.configParams = {
              activeIndex: DataMap.Resource_Type.saponoracleDatabase.value,
              tableCols: [],
              tableOpts: [
                'register',
                'protect',
                'modifyProtect',
                'removeProtection',
                'activeProtection',
                'deactiveProtection',
                'recovery',
                'manualBackup',
                'connectivityTest',
                'modify',
                'deleteResource'
              ],
              registerComponent: SaponoracleRegisterDatabaseComponent
            };
            this.databaseTemplateComponent.initConfig();
            this.databaseTemplateComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.informixService.value:
            this.databaseTemplateComponent.configParams = {
              activeIndex: DataMap.Resource_Type.informixService.value,
              tableCols: [],
              tableOpts: [
                'register',
                'resourceAuth',
                'resourceReclaiming',
                'connectivityTest',
                'modify',
                'deleteResource'
              ],
              registerComponent: RegisterInformixServiceComponent
            };
            this.databaseTemplateComponent.initConfig();
            this.databaseTemplateComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.informixInstance.value:
          case DataMap.Resource_Type.informixClusterInstance.value:
            this.databaseTemplateComponent.configParams = {
              activeIndex: DataMap.Resource_Type.informixInstance.value,
              tableCols: [],
              tableOpts: [
                'register',
                'protect',
                'modifyProtect',
                'removeProtection',
                'activeProtection',
                'deactiveProtection',
                'recovery',
                'manualBackup',
                'connectivityTest',
                'modify',
                'deleteResource'
              ],
              registerComponent: RegisterInformixInstanceComponent
            };
            this.databaseTemplateComponent.initConfig();
            this.databaseTemplateComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.tdsqlCluster.value:
            this.databaseTemplateComponent.configParams = {
              activeIndex: DataMap.Resource_Type.tdsqlCluster.value,
              tableCols: [],
              tableOpts: [
                'register',
                'resourceAuth',
                'resourceReclaiming',
                'connectivityTest',
                'modify',
                'deleteResource'
              ],
              registerComponent: RegisterClusterComponent
            };
            this.databaseTemplateComponent.initConfig();
            this.databaseTemplateComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.tdsqlInstance.value:
            this.databaseTemplateComponent.configParams = {
              activeIndex: DataMap.Resource_Type.tdsqlInstance.value,
              tableCols: [],
              tableOpts: [
                'register',
                'protect',
                'modifyProtect',
                'removeProtection',
                'activeProtection',
                'deactiveProtection',
                'recovery',
                'manualBackup',
                'connectivityTest',
                'modify',
                'deleteResource'
              ],
              registerComponent: RegisterInstanceComponent
            };
            this.databaseTemplateComponent.initConfig();
            this.databaseTemplateComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.tdsqlDistributedInstance.value:
            this.databaseTemplateComponent.configParams = {
              activeIndex: DataMap.Resource_Type.tdsqlDistributedInstance.value,
              tableCols: [],
              tableOpts: [
                'register',
                'protect',
                'modifyProtect',
                'removeProtection',
                'activeProtection',
                'deactiveProtection',
                'recovery',
                'manualBackup',
                'connectivityTest',
                'modify',
                'deleteResource'
              ],
              registerComponent: RegisterDistributedInstanceComponent
            };
            this.databaseTemplateComponent.initConfig();
            this.databaseTemplateComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.OceanBaseCluster.value:
            this.databaseTemplateComponent.configParams = {
              activeIndex: DataMap.Resource_Type.OceanBaseCluster.value,
              tableCols: [],
              tableOpts: [
                'protect',
                'modifyProtect',
                'removeProtection',
                'activeProtection',
                'deactiveProtection',
                'recovery',
                'manualBackup',
                'connectivityTest',
                'modify',
                'deleteResource'
              ],
              registerComponent: RegisterOceanBaseComponent
            };
            this.databaseTemplateComponent.initConfig();
            this.databaseTemplateComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.OceanBaseTenant.value:
            this.databaseTemplateComponent.configParams = {
              activeIndex: DataMap.Resource_Type.OceanBaseTenant.value,
              tableCols: [],
              tableOpts: [
                'protect',
                'modifyProtect',
                'removeProtection',
                'activeProtection',
                'deactiveProtection',
                'recovery',
                'manualBackup',
                'modify',
                'deleteResource'
              ],
              registerComponent: RegisterTenantComponent
            };
            this.databaseTemplateComponent.initConfig();
            this.databaseTemplateComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.tidbCluster.value:
            this.databaseTemplateComponent.configParams = {
              activeIndex: DataMap.Resource_Type.tidbCluster.value,
              tableCols: [],
              tableOpts: [
                'protect',
                'modifyProtect',
                'removeProtection',
                'activeProtection',
                'deactiveProtection',
                'recovery',
                'manualBackup',
                'modify',
                'deleteResource'
              ],
              registerComponent: RegisterTidbClusterComponent
            };
            this.databaseTemplateComponent.initConfig();
            this.databaseTemplateComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.tidbDatabase.value:
            this.databaseTemplateComponent.configParams = {
              activeIndex: DataMap.Resource_Type.tidbDatabase.value,
              tableCols: [],
              tableOpts: [
                'protect',
                'modifyProtect',
                'removeProtection',
                'activeProtection',
                'deactiveProtection',
                'recovery',
                'manualBackup',
                'modify',
                'deleteResource'
              ],
              registerComponent: RegisterTidbDatabaseComponent
            };
            this.databaseTemplateComponent.initConfig();
            this.databaseTemplateComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.tidbTable.value:
            this.databaseTemplateComponent.configParams = {
              activeIndex: DataMap.Resource_Type.tidbTable.value,
              tableCols: [],
              tableOpts: [
                'protect',
                'modifyProtect',
                'removeProtection',
                'activeProtection',
                'deactiveProtection',
                'recovery',
                'manualBackup',
                'modify',
                'deleteResource'
              ],
              registerComponent: RegisterTidbTableComponent
            };
            this.databaseTemplateComponent.initConfig();
            this.databaseTemplateComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.gaussdbForOpengauss.value:
            this.instanceDatabaseComponent.activeIndex =
              DataMap.Resource_Type.gaussdbForOpengauss.value;
            this.instanceDatabaseComponent.initConfig();
            this.instanceDatabaseComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.gaussdbForOpengaussProject.value:
            this.instanceDatabaseComponent.activeIndex =
              DataMap.Resource_Type.gaussdbForOpengaussProject.value;
            this.instanceDatabaseComponent.initConfig();
            this.instanceDatabaseComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.SQLServerCluster.value:
            this.instanceDatabaseComponent.activeIndex =
              DataMap.Resource_Type.SQLServerCluster.value;
            this.instanceDatabaseComponent.initConfig();
            this.instanceDatabaseComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.SQLServerClusterInstance.value:
          case DataMap.Resource_Type.SQLServerInstance.value:
            assign(res, {
              clusterOrHostName: res.environment?.name,
              nodeIpAddress:
                res.subType ===
                DataMap.Resource_Type.SQLServerClusterInstance.value
                  ? res.path
                  : res.environment?.endpoint
            });
            this.instanceDatabaseComponent.activeIndex =
              DataMap.Resource_Type.SQLServerInstance.value;
            this.instanceDatabaseComponent.initConfig();
            this.instanceDatabaseComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.SQLServerGroup.value:
            assign(res, {
              clusterOrHostName: res.parentName
            });
            this.instanceDatabaseComponent.activeIndex =
              DataMap.Resource_Type.SQLServerGroup.value;
            this.instanceDatabaseComponent.initConfig();
            this.instanceDatabaseComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.SQLServerDatabase.value:
            assign(res, {
              ownedInstance: res.extendInfo?.instanceName,
              clusterOrHostName: res.extendInfo?.hostName
            });
            this.instanceDatabaseComponent.activeIndex =
              DataMap.Resource_Type.SQLServerDatabase.value;
            this.instanceDatabaseComponent.initConfig();
            this.instanceDatabaseComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.DWS_Cluster.value:
            this.instanceDatabaseComponent.activeIndex =
              DataMap.Resource_Type.DWS_Cluster.value;
            this.instanceDatabaseComponent.initConfig();
            this.instanceDatabaseComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.DWS_Database.value:
            assign(res, {
              clusterOrHostName: res.environment?.name
            });
            this.instanceDatabaseComponent.activeIndex =
              DataMap.Resource_Type.DWS_Database.value;
            this.instanceDatabaseComponent.initConfig();
            this.instanceDatabaseComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.DWS_Schema.value:
            assign(res, {
              clusterOrHostName: res.environment?.name,
              database: res.parentName
            });
            this.instanceDatabaseComponent.activeIndex =
              DataMap.Resource_Type.DWS_Schema.value;
            this.instanceDatabaseComponent.initConfig();
            this.instanceDatabaseComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.DWS_Table.value:
            assign(res, {
              clusterOrHostName: res.environment?.name,
              database: res.parentName
            });
            this.instanceDatabaseComponent.activeIndex =
              DataMap.Resource_Type.DWS_Table.value;
            this.instanceDatabaseComponent.initConfig();
            this.instanceDatabaseComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.goldendbCluter.value:
            assign(res, {
              nodeIpAddress: get(res, 'endpoint')
            });
            this.instanceDatabaseComponent.activeIndex =
              DataMap.Resource_Type.goldendbCluter.value;
            this.instanceDatabaseComponent.initConfig();
            this.instanceDatabaseComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.goldendbInstance.value:
            assign(res, {
              nodeIpAddress: item.environment?.endpoint,
              linkStatus: item.extendInfo?.linkStatus
            });
            this.instanceDatabaseComponent.activeIndex =
              DataMap.Resource_Type.goldendbInstance.value;
            this.instanceDatabaseComponent.initConfig();
            this.instanceDatabaseComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.generalDatabase.value:
            assign(res, {
              sub_type: res.subType
            });
            this.generalDatabaseComponent.sourceType =
              DataMap.Resource_Type.generalDatabase.value;
            this.generalDatabaseComponent.initConfig();
            this.generalDatabaseComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.dbTwoCluster.value:
            assign(res, {
              clusterType: res?.extendInfo?.clusterType
            });
            this.instanceDatabaseComponent.activeIndex =
              DataMap.Resource_Type.dbTwoCluster.value;
            this.instanceDatabaseComponent.initConfig();
            this.instanceDatabaseComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.dbTwoClusterInstance.value:
          case DataMap.Resource_Type.dbTwoInstance.value:
            assign(res, {
              clusterOrHostName: has(res, 'environment.extendInfo.clusterType')
                ? res.environment?.name
                : `${res.environment?.name}(${res.environment?.endpoint})`,
              linkStatus: res.extendInfo?.linkStatus
            });
            this.instanceDatabaseComponent.activeIndex =
              DataMap.Resource_Type.dbTwoInstance.value;
            this.instanceDatabaseComponent.initConfig();
            this.instanceDatabaseComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.dbTwoDatabase.value:
            assign(res, {
              clusterOrHostName: has(res, 'environment.extendInfo.clusterType')
                ? res.environment?.name
                : `${res.environment?.name}(${res.environment?.endpoint})`,
              ownedInstance: res.parentName
            });
            this.instanceDatabaseComponent.activeIndex =
              DataMap.Resource_Type.dbTwoDatabase.value;
            this.instanceDatabaseComponent.initConfig();
            this.instanceDatabaseComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.dbTwoTableSet.value:
            assign(res, {
              clusterOrHostName: has(res, 'environment.extendInfo.clusterType')
                ? res.environment?.name
                : `${res.environment?.name}(${res.environment?.endpoint})`,
              ownedInstance: res.extendInfo?.instance,
              database: res.parentName
            });
            this.instanceDatabaseComponent.activeIndex =
              DataMap.Resource_Type.dbTwoTableSet.value;
            this.instanceDatabaseComponent.initConfig();
            this.instanceDatabaseComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.OpenGauss.value:
            assign(res, {
              deployType: item.extendInfo.deployType,
              clusterVersion: item.extendInfo.clusterVersion
            });
            this.openGaussClusterComponent.initConfig();
            this.openGaussClusterComponent.getDetail(res);
            break;
          case DataMap.Resource_Type.OpenGauss_instance.value:
          case DataMap.Resource_Type.OpenGauss_database.value:
            this.openGaussComponent.initConfig();
            this.openGaussComponent.getResourceDetail(assign(item, res));
            break;
          case DataMap.Resource_Type.Dameng_cluster.value:
          case DataMap.Resource_Type.Dameng_singleNode.value:
            this.damengComponent.initConfig();
            this.damengComponent.getResourceDetail(assign(item, res));
            break;
          case DataMap.Resource_Type.gaussdbTSingle.value:
          case DataMap.Resource_Type.GaussDB_T.value:
            this.nasSharedComponent.subType =
              DataMap.Resource_Type.GaussDB_T.value;
            this.nasSharedComponent.initConfig();
            this.nasSharedComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.HBase.value:
            this.clustersComponent.resSubType =
              DataMap.Resource_Type.HBase.value;
            this.clustersComponent.initConfig();
            this.clustersComponent.getDetail(res);
            break;
          case DataMap.Resource_Type.Hive.value:
            this.clustersComponent.resSubType =
              DataMap.Resource_Type.Hive.value;
            this.clustersComponent.initConfig();
            this.clustersComponent.getDetail(res);
            break;
          case DataMap.Resource_Type.Elasticsearch.value:
            this.clustersComponent.resSubType =
              DataMap.Resource_Type.Elasticsearch.value;
            this.clustersComponent.initConfig();
            this.clustersComponent.getDetail(res);
            break;
          case DataMap.Resource_Type.openStackProject.value:
          case DataMap.Resource_Type.openStackCloudServer.value:
            this.openstackListComponent.resType = item.sub_type;
            this.openstackListComponent.initConfig();
            this.openstackListComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.MongodbSingleInstance.value:
          case DataMap.Resource_Type.MongodbClusterInstance.value:
            assign(res, {
              sub_type: res.subType
            });
            this.kubernetesComponent.resourceSubType =
              DataMap.Resource_Type.MongoDB.value;
            this.kubernetesComponent.extraConfig = this.mongodbComponent.extraConfig;
            this.kubernetesComponent.columns = [];
            this.kubernetesComponent.initConfig();
            this.kubernetesComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.lightCloudGaussdbProject.value:
            this.databaseTemplateComponent.configParams = {
              activeIndex: DataMap.Resource_Type.lightCloudGaussdbProject.value,
              tableCols: [],
              tableOpts: [
                'register',
                'resourceAuth',
                'resourceReclaiming',
                'rescan',
                'connectivityTest',
                'modify',
                'deleteResource'
              ],
              registerComponent: RegisterLightCloudGuassdbComponent
            };
            this.databaseTemplateComponent.initConfig();
            this.databaseTemplateComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.lightCloudGaussdbInstance.value:
            this.databaseTemplateComponent.configParams = {
              activeIndex:
                DataMap.Resource_Type.lightCloudGaussdbInstance.value,
              tableCols: [],
              tableOpts: [
                'protect',
                'modifyProtect',
                'removeProtection',
                'activeProtection',
                'deactiveProtection',
                'recovery',
                'manualBackup'
              ]
            };
            this.databaseTemplateComponent.initConfig();
            this.databaseTemplateComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.ObjectStorage.value:
            this.objectStorageComponent.initConfig();
            this.objectStorageComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.ObjectSet.value:
            this.databaseTemplateComponent.configParams = {
              activeIndex: DataMap.Resource_Type.ObjectSet.value,
              tableCols: [],
              tableOpts: [
                'register',
                'protect',
                'modifyProtect',
                'removeProtection',
                'activeProtection',
                'deactiveProtection',
                'recovery',
                'manualBackup',
                'deleteResource',
                'modify'
              ]
            };
            this.databaseTemplateComponent.initConfig();
            this.databaseTemplateComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.volume.value:
            assign(res, {
              sub_type: res.subType
            });
            this.databaseTemplateComponent.configParams = {
              activeIndex: DataMap.Resource_Type.volume.value,
              tableCols: [],
              tableOpts: [
                'protect',
                'modifyProtect',
                'removeProtection',
                'activeProtection',
                'deactiveProtection',
                'recovery',
                'manualBackup',
                'modify',
                'deleteResource'
              ],
              registerComponent: CreateVolumeComponent
            };
            this.databaseTemplateComponent.initConfig();
            this.databaseTemplateComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.APSCloudServer.value:
          case DataMap.Resource_Type.APSZone.value:
          case DataMap.Resource_Type.APSResourceSet.value:
            this.baseTableComponent.initConfig();
            this.baseTableComponent.getResourceDetail(res);
            break;
          case DataMap.Resource_Type.cNwareCluster.value:
          case DataMap.Resource_Type.cNwareHost.value:
          case DataMap.Resource_Type.cNwareVm.value:
          case DataMap.Resource_Type.hyperVCluster.value:
          case DataMap.Resource_Type.hyperVHost.value:
          case DataMap.Resource_Type.hyperVVm.value:
          case DataMap.Resource_Type.nutanixCluster.value:
          case DataMap.Resource_Type.nutanixHost.value:
          case DataMap.Resource_Type.nutanixVm.value:
            this.baseTableComponent.subType = res.subType;
            this.baseTableComponent.initConfig();
            this.baseTableComponent.getResourceDetail(res);
            break;
        }
      });
  }

  openDetail(res: any, optItems: any[]) {
    if (isEmpty(res)) {
      this.message.error(this.i18n.get('common_resource_not_exist_label'), {
        lvShowCloseButton: true,
        lvMessageKey: 'resNotExistMesageKey'
      });
      return;
    }

    if (
      includes(
        mapValues(this.drawModalService.modals, 'key'),
        'slaDetailModalKey'
      )
    ) {
      this.drawModalService.destroyModal('slaDetailModalKey');
    }

    if (
      !includes(
        [
          DataMap.Resource_Type.clusterComputeResource.value,
          DataMap.Resource_Type.hostSystem.value,
          DataMap.Resource_Type.virtualMachine.value
        ],
        res.subType
      )
    ) {
      if (
        includes(
          [
            DataMap.Resource_Type.UBackupAgent.value,
            DataMap.Resource_Type.DBBackupAgent.value,
            DataMap.Resource_Type.VMBackupAgent.value
          ],
          res.subType
        )
      ) {
        this.detailService.openDetailModal(
          DataMap.Resource_Type.ABBackupClient.value,
          {
            data: { ...res, optItems }
          }
        );
      } else {
        this.detailService.openDetailModal(res.subType, {
          data: { ...res, optItems }
        });
      }
    }
  }

  getNameLink(item) {
    if (
      includes(
        [
          DataMap.Resource_Type.FusionCompute.value,
          DataMap.Resource_Type.fusionOne.value
        ],
        item.sub_type
      )
    ) {
      return includes(this.resourceTypeValues, item.type);
    }
    if (
      item.sub_type === DataMap.Resource_Type.ClickHouse.value &&
      includes(['Table', 'Node'], item.type)
    ) {
      return false;
    }
    return (
      (includes(this.resourceTypeValues, item.sub_type) &&
        !(item.environment_is_cluster === 'True' && item.type === 'Host')) ||
      this.cookieService.isCloudBackup
    );
  }

  // 不支持保护的应用保护状态需要展示为'--'
  getProtectionStatus(item) {
    if (
      includes(
        [
          DataMap.Resource_Type.tdsqlCluster.value,
          DataMap.Resource_Type.goldendbCluter.value,
          DataMap.Resource_Type.PostgreSQLCluster.value,
          DataMap.Resource_Type.UBackupAgent.value,
          DataMap.Resource_Type.SQLServerCluster.value,
          DataMap.Resource_Type.MySQLCluster.value,
          DataMap.Resource_Type.dbTwoCluster.value,
          DataMap.Resource_Type.dbTwoClusterInstance.value,
          DataMap.Resource_Type.dbTwoInstance.value,
          DataMap.Resource_Type.informixService.value,
          DataMap.Resource_Type.Elasticsearch.value,
          DataMap.Resource_Type.HBase.value,
          DataMap.Resource_Type.HDFS.value,
          DataMap.Resource_Type.Hive.value,
          DataMap.Resource_Type.hyperVCluster.value,
          DataMap.Resource_Type.hyperVScvmm.value,
          DataMap.Resource_Type.Kubernetes.value,
          DataMap.Resource_Type.saphanaInstance.value,
          DataMap.Resource_Type.gaussdbForOpengaussProject.value,
          DataMap.Resource_Type.KingBaseCluster.value,
          DataMap.Resource_Type.OpenGauss.value,
          DataMap.Resource_Type.ClickHouse.value
        ],
        item.sub_type || item.subType
      )
    ) {
      return false;
    }
    return true;
  }

  getComponent() {
    this.hostComponent = new HostComponent(
      this.router,
      this.fb,
      this.i18n,
      this.cookieService,
      this.slaService,
      this.messageBox,
      this.hostApiService,
      this.switchService,
      this.detailService,
      this.protectService,
      this.dataMapService,
      this.warningMessageService,
      this.projectedObjectApiService,
      this.takeManualBackupService,
      this.environmentsApiService,
      this.drawModalService,
      this.rememberColumnsService,
      this.globalService,
      this.virtualScroll,
      this.cdr,
      this.snmpApiService,
      this.message,
      this.componentRestApiService,
      this.protectedResourceApiService,
      this.batchOperateService,
      this.protectedEnvironmentApiService,
      this.exportFilesApi,
      this.clientManagerApiService,
      this.baseUtilService,
      this.hcsResourceService,
      this.opHcsServiceApiService,
      this.setResourceTagService
    );
    this.filesetListComponent = new FilesetComponent(
      this.i18n,
      this.slaService,
      this.detailService,
      this.drawModalService,
      this.protectService,
      this.dataMapService,
      this.globalService,
      this.warningMessageService,
      this.projectedObjectApiService,
      this.takeManualBackupService,
      this.batchOperateService,
      this.cookieService,
      this.rememberColumnsService,
      this.virtualScroll,
      this.cdr,
      this.warningBatchConfirmsService,
      this.protectedResourceApiService,
      this.setResourceTagService
    );
    this.oracleComponent = new OracleDatabaseListComponent(
      this.i18n,
      this.dataMapService,
      this.drawModalService,
      this.protectService,
      this.warningMessageService,
      this.projectedObjectApiService,
      this.takeManualBackupService,
      this.databasesService,
      this.slaService,
      this.detailService,
      this.cookieService,
      this.rememberColumnsService,
      this.globalService,
      this.virtualScroll,
      this.cdr,
      this.warningBatchConfirmsService,
      this.protectedResourceApiService,
      this.batchOperateService,
      this.messageService,
      this.setResourceTagService
    );
    this.vmListComponent = new VmListComponent(
      this.i18n,
      this.dataMapService,
      this.drawModalService,
      this.detailService,
      this.protectService,
      this.takeManualBackupService,
      this.projectedObjectApiService,
      this.virtualResourceService,
      this.warningMessageService,
      this.slaService,
      this.cookieService,
      this.rememberColumnsService,
      this.virtualScroll,
      this.warningBatchConfirmsService,
      this.cdr,
      this.setResourceTagService
    );
    this.doradoFileSystemComponent = new DoradoFileSystemComponent(
      this.i18n,
      this.slaService,
      this.dataMapService,
      this.protectService,
      this.message,
      this.drawModalService,
      this.virtualScroll,
      this.cookieService,
      this.cdr,
      this.detailService,
      this.infoMessageService,
      this.takeManualBackupService,
      this.protectedResourceApiService,
      this.protectedEnvironmentApiService,
      this.setResourceTagService,
      this.getLabelOptionsService
    );
    this.nasSharedComponent = new NasSharedComponent(
      this.i18n,
      this.cdr,
      this.slaService,
      this.dataMapService,
      this.protectService,
      this.message,
      this.drawModalService,
      this.warningMessageService,
      this.virtualScroll,
      this.detailService,
      this.batchOperateService,
      this.takeManualBackupService,
      this.protectedResourceApiService,
      this.setResourceTagService,
      this.getLabelOptionsService,
      this.appUtilsService
    );
    this.hdfsFilesetsComponent = new FilesetsComponent(
      this.i18n,
      this.cdr,
      this.slaService,
      this.dataMapService,
      this.protectService,
      this.message,
      this.drawModalService,
      this.virtualScroll,
      this.detailService,
      this.batchOperateService,
      this.warningMessageService,
      this.takeManualBackupService,
      this.protectedResourceApiService,
      this.setResourceTagService,
      this.getLabelOptionsService,
      this.appUtilsService
    );
    this.hbaseBackupSetComponent = new BackupSetComponent(
      this.i18n,
      this.cdr,
      this.slaService,
      this.dataMapService,
      this.protectService,
      this.drawModalService,
      this.warningMessageService,
      this.virtualScroll,
      this.detailService,
      this.batchOperateService,
      this.takeManualBackupService,
      this.protectedResourceApiService,
      this.setResourceTagService,
      this.getLabelOptionsService,
      this.appUtilsService
    );
    this.kubernetesClusterComponent = new ClusterComponent(
      this.i18n,
      this.cdr,
      this.dataMapService,
      this.messageService,
      this.drawModalService,
      this.batchOperateService,
      this.warningMessageService,
      this.virtualScroll,
      this.protectedEnvironmentApiService,
      this.protectedResourceApiService,
      this.setResourceTagService,
      this.getLabelOptionsService
    );
    this.kubernetesComponent = new BaseTemplateComponent(
      this.i18n,
      this.slaService,
      this.cookieService,
      this.dataMapService,
      this.protectService,
      this.messageService,
      this.drawModalService,
      this.virtualScroll,
      this.detailService,
      this.batchOperateService,
      this.warningMessageService,
      this.takeManualBackupService,
      this.protectedResourceApiService,
      this.setResourceTagService,
      this.getLabelOptionsService
    );
    this.redisClusterComponent = new RedisShowComponent(
      this.i18n,
      this.cdr,
      this.slaService,
      this.dataMapService,
      this.protectService,
      this.messageService,
      this.drawModalService,
      this.warningMessageService,
      this.virtualScroll,
      this.detailService,
      this.batchOperateService,
      this.takeManualBackupService,
      this.protectedResourceApiService,
      this.setResourceTagService,
      this.getLabelOptionsService,
      this.appUtilsService
    );
    this.postgreClusterComponent = new PostgreClusterComponent(
      this.i18n,
      this.cdr,
      this.dataMapService,
      this.drawModalService,
      this.virtualScroll,
      this.batchOperateService,
      this.warningMessageService,
      this.protectedResourceApiService,
      this.setResourceTagService,
      this.getLabelOptionsService
    );
    this.postgreInstanceComponent = new PostgreInstanceDatabaseComponent(
      this.i18n,
      this.cdr,
      this.slaService,
      this.dataMapService,
      this.protectService,
      this.messageService,
      this.drawModalService,
      this.warningMessageService,
      this.virtualScroll,
      this.detailService,
      this.batchOperateService,
      this.takeManualBackupService,
      this.protectedResourceApiService,
      this.setResourceTagService,
      this.getLabelOptionsService,
      this.appUtilsService
    );
    this.kingBaseClusterComponent = new KingBaseClusterComponent(
      this.i18n,
      this.cdr,
      this.dataMapService,
      this.drawModalService,
      this.virtualScroll,
      this.cookieService,
      this.batchOperateService,
      this.warningMessageService,
      this.protectedResourceApiService,
      this.setResourceTagService,
      this.getLabelOptionsService
    );
    this.kingBaseInstanceComponent = new KingBaseInstanceDatabaseComponent(
      this.i18n,
      this.cdr,
      this.slaService,
      this.dataMapService,
      this.protectService,
      this.messageService,
      this.drawModalService,
      this.warningMessageService,
      this.virtualScroll,
      this.detailService,
      this.batchOperateService,
      this.takeManualBackupService,
      this.protectedResourceApiService,
      this.setResourceTagService,
      this.getLabelOptionsService,
      this.appUtilsService
    );
    this.fcListComponent = new FusionListComponent(
      this.i18n,
      this.dataMapService,
      this.drawModalService,
      this.detailService,
      this.protectService,
      this.takeManualBackupService,
      this.projectedObjectApiService,
      this.warningMessageService,
      this.slaService,
      this.cookieService,
      this.rememberColumnsService,
      this.virtualScroll,
      this.warningBatchConfirmsService,
      this.setResourceTagService
    );
    this.hcsListComponent = new HuaWeiStackListComponent(
      this.i18n,
      this.dataMapService,
      this.drawModalService,
      this.detailService,
      this.protectService,
      this.takeManualBackupService,
      this.projectedObjectApiService,
      this.warningMessageService,
      this.slaService,
      this.cookieService,
      this.rememberColumnsService,
      this.virtualScroll,
      this.warningBatchConfirmsService,
      this.protectedResourceApiService,
      this.batchOperateService,
      this.messageService,
      this.setResourceTagService
    );
    this.mysqlClusterComponent = new MySQLClusterComponent(
      this.i18n,
      this.cdr,
      this.globalService,
      this.dataMapService,
      this.drawModalService,
      this.virtualScroll,
      this.batchOperateService,
      this.warningMessageService,
      this.protectedResourceApiService,
      this.setResourceTagService,
      this.appUtilsService,
      this.getLabelOptionsService
    );
    this.mysqlListComponent = new MySQLListComponent(
      this.i18n,
      this.cdr,
      this.slaService,
      this.dataMapService,
      this.protectService,
      this.messageService,
      this.drawModalService,
      this.warningMessageService,
      this.virtualScroll,
      this.detailService,
      this.batchOperateService,
      this.takeManualBackupService,
      this.protectedEnvironmentApiService,
      this.protectedResourceApiService,
      this.setResourceTagService,
      this.getLabelOptionsService
    );
    this.clickhouseDatabaseComponent = new ClickHouseDatabaseComponent(
      this.i18n,
      this.cdr,
      this.slaService,
      this.messageService,
      this.protectService,
      this.dataMapService,
      this.drawModalService,
      this.virtualScroll,
      this.detailService,
      this.takeManualBackupService,
      this.protectedResourceApiService,
      this.setResourceTagService,
      this.getLabelOptionsService,
      this.appUtilsService
    );
    this.clickHouseTablesetComonent = new ClickHouseTablesetComonent(
      this.i18n,
      this.protectService,
      this.dataMapService,
      this.virtualScroll,
      this.detailService,
      this.takeManualBackupService,
      this.protectedResourceApiService,
      this.batchOperateService,
      this.cdr,
      this.protectedEnvironmentApiService,
      this.messageService,
      this.warningMessageService,
      this.slaService,
      this.drawModalService,
      this.setResourceTagService,
      this.getLabelOptionsService,
      this.appUtilsService
    );
    this.clickHouseClusterComonent = new ClickHouseClusterComponent(
      this.i18n,
      this.cdr,
      this.messageService,
      this.globalService,
      this.dataMapService,
      this.virtualScroll,
      this.drawModalService,
      this.detailService,
      this.batchOperateService,
      this.warningMessageService,
      this.protectedEnvironmentApiService,
      this.protectedResourceApiService,
      this.setResourceTagService,
      this.getLabelOptionsService
    );
    this.instanceDatabaseComponent = new InstanceDatabaseComponent(
      this.i18n,
      this.cdr,
      this.slaService,
      this.dataMapService,
      this.protectService,
      this.messageService,
      this.drawModalService,
      this.warningMessageService,
      this.virtualScroll,
      this.detailService,
      this.batchOperateService,
      this.takeManualBackupService,
      this.protectedResourceApiService,
      this.protectedEnvironmentApiService,
      this.setResourceTagService,
      this.getLabelOptionsService
    );
    this.openGaussComponent = new OpenGaussComponent(
      this.i18n,
      this.slaService,
      this.dataMapService,
      this.protectService,
      this.virtualScroll,
      this.cookieService,
      this.detailService,
      this.takeManualBackupService,
      this.protectedResourceApiService,
      this.setResourceTagService,
      this.getLabelOptionsService
    );
    this.openGaussClusterComponent = new OpenGaussClusterComponent(
      this.i18n,
      this.messageService,
      this.cdr,
      this.cookieService,
      this.dataMapService,
      this.virtualScroll,
      this.drawModalService,
      this.batchOperateService,
      this.warningMessageService,
      this.protectedResourceApiService,
      this.protectedEnvironmentApiService,
      this.setResourceTagService,
      this.getLabelOptionsService
    );
    this.damengComponent = new DamengComponent(
      this.i18n,
      this.cdr,
      this.slaService,
      this.messageService,
      this.protectService,
      this.dataMapService,
      this.drawModalService,
      this.virtualScroll,
      this.detailService,
      this.batchOperateService,
      this.warningMessageService,
      this.takeManualBackupService,
      this.protectedResourceApiService,
      this.setResourceTagService,
      this.getLabelOptionsService,
      this.appUtilsService
    );
    this.clustersComponent = new ClustersComponent(
      this.i18n,
      this.cdr,
      this.dataMapService,
      this.drawModalService,
      this.virtualScroll,
      this.detailService,
      this.warningMessageService,
      this.protectedResourceApiService,
      this.protectedEnvironmentApiService,
      this.exceptionService,
      this.message,
      this.setResourceTagService,
      this.getLabelOptionsService
    );
    this.openstackListComponent = new OpenstackListComponent(
      this.i18n,
      this.slaService,
      this.cookieService,
      this.messageService,
      this.protectService,
      this.dataMapService,
      this.drawModalService,
      this.virtualScroll,
      this.detailService,
      this.takeManualBackupService,
      this.protectedResourceApiService,
      this.setResourceTagService,
      this.getLabelOptionsService
    );
    this.generalDatabaseComponent = new TableTemplateComponent(
      this.i18n,
      this.cdr,
      this.slaService,
      this.dataMapService,
      this.protectService,
      this.messageService,
      this.drawModalService,
      this.warningMessageService,
      this.virtualScroll,
      this.detailService,
      this.batchOperateService,
      this.takeManualBackupService,
      this.protectedResourceApiService,
      this.setResourceTagService,
      this.getLabelOptionsService,
      this.appUtilsService
    );

    this.databaseTemplateComponent = new DatabaseTemplateComponent(
      this.i18n,
      this.cdr,
      this.slaService,
      this.jobApiService,
      this.dataMapService,
      this.protectService,
      this.messageBox,
      this.messageService,
      this.registerService,
      this.drawModalService,
      this.virtualScroll,
      this.detailService,
      this.batchOperateService,
      this.warningMessageService,
      this.takeManualBackupService,
      this.protectedResourceApiService,
      this.setResourceTagService,
      this.getLabelOptionsService
    );

    this.objectStorageComponent = new ObjectStorageComponent(
      this.i18n,
      this.cdr,
      this.globalService,
      this.messageService,
      this.drawModalService,
      this.dataMapService,
      this.batchOperateService,
      this.warningMessageService,
      this.protectedResourceApiService,
      this.setResourceTagService,
      this.getLabelOptionsService
    );

    this.mongodbComponent = new MongodbComponent(
      this.i18n,
      this.dataMapService,
      this.drawModalService
    );

    this.kubernetesContainerComponent = new KubernetesContainerComponent(
      this.i18n,
      this.drawModalService
    );

    this.baseTableComponent = new BaseTableComponent(
      this.i18n,
      this.slaService,
      this.cookieService,
      this.protectService,
      this.dataMapService,
      this.drawModalService,
      this.virtualScroll,
      this.detailService,
      this.takeManualBackupService,
      this.protectedResourceApiService,
      this.warningMessageService,
      this.batchOperateService,
      this.globalService,
      this.setResourceTagService,
      this.getLabelOptionsService
    );
  }
}
