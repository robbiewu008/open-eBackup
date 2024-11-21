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
  AfterViewInit,
  ChangeDetectorRef,
  Component,
  EventEmitter,
  Input,
  OnChanges,
  OnDestroy,
  OnInit,
  Output,
  SimpleChanges,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  ClusterEnvironment,
  DataMap,
  DataMapService,
  extendSlaInfo,
  FilesetTemplatesApiService,
  GlobalService,
  I18NService,
  InstanceType,
  ProtectedResourceApiService,
  ResourceSetApiService,
  ResourceSetType
} from 'app/shared';
import {
  ProtectedResourcePageListResponse,
  ProtectedResourceResponse
} from 'app/shared/api/models';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { SlaService } from 'app/shared/services/sla.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  cloneDeep,
  defer,
  differenceBy,
  each,
  find,
  get,
  has,
  includes,
  isEmpty,
  map as _map,
  set,
  size,
  trim
} from 'lodash';
import { Observable, Subscription } from 'rxjs';
import { map } from 'rxjs/operators';

@Component({
  selector: 'aui-normal-resourceset-template',
  templateUrl: './normal-resourceset-template.component.html',
  styleUrls: ['./normal-resourceset-template.component.less']
})
export class NormalResourcesetTemplateComponent
  implements OnInit, OnDestroy, AfterViewInit, OnChanges {
  @Input() resourceType;
  @Input() subName;
  @Input() isDetail;
  @Input() allSelectionMap;
  @Input() appType;
  @Input() data;
  @Input() allSelect;
  @Input() tableLabel;
  @Output() allSelectChange = new EventEmitter<any>();
  @Output() onNumChange = new EventEmitter<any>(); // 特殊应用如nas需要获取后更新数量

  tableData: TableData;
  tableConfig: TableConfig;
  dataFetch$: Subscription = new Subscription();
  parentRelatedFetch$: Subscription = new Subscription();
  syncData$: Subscription = new Subscription();
  dataMap = DataMap;
  tmpAppType; //用于nas的父子关联
  isFirst = true; // nas触发其他组件获取数据只获取一次

  selectionData = [];

  @ViewChild('clusterTypeTpl', { static: true }) clusterTypeTpl: TemplateRef<
    any
  >;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('slaComplianceExtraTpl', { static: true })
  slaComplianceExtraTpl: TemplateRef<any>;
  @ViewChild('storageDeviceTpl', { static: true })
  storageDeviceTpl: TemplateRef<any>;
  @ViewChild('databaseTypeTpl', { static: true })
  databaseTypeTpl: TemplateRef<any>;
  @ViewChild('sapHanaDbDeployType', { static: true })
  sapHanaDbDeployType: TemplateRef<any>;

  constructor(
    public globalService: GlobalService,
    public virtualScroll: VirtualScrollService,
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private slaService: SlaService,
    private dataMapService: DataMapService,
    private appUtilsService: AppUtilsService,
    private resourceSetService: ResourceSetApiService,
    private filesetTemplatesApiService: FilesetTemplatesApiService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    if (this.resourceType === 'filesetTemplate') {
      this.appType = ResourceSetType.FilesetTemplate;
    }
    this.tmpAppType = includes(
      [ResourceSetType.NasFileSystem, ResourceSetType.NasShare],
      this.appType
    )
      ? ResourceSetType.StorageEquipment
      : this.appType;
    this.initConfig();
  }

  ngOnDestroy(): void {
    this.dataFetch$.unsubscribe();
    this.parentRelatedFetch$.unsubscribe();
    this.syncData$.unsubscribe();
  }

  ngOnChanges(changes: SimpleChanges) {
    if (changes.allSelect?.currentValue) {
      set(this.allSelectionMap, this.appType, {
        isAllSelected: true
      });
      // 全选
      this.selectionData = this.tableData.data;
      each(this.tableData.data, item => {
        item.disabled = true;
      });
      this.dataTable.setSelections(cloneDeep(this.selectionData));
    }

    // 取消全选
    if (
      !changes.allSelect?.currentValue &&
      !isEmpty(this.allSelectionMap[this.appType])
    ) {
      set(this.allSelectionMap, this.appType, {
        isAllSelected: false
      });
      this.selectionData = [];
      each(this.tableData?.data, item => {
        item.disabled = false;
      });
      this.dataTable.setSelections(cloneDeep(this.selectionData));
    }
  }

  ngAfterViewInit() {
    this.getFetchState();
  }

  getFetchState() {
    this.dataFetch$ = this.globalService
      .getState(this.subName)
      .subscribe(res => {
        if (!!this.tableData?.data) {
          return;
        }
        // 选择的nas资源需要去获取存储设备的选择情况，保证父亲被选中时不下发子类型资源
        if (
          [ResourceSetType.NasFileSystem, ResourceSetType.NasShare].includes(
            this.appType
          ) &&
          isEmpty(
            this.allSelectionMap[ResourceSetType.StorageEquipment]?.data
          ) &&
          !this.isDetail
        ) {
          this.globalService.emitStore({
            action: this.i18n.get('protection_storage_device_label'),
            state: true
          });
          defer(() => this.dataTable.fetchData());
        } else if (
          this.appType === ResourceSetType.StorageEquipment &&
          this.isFirst &&
          !this.isDetail
        ) {
          // 同理，选择存储设备时要顺带把子资源获取并清理
          this.isFirst = false;
          this.globalService.emitStore({
            action: this.i18n.get('common_nas_shares_label'),
            state: true
          });
          this.globalService.emitStore({
            action: this.i18n.get('common_nas_file_systems_label'),
            state: true
          });
        } else {
          this.dataTable.fetchData();
        }
      });

    if (!this.isDetail) {
      this.parentRelatedFetch$ = this.globalService
        .getState(`${this.tmpAppType}parentSelect`)
        .subscribe(res => {
          if (
            !isEmpty(this.allSelectionMap[this.tmpAppType]?.data) &&
            !this.allSelectionMap[this.appType]?.isAllSelected
          ) {
            this.parentSelectChild();
          }
        });

      this.syncData$ = this.globalService
        .getState(`${this.appType}syncData`)
        .subscribe(res => {
          this.selectionData = this.selectionData.filter(item =>
            find(this.allSelectionMap[this.appType].data, { uuid: item.uuid })
          );
          this.dataTable.setSelections(cloneDeep(this.selectionData));
        });
    }
  }

  initConfig() {
    // 可保护资源和顶层资源做区分
    const basicCols: { [key: string]: TableCols } = {
      // 资源ID
      uuid: {
        key: 'uuid',
        name: this.i18n.get('protection_resource_id_label'),
        hidden: true,
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      // 名称
      name: {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        cellRender: {
          type: 'text',
          config: {
            id: 'outerClosable',
            iconPos: 'flow-text'
          }
        },
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      // 状态，一般是是否在线
      linkStatus: {
        key: 'linkStatus',
        name: this.i18n.get('common_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('resource_LinkStatus_Special')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('resource_LinkStatus_Special')
        }
      },
      status: {
        key: 'status',
        name: this.i18n.get('common_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('gaussDBInstance')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('gaussDBInstance')
        }
      },
      // 另一个认证状态
      auth_status: {
        key: 'auth_status',
        name: this.i18n.get('common_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('resource_LinkStatus_Special')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('resource_LinkStatus_Special')
        }
      },
      subType: {
        key: 'subType', // 根据资源subType筛选
        name: this.i18n.get('common_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.getResourceSubTypeConfigMap()
        },
        cellRender: {
          type: 'status',
          config: this.getResourceSubTypeConfigMap()
        }
      },
      // ip
      path: {
        name: this.getPathLabel(),
        key: includes(
          [
            ResourceSetType.ObjectStorage,
            ResourceSetType.Kubernetes_CSI,
            ResourceSetType.Kubernetes_FlexVolume,
            ResourceSetType.HBase,
            ResourceSetType.HDFS,
            ResourceSetType.Hive,
            ResourceSetType.PostgreSQL
          ],
          this.appType
        )
          ? 'endpoint'
          : 'path'
      },
      // nasShare的ip
      ip: {
        key: 'ip',
        name: this.i18n.get('protection_fqdn_ip_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        extendParameter: [DataMap.Resource_Type.NASShare.value]
      },
      // 轻量云gaussdb的
      pmAddress: {
        key: 'pmAddress',
        name: this.i18n.get('protection_project_address_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      address: {
        key: 'address',
        name: this.i18n.get('common_address_label')
      },
      // 类型
      mysqlClusterType: {
        name:
          this.resourceType === DataMap.Resource_Type.KingBaseCluster.value
            ? this.i18n.get('protection_cluster_type_label')
            : this.i18n.get('common_type_label'),
        key: 'clusterType',
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.getTypeOptions()
        },
        cellRender: {
          type: 'status',
          config: this.getTypeOptions()
        }
      },
      // 类型
      oracleType: {
        name: this.i18n.get('common_type_label'),
        key: 'subType',
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('oracleType')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('oracleType')
        }
      },
      // 操作系统类型
      osType: {
        name: this.i18n.get('protection_os_type_label'),
        key: 'osType',
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.getOsTypeOptions()
        },
        cellRender: {
          type: 'status',
          config: this.getOsTypeOptions()
        }
      },
      // 数据库类型这个看能不能整合
      databaseType: {
        key: 'databaseType',
        name: this.i18n.get('protection_database_type_label')
      },
      sapHanaDbType: {
        key: 'sapHanaDbType',
        name: this.i18n.get('protection_database_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('saphanaDatabaseType')
        },
        cellRender: this.databaseTypeTpl
      },
      sapHanaDbDeployType: {
        key: 'sapHanaDbDeployType',
        name: this.i18n.get('common_database_deploy_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('saphanaDatabaseDeployType')
        },
        cellRender: this.sapHanaDbDeployType
      },
      // 版本
      version: {
        name: this.i18n.get('common_version_label'),
        key: 'version'
      },
      // 认证状态
      verify_status: {
        name: this.i18n.get('common_auth_status_label'),
        key: 'verify_status',
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Verify_Status')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Verify_Status')
        }
      },
      // 认证模式
      authType: {
        key: 'authType',
        name: this.i18n.get('protection_auth_mode_label'),
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('HDFS_Clusters_Auth_Type')
        }
      },
      // 轻量云的状态
      instanceStatus: {
        key: 'instanceStatus',
        name: this.i18n.get('common_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('airgapDeviceStatus')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('airgapDeviceStatus')
        }
      },
      // opengauss、gaussdbt的状态
      clusterState: {
        key: 'clusterState',
        name: this.i18n.get('common_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.getClusterStateOptions()
        },
        cellRender: {
          type: 'status',
          config: this.getClusterStateOptions()
        }
      },
      // informix的日志备份
      logBackup: {
        key: 'logBackup',
        name: this.i18n.get('common_log_backup_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('logBackupStatus')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('logBackupStatus')
        }
      },
      // 用于postsql的集群类型
      installDeployType: {
        key: 'installDeployType',
        name: this.i18n.get('protection_cluster_type_label'),
        filter: {
          type: 'select',
          options: this.dataMapService.toArray('PostgreSqlDeployType')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('PostgreSqlDeployType')
        }
      },
      // 集群类型看能不能跟数据库整合
      clusterType: {
        key: 'clusterType',
        name: this.i18n.get('common_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('oracleClusterType')
        },
        cellRender: this.clusterTypeTpl
      },
      // 所属环境名
      environmentName: {
        key: 'environmentName',
        name: this.getEnvironmentNameLabel(),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      environmentEndpoint: {
        key: 'environmentEndpoint',
        name: this.getEnvironmentEndpointName(),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      storageType: {
        key: 'storageType',
        name:
          this.resourceType === DataMap.Resource_Type.ObjectStorage.value
            ? this.i18n.get('common_type_label')
            : this.i18n.get('protection_object_storage_owned_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('objectStorageType')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('objectStorageType')
        }
      },
      // 特殊ip
      nodeIpAddress: {
        key: 'nodeIpAddress',
        name: this.i18n.get('protection_node_ip_address_label')
      },
      // 所属父名
      parentName: {
        key: 'parentName',
        name: this.getParentNameLabel(),
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        cellRender:
          this.appType === ResourceSetType.NasShare
            ? this.storageDeviceTpl
            : null
      },
      ak: {
        key: 'AK',
        name: 'AK',
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      // 协议
      protocol: {
        key:
          this.appType === ResourceSetType.NasShare ? 'shareMode' : 'protocol',
        name: this.i18n.get('explore_share_protocol_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.getProtocolType()
        },
        cellRender: {
          type: 'status',
          config: this.getProtocolType()
        }
      },
      tenantName: {
        key: 'tenantName',
        name: this.i18n.get('common_tenant_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      templateName: {
        key: 'templateName',
        name: this.i18n.get('protection_associate_template_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      // 存储设备的端口
      port: {
        key: 'port',
        name: this.i18n.get('common_port_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      wwn: {
        key: 'wwn',
        name: 'WWN',
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      esn: {
        key: 'esn',
        name: this.i18n.get('common_serial_number_label')
      },
      // sla三剑客
      sla: {
        key: 'sla_name',
        name: this.i18n.get('common_sla_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      sla_compliance: {
        key: 'sla_compliance',
        name: this.i18n.get('common_sla_compliance_label'),
        thExtra: this.slaComplianceExtraTpl,
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Sla_Compliance')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Sla_Compliance')
        }
      },
      protectionStatus: {
        key: 'protectionStatus',
        name: this.i18n.get('protection_protected_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Protection_Status')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Protection_Status')
        }
      }
    };
    this.tableConfig = {
      filterTags: true,
      table: {
        compareWith: 'uuid',
        columns: this.getApplicationCols(basicCols),
        rows: this.isDetail
          ? null
          : {
              selectionMode: 'multiple',
              selectionTrigger: 'selector',
              showSelector: true
            },
        scrollFixed: true,
        scroll: { y: '30vh' },
        colDisplayControl: {
          ignoringColsType: 'hide'
        },
        fetchData: (filter: Filters, args) => {
          this.getData(filter, args);
        },
        selectionChange: selection => {
          if (this.selectionData.length < selection.length) {
            // 新增选中的处理
            this.selectionData = selection;
            if (isEmpty(this.allSelectionMap[this.appType]?.data)) {
              set(this.allSelectionMap, this.appType, {
                data: cloneDeep(this.selectionData)
              });
            } else {
              this.parseSelected();
            }
          } else {
            // 减少选中时的处理
            const diffArray = differenceBy(
              this.selectionData,
              selection,
              'uuid'
            );
            this.allSelectionMap[this.appType].data = this.allSelectionMap[
              this.appType
            ].data.filter(item => {
              return !find(diffArray, { uuid: item.uuid });
            });
            this.selectionData = selection;
          }
          // 修改状态下需要在所有的selectionData都把取消的那个删掉，不然父子关联会有问题，导致selectionMap里面没有真正删除
          if (!!this.data) {
            this.globalService.emitStore({
              action: `${this.tmpAppType}syncData`,
              state: true
            });
          }
          // 触发父子关联选择
          defer(() => {
            this.globalService.emitStore({
              action: `${this.tmpAppType}parentSelect`,
              state: true
            });
          });

          this.allSelectChange.emit();
        },
        trackByFn: (index, item) => {
          return item.uuid;
        }
      }
    };
  }

  parseSelected() {
    each(this.selectionData, item => {
      if (!find(this.allSelectionMap[this.appType].data, { uuid: item.uuid })) {
        this.allSelectionMap[this.appType].data.push(item);
      }
    });
  }

  getPathLabel() {
    // 获取endpoint或者ip的label
    if (this.appType === ResourceSetType.HBase) {
      return this.i18n.get('common_ip_domain_name_label');
    } else if (this.appType === ResourceSetType.HDFS) {
      return this.i18n.get('fs.defaultFS');
    } else if (this.appType === ResourceSetType.Hive) {
      return this.i18n.get('protection_hive_server_link_label');
    } else if (this.appType === ResourceSetType.PostgreSQL) {
      return this.i18n.get('common_virtual_ip_label');
    } else {
      return this.i18n.get('common_ip_address_label');
    }
  }

  getEnvironmentNameLabel() {
    // 获取所属环境的表头名称
    if (
      includes(
        [
          DataMap.Resource_Type.SQLServerClusterInstance.value,
          DataMap.Resource_Type.SQLServerInstance.value,
          DataMap.Resource_Type.SQLServerDatabase.value,
          DataMap.Resource_Type.dbTwoInstance.value,
          DataMap.Resource_Type.dbTwoDatabase.value,
          DataMap.Resource_Type.dbTwoTableSet.value,
          DataMap.Resource_Type.PostgreSQLInstance.value,
          DataMap.Resource_Type.KingBaseInstance.value
        ],
        this.resourceType
      )
    ) {
      return this.i18n.get('protection_host_cluster_name_label');
    } else if (
      includes(
        [
          DataMap.Resource_Type.fileset.value,
          DataMap.Resource_Type.volume.value,
          DataMap.Resource_Type.ActiveDirectory.value
        ],
        this.resourceType
      )
    ) {
      return this.i18n.get('protection_host_name_label');
    } else if (
      includes(
        [
          DataMap.Resource_Type.ExchangeDataBase.value,
          DataMap.Resource_Type.ExchangeEmail.value
        ],
        this.resourceType
      )
    ) {
      // 所属单机/可用性组
      return this.i18n.get('protection_single_node_system_group_tag_label');
    } else if (
      includes([DataMap.Resource_Type.ObjectSet.value], this.resourceType)
    ) {
      return this.i18n.get('protection_object_storage_owned_label');
    } else if (
      this.resourceType === DataMap.Resource_Type.saphanaDatabase.value
    ) {
      // 所属实例
      return this.i18n.get('commom_owned_instance_label');
    } else {
      return this.i18n.get('insight_report_belong_cluster_label');
    }
  }

  getClusterStateOptions() {
    if (this.appType === ResourceSetType.OpenGauss) {
      return this.dataMapService.toArray('opengauss_Clusterstate');
    } else if (this.appType === ResourceSetType.GaussDB_T) {
      return this.dataMapService.toArray('gaussDBT_Resource_LinkStatus');
    }
  }

  getEnvironmentEndpointName() {
    // 环境endpoint的名字
    if (
      this.resourceType ===
      DataMap.Resource_Type.kubernetesNamespaceCommon.value
    ) {
      return this.i18n.get('protection_cluster_ip_label');
    } else {
      return this.i18n.get('common_ip_address_label');
    }
  }

  getProtocolType() {
    if (this.appType === ResourceSetType.NasFileSystem) {
      return this.dataMapService.toArray('NasFileSystem_Protocol');
    } else if (this.appType === ResourceSetType.NasShare) {
      return this.dataMapService.toArray('Shared_Mode');
    } else if (this.appType === ResourceSetType.ObjectStorage) {
      return this.dataMapService.toArray('protocolType');
    } else {
      return [];
    }
  }

  // 根据类型返回资源子类型的DataMap
  getResourceSubTypeConfigMap() {
    switch (this.resourceType) {
      case DataMap.Resource_Type.Exchange.value:
        return this.dataMapService.toArray('exchangeGroupType');
      case DataMap.Resource_Type.Dameng.value:
        return this.dataMapService.toArray('Dameng_Type');
      case DataMap.Resource_Type.GaussDB_T.value:
        return this.dataMapService.toArray('gaussDBTClusterType');
      case DataMap.Resource_Type.NASFileSystem.value:
        if (this.appType === ResourceSetType.StorageEquipment) {
          return this.dataMapService
            .toArray('Device_Storage_Type')
            .filter(item => {
              return item.value !== DataMap.Device_Storage_Type.Other.value;
            });
        }
    }
  }

  getApplicationCols(cols: { [key: string]: TableCols }): TableCols[] {
    // 用于获取每个应用的每个资源类型自己的表格项
    // 定义一个sla常用的表格项组合
    const slaCols = [cols.sla, cols.sla_compliance, cols.protectionStatus];
    // 定义一个名称常用的表格项组合
    const nameCols = [cols.uuid, cols.name];
    const resType = DataMap.Resource_Type;
    switch (this.resourceType) {
      // 数据库应用
      case ClusterEnvironment.oralceClusterEnv:
        return [cols.uuid, cols.name, cols.linkStatus, cols.clusterType];
      case resType.oracle.value:
        return [
          ...nameCols,
          cols.linkStatus,
          cols.path,
          cols.oracleType,
          cols.osType,
          cols.version,
          cols.verify_status,
          ...slaCols
        ];
      case resType.MySQLCluster.value:
        return [cols.name, cols.linkStatus, cols.mysqlClusterType];
      case DataMap.Resource_Type.MySQLInstance.value:
        return [
          ...nameCols,
          cols.auth_status,
          cols.environmentName,
          cols.databaseType,
          cols.version,
          ...slaCols
        ];
      case resType.MySQLDatabase.value:
        return [...nameCols, cols.environmentName, cols.parentName, ...slaCols];
      case resType.SQLServerCluster.value:
        return [...nameCols, cols.linkStatus];
      case resType.SQLServerInstance.value:
        return [
          ...nameCols,
          cols.nodeIpAddress,
          cols.environmentName,
          cols.version,
          ...slaCols
        ];
      case resType.SQLServerGroup.value:
        return [...nameCols, cols.environmentName, ...slaCols];
      case resType.SQLServerDatabase.value:
        return [...nameCols, cols.environmentName, cols.parentName, ...slaCols];
      case resType.PostgreSQLCluster.value:
        return [
          cols.name,
          cols.linkStatus,
          cols.installDeployType,
          cols.mysqlClusterType,
          cols.path
        ];
      case resType.PostgreSQLInstance.value:
        return [
          ...nameCols,
          cols.linkStatus,
          cols.environmentName,
          cols.environmentEndpoint,
          cols.version,
          ...slaCols
        ];
      case resType.dbTwoCluster.value:
        return [...nameCols, cols.linkStatus, cols.mysqlClusterType];
      case resType.dbTwoInstance.value:
        return [
          ...nameCols,
          cols.linkStatus,
          cols.environmentName,
          cols.version
        ];
      case resType.dbTwoDatabase.value:
        return [...nameCols, cols.parentName, cols.environmentName, ...slaCols];
      case resType.dbTwoTableSet.value:
        return [...nameCols, cols.parentName, cols.environmentName, ...slaCols];
      case resType.informixService.value:
        return [...nameCols, cols.linkStatus, cols.logBackup];
      case resType.informixInstance.value:
        return [
          ...nameCols,
          cols.linkStatus,
          cols.environmentName,
          cols.version,
          ...slaCols
        ];
      case resType.OpenGauss.value:
        return [...nameCols, cols.clusterState];
      case resType.OpenGauss_instance.value:
        return [...nameCols, cols.linkStatus, cols.environmentName, ...slaCols];
      case resType.OpenGauss_database.value:
        return [...nameCols, cols.parentName, cols.environmentName, ...slaCols];
      case resType.tidbCluster.value:
      case resType.OceanBaseCluster.value:
        return [...nameCols, cols.linkStatus, cols.version, ...slaCols];
      case resType.tidbDatabase.value:
        return [...nameCols, cols.linkStatus, cols.parentName, ...slaCols];
      case resType.tidbTable.value:
        return [
          ...nameCols,
          cols.linkStatus,
          cols.environmentName,
          cols.parentName,
          ...slaCols
        ];
      case resType.OceanBaseTenant.value:
      case resType.tdsqlInstance.value:
      case resType.tdsqlDistributedInstance.value:
        return [...nameCols, cols.linkStatus, cols.environmentName, ...slaCols];
      case resType.tdsqlCluster.value:
        return [...nameCols, cols.linkStatus];
      case resType.KingBaseCluster.value:
        return [...nameCols, cols.linkStatus, cols.mysqlClusterType];
      case resType.KingBaseInstance.value:
        return [
          ...nameCols,
          cols.environmentName,
          cols.version,
          cols.linkStatus,
          ...slaCols
        ];
      case resType.Dameng.value:
        return [
          ...nameCols,
          cols.linkStatus,
          cols.subType,
          cols.version,
          ...slaCols
        ];
      case resType.goldendbCluter.value:
        return [...nameCols, cols.linkStatus, cols.nodeIpAddress, cols.version];
      case resType.goldendbInstance.value:
        return [...nameCols, cols.linkStatus, cols.nodeIpAddress, ...slaCols];
      case resType.generalDatabase.value:
        return [
          ...nameCols,
          cols.linkStatus,
          cols.databaseType,
          cols.nodeIpAddress,
          cols.version,
          ...slaCols
        ];
      case resType.lightCloudGaussdbProject.value:
        return [...nameCols, cols.pmAddress, cols.linkStatus];
      case resType.lightCloudGaussdbInstance.value:
        return [
          ...nameCols,
          cols.pmAddress,
          cols.instanceStatus,
          cols.parentName,
          ...slaCols
        ];
      case resType.GaussDB_T.value:
        return [...nameCols, cols.version, cols.subType, ...slaCols];
      // 文件系统
      case resType.fileset.value:
        return [
          ...nameCols,
          cols.environmentName,
          cols.path,
          cols.osType,
          cols.templateName,
          ...slaCols
        ];
      case 'filesetTemplate':
        return [cols.name, cols.osType];
      case resType.commonShare.value:
        return [...nameCols, ...slaCols];
      case resType.ActiveDirectory.value:
        return [
          ...nameCols,
          cols.environmentName,
          cols.environmentEndpoint,
          ...slaCols
        ];
      case resType.saphanaInstance.value:
        return [...nameCols, cols.linkStatus, cols.version];
      case resType.saphanaDatabase.value:
        return [
          ...nameCols,
          cols.linkStatus,
          cols.sapHanaDbType,
          cols.sapHanaDbDeployType,
          cols.environmentName,
          cols.environmentEndpoint,
          ...slaCols
        ];
      case resType.NASFileSystem.value:
        if (this.appType === ResourceSetType.StorageEquipment) {
          return [
            ...nameCols,
            cols.linkStatus,
            cols.subType,
            cols.path,
            cols.port,
            cols.wwn,
            cols.esn
          ];
        } else {
          return [
            ...nameCols,
            cols.parentName,
            cols.protocol,
            cols.tenantName,
            ...slaCols
          ];
        }
      case resType.NASShare.value:
        return [
          ...nameCols,
          cols.ip,
          cols.parentName,
          cols.protocol,
          ...slaCols
        ];
      case resType.ndmp.value:
        if (this.tableLabel === this.i18n.get('common_file_systems_label')) {
          return [...nameCols, cols.parentName, cols.tenantName, ...slaCols];
        } else {
          return [...nameCols, cols.parentName, ...slaCols];
        }
      case resType.volume.value:
        return [
          ...nameCols,
          cols.environmentName,
          cols.environmentEndpoint,
          ...slaCols
        ];
      case resType.ObjectStorage.value:
        return [
          ...nameCols,
          cols.linkStatus,
          cols.storageType,
          cols.path,
          cols.protocol,
          cols.ak
        ];
      case resType.ObjectSet.value:
        return [
          ...nameCols,
          cols.environmentName,
          cols.storageType,
          ...slaCols
        ];
      case resType.Exchange.value:
        return [...nameCols, cols.linkStatus, cols.subType, ...slaCols];
      case resType.ExchangeDataBase.value:
        return [...nameCols, cols.environmentName, ...slaCols];
      case resType.ExchangeEmail.value:
        return [
          ...nameCols,
          cols.address,
          cols.environmentName,
          cols.parentName,
          ...slaCols
        ];
      case resType.kubernetesClusterCommon.value:
        return [cols.name, cols.version, cols.linkStatus, cols.path];
      case resType.kubernetesNamespaceCommon.value:
        return [
          ...nameCols,
          cols.environmentName,
          cols.environmentEndpoint,
          ...slaCols
        ];
      case resType.kubernetesDatasetCommon.value:
      case resType.KubernetesStatefulset.value:
        return [...nameCols, cols.parentName, cols.environmentName, ...slaCols];
      case resType.Kubernetes.value:
        return [cols.name, cols.version, cols.linkStatus, cols.path];
      case resType.KubernetesNamespace.value:
        return [...nameCols, ...slaCols];
      case resType.MongoDB.value:
        return [
          ...nameCols,
          cols.linkStatus,
          cols.version,
          cols.mysqlClusterType,
          ...slaCols
        ];
      case resType.Redis.value:
        return [...nameCols, cols.linkStatus, cols.version, ...slaCols];
      case resType.HBase.value:
        return [cols.name, cols.linkStatus, cols.path, cols.authType];
      case resType.HBaseBackupSet.value:
      case resType.HDFSFileset.value:
      case resType.HiveBackupSet.value:
        return [...nameCols, cols.environmentName, ...slaCols];
      case resType.HDFS.value:
        return [cols.name, cols.path, cols.linkStatus, cols.authType];
      case resType.Hive.value:
        return [cols.name, cols.linkStatus, cols.path, cols.authType];
      default:
        return nameCols;
    }
  }

  getOsTypeOptions() {
    // 用于获取操作系统类型
    switch (this.resourceType) {
      case DataMap.Resource_Type.oracle.value:
        return this.dataMapService.toArray('Os_Type').filter(item => {
          return includes(
            [
              DataMap.Os_Type.windows.value,
              DataMap.Os_Type.linux.value,
              DataMap.Os_Type.aix.value
            ],
            item.value
          );
        });
      case DataMap.Resource_Type.fileset.value:
        return this.dataMapService.toArray('Os_Type').filter(item => {
          return includes(
            [
              DataMap.Os_Type.windows.value,
              DataMap.Os_Type.linux.value,
              DataMap.Os_Type.aix.value,
              DataMap.Os_Type.solaris.value
            ],
            item.value
          );
        });
      case 'filesetTemplate':
        return this.dataMapService.toArray('Fileset_Template_Os_Type');
    }
  }

  getTypeOptions() {
    switch (this.resourceType) {
      case DataMap.Resource_Type.MySQLCluster.value:
        return this.dataMapService.toArray('Mysql_Cluster_Type');
      case DataMap.Resource_Type.MongoDB.value:
        return this.dataMapService
          .toArray('mongodbInstanceType')
          .filter(
            item =>
              !includes(
                [DataMap.mongodbInstanceType.clusterPrimary.value],
                item.value
              )
          );
      case DataMap.Resource_Type.PostgreSQLCluster.value:
      case DataMap.Resource_Type.KingBaseCluster.value:
        return this.dataMapService.toArray('PostgreSql_Cluster_Type');
      case DataMap.Resource_Type.dbTwoCluster.value:
        return this.dataMapService.toArray('dbTwoType');
    }
  }

  getParentNameLabel() {
    switch (this.resourceType) {
      case DataMap.Resource_Type.MySQLDatabase.value:
        return this.i18n.get('protection_database_instance_label');
      case DataMap.Resource_Type.DWS_Schema.value:
      case DataMap.Resource_Type.DWS_Table.value:
      case DataMap.Resource_Type.dbTwoTableSet.value:
      case DataMap.Resource_Type.tidbTable.value:
      case DataMap.Resource_Type.ExchangeEmail.value:
        // 所属数据库
        return this.i18n.get('protection_host_database_name_label');
      case DataMap.Resource_Type.lightCloudGaussdbInstance.value:
        // 所属项目
        return this.i18n.get('commom_owned_project_label');
      case DataMap.Resource_Type.SQLServerGroup.value:
      case DataMap.Resource_Type.OceanBaseTenant.value:
      case DataMap.Resource_Type.tidbDatabase.value:
      case DataMap.Resource_Type.kubernetesNamespaceCommon.value:
        // 所属集群
        return this.i18n.get('insight_report_belong_cluster_label');
      case DataMap.Resource_Type.NASFileSystem.value:
      case DataMap.Resource_Type.NASShare.value:
      case DataMap.Resource_Type.ndmp.value:
        // 存储设备
        return this.i18n.get('protection_storage_device_label');
      case DataMap.Resource_Type.kubernetesDatasetCommon.value:
      case DataMap.Resource_Type.KubernetesStatefulset.value:
        // 所属命名空间
        return this.i18n.get('protection_belong_namespace_label');
      default:
        // 所属实例
        return this.i18n.get('commom_owned_instance_label');
    }
  }

  getData(filters: Filters, args) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize
    };

    const defaultConditions = this.getDefaultConditions();

    if (!isEmpty(filters.conditions_v2)) {
      const conditionsTemp = JSON.parse(
        this.resourceType === 'filesetTemplate'
          ? filters.conditions
          : filters.conditions_v2
      );
      if (conditionsTemp.isAllowRestore) {
        if (conditionsTemp.isAllowRestore.length === 3) {
          delete conditionsTemp.isAllowRestore;
        } else if (conditionsTemp.isAllowRestore.includes('false')) {
          conditionsTemp.isAllowRestore[0] = ['!='];
          conditionsTemp.isAllowRestore[1] = 'true';
        }
      }
      if (
        conditionsTemp.osType &&
        this.resourceType === DataMap.Resource_Type.fileset.value
      ) {
        assign(conditionsTemp, {
          environment: {
            osType: conditionsTemp.osType
          }
        });
        delete conditionsTemp.osType;
      }
      // environment中的过滤条件需要单独处理
      const { environmentName, environmentEndpoint, ...rest } = conditionsTemp;
      const environmentCondition = {};
      if (environmentName) {
        assign(environmentCondition, {
          name: environmentName
        });
      }
      if (environmentEndpoint) {
        assign(environmentCondition, {
          endpoint: environmentEndpoint
        });
      }
      if (!isEmpty(environmentCondition)) {
        assign(rest, {
          environment: environmentCondition
        });
      }
      assign(defaultConditions, rest);
    }

    if (this.isDetail) {
      // 详情时的获取处理
      assign(defaultConditions, {
        resourceSetId: this.data[0].uuid
      });
    }

    if (this.resourceType === 'filesetTemplate') {
      params.pageNo++;
      delete defaultConditions.subType;
    }

    assign(params, { conditions: JSON.stringify(defaultConditions) });

    if (!!size(filters.sort)) {
      assign(params, { orders: filters.orders });
    }

    const queryFunc: Observable<any> =
      this.resourceType === 'filesetTemplate'
        ? this.filesetTemplatesApiService.listUsingGET(params)
        : this.protectedResourceApiService.ListResources(params);

    queryFunc
      .pipe(
        map((res: ProtectedResourcePageListResponse) => {
          each(res.records, (item: ProtectedResourceResponse) => {
            this.formatData(item);
            this.formatCustomData(item);
            extendSlaInfo(item);
          });
          return res;
        })
      )
      .subscribe(res => {
        // 全选则翻页时自动选中
        if (this.allSelect) {
          each(res.records, (item: any) => {
            item.disabled = true;
            if (!find(this.selectionData, { uuid: item.uuid })) {
              this.selectionData.push(item);
            }
            this.dataTable.setSelections(this.selectionData);
          });
        }

        if (
          this.isDetail &&
          includes(
            [
              ResourceSetType.NasFileSystem,
              ResourceSetType.NasShare,
              ResourceSetType.StorageEquipment,
              ResourceSetType.PostgreSQL,
              ResourceSetType.Informix,
              ResourceSetType.KingBase,
              ResourceSetType.MySQL
            ],
            this.appType
          ) &&
          !this.tableData?.data
        ) {
          this.onNumChange.emit({ num: res.totalCount, appType: this.appType });
        }

        this.tableData = {
          total: res.totalCount,
          data: res.records
        };

        // 如果父级被选中，则需要把子级选中并且置灰,父级被取消选中则取消置灰,全选状态下不考虑这个
        if (
          !isEmpty(this.allSelectionMap[this.tmpAppType]?.data) &&
          !this.allSelectionMap[this.appType]?.isAllSelected &&
          !this.isDetail
        ) {
          this.parentSelectChild();
        }

        if (
          !!this.data &&
          isEmpty(this.allSelectionMap[this.appType]?.data) &&
          !this.isDetail
        ) {
          // 只有在修改场景时第一次进入组件会获取一次
          this.getSelectedData();
        } else if (
          !!this.data &&
          !isEmpty(this.allSelectionMap[this.appType]?.data) &&
          !this.isDetail
        ) {
          // 一个大应用下的多层tab可能会触发不同的获取数据接口，如果时间差太多会导致不单独获取数据，需要手动回显
          this.selectionData = cloneDeep(
            this.allSelectionMap[this.appType].data
          );
          this.dataTable.setSelections(cloneDeep(this.selectionData));
          this.allSelectChange.emit();
          this.parentSelectChild();
        }
        this.cdr.detectChanges();
      });
  }

  private parentSelectChild() {
    if (!this.tableData) {
      return;
    }
    each(this.tableData.data, item => {
      if (
        find(
          this.allSelectionMap[this.tmpAppType]?.data,
          val =>
            (val.uuid === item?.parentUuid ||
              val.uuid === item?.environment?.uuid) &&
            !(
              this.appType === ResourceSetType.StorageEquipment &&
              val.uuid === item?.parentUuid
            )
        )
      ) {
        assign(item, {
          disabled: true
        });
        if (!find(this.selectionData, { uuid: item.uuid })) {
          this.selectionData.push(item);
        }
      } else {
        assign(item, {
          disabled: false
        });
      }
    });
    this.dataTable.setSelections(cloneDeep(this.selectionData));
    // nas的父级是存储设备，资源集类型不一样所以的单独判断一下之前有没有值
    if (
      includes(
        [ResourceSetType.NasFileSystem, ResourceSetType.NasShare],
        this.appType
      ) &&
      isEmpty(this.allSelectionMap[this.appType]?.data)
    ) {
      set(this.allSelectionMap, this.appType, {
        data: cloneDeep(this.selectionData)
      });
    }
    this.parseSelected();
    this.allSelectChange.emit();
  }

  getSelectedData() {
    // 用于修改时获取之前被选中的数据
    const params: any = {
      resourceSetId: this.data[0].uuid,
      scopeModule: this.appType,
      type: 'RESOURCE'
    };
    if ([ResourceSetType.PostgreSQL].includes(this.appType)) {
      assign(params, {
        isNameExist: true
      });
    }
    this.resourceSetService.queryResourceObjectIdList(params).subscribe(res => {
      set(this.allSelectionMap, this.appType, {
        data: _map(res, item => {
          return { uuid: item };
        })
      });
      this.selectionData = cloneDeep(this.allSelectionMap[this.appType].data);
      this.dataTable.setSelections(cloneDeep(this.selectionData));
      this.allSelectChange.emit();
      this.parentSelectChild();
    });
  }

  getDefaultConditions() {
    const resType = DataMap.Resource_Type;
    switch (this.resourceType) {
      case ClusterEnvironment.oralceClusterEnv:
        return {
          subType: [ClusterEnvironment.oralceClusterEnv]
        };
      case resType.oracle.value:
        return {
          subType: [resType.oracle.value, resType.oracleCluster.value]
        };
      case resType.MySQLInstance.value:
        return {
          subType: [
            resType.MySQLClusterInstance.value,
            resType.MySQLInstance.value
          ],
          isTopInstance: InstanceType.TopInstance
        };
      case resType.SQLServerInstance.value:
        return {
          subType: [
            resType.SQLServerInstance.value,
            resType.SQLServerClusterInstance.value
          ]
        };
      case resType.SQLServerDatabase.value:
        return {
          subType: resType.SQLServerDatabase.value,
          agId: [['=='], '']
        };
      case resType.PostgreSQLInstance.value:
        return {
          subType: [
            resType.PostgreSQLClusterInstance.value,
            resType.PostgreSQLInstance.value
          ],
          isTopInstance: InstanceType.TopInstance
        };
      case resType.dbTwoInstance.value:
        return {
          subType: [
            resType.dbTwoClusterInstance.value,
            resType.dbTwoInstance.value
          ],
          isTopInstance: InstanceType.TopInstance
        };
      case resType.informixInstance.value:
        return {
          subType: [
            resType.informixInstance.value,
            resType.informixClusterInstance.value
          ],
          isTopInstance: InstanceType.TopInstance
        };
      case resType.GaussDB_T.value:
        return {
          subType: [resType.GaussDB_T.value, resType.gaussdbTSingle.value]
        };
      case resType.KingBaseInstance.value:
        return {
          subType: [
            resType.KingBaseInstance.value,
            resType.KingBaseClusterInstance.value
          ],
          isTopInstance: InstanceType.TopInstance
        };
      case resType.Dameng.value:
        return {
          subType: [
            resType.Dameng_cluster.value,
            resType.Dameng_singleNode.value
          ],
          isTopInstance: InstanceType.TopInstance
        };
      case resType.generalDatabase.value:
        return {
          subType: resType.generalDatabase.value,
          firstClassification: '2'
        };
      case resType.MongoDB.value:
        return {
          subType: [
            resType.MongodbClusterInstance.value,
            resType.MongodbSingleInstance.value
          ],
          isTopInstance: InstanceType.TopInstance
        };
      case resType.Redis.value:
        return {
          subType: [resType.Redis.value],
          resourceType: [['=='], 'cluster']
        };
      case resType.ClickHouseCluster.value:
      case resType.ClickHouseDatabase.value:
      case resType.ClickHouseTableset.value:
        return {
          subType: resType.ClickHouse.value,
          type:
            this.resourceType === resType.ClickHouseCluster.value
              ? 'Cluster'
              : this.resourceType === resType.ClickHouseDatabase.value
              ? 'Database'
              : 'TableSet'
        };
      case resType.Exchange.value:
        return {
          subType: [resType.ExchangeGroup.value, resType.ExchangeSingle.value]
        };
      case resType.NASFileSystem.value:
        if (this.appType === ResourceSetType.NasFileSystem) {
          return {
            subType: [resType.NASFileSystem.value]
          };
        } else {
          return {
            type: 'StorageEquipment',
            subType: [['!='], DataMap.Device_Storage_Type.Other.value]
          };
        }
      case DataMap.Resource_Type.saphanaInstance.value:
        return {
          subType: [DataMap.Resource_Type.saphanaInstance.value],
          isTopInstance: InstanceType.TopInstance
        };
      case DataMap.Resource_Type.ndmp.value:
        if (this.tableLabel === this.i18n.get('common_file_systems_label')) {
          return {
            subType: [DataMap.Resource_Type.ndmp.value],
            isFs: [['!='], '0']
          };
        } else {
          return {
            subType: [DataMap.Resource_Type.ndmp.value],
            isFs: [['=='], '0']
          };
        }
      default:
        return {
          subType: this.resourceType
        };
    }
  }

  formatData(item) {
    assign(item, {
      sub_type: item.subType,
      environmentName: item?.environment?.name
    });
  }

  formatCustomData(item: any) {
    // 名字太长了简化一下，搞一个resType
    const resType = DataMap.Resource_Type;
    switch (this.resourceType) {
      case DataMap.Resource_Type.saphanaInstance.value:
        assign(item, {
          enableLogBackup: item?.extendInfo?.enableLogBackup
        });
        break;
      case DataMap.Resource_Type.saphanaDatabase.value:
        assign(item, {
          sapHanaDbType: item?.extendInfo?.sapHanaDbType,
          sapHanaDbDeployType: item?.extendInfo?.sapHanaDbDeployType,
          linkStatus: item.extendInfo?.linkStatus,
          environmentEndpoint: item.environment?.endpoint
        });
        break;
      case DataMap.Resource_Type.ActiveDirectory.value:
        assign(item, {
          environmentEndpoint: item.environment?.endpoint,
          environmentName: item.environment?.name
        });
        break;
      case resType.GaussDB_T.value:
        assign(item, {
          clusterState: item.extendInfo?.clusterState
        });
        break;
      case resType.generalDatabase.value:
        assign(item, {
          databaseType: item.extendInfo?.databaseTypeDisplay,
          nodeIpAddress: item.extendInfo?.relatedHostIps
        });
        break;
      case resType.goldendbCluter.value:
        assign(item, {
          nodeIpAddress: item.endpoint
        });
        break;
      case resType.goldendbInstance.value:
        assign(item, {
          nodeIpAddress: item.environment?.endpoint,
          linkStatus: item.extendInfo?.linkStatus
        });
        break;
      case resType.Hive.value:
      case resType.HDFS.value:
        assign(item, {
          authType:
            item?.auth?.authType === DataMap.HDFS_Clusters_Auth_Type.ldap.value
              ? DataMap.HDFS_Clusters_Auth_Type.kerberos.value
              : item?.auth?.authType
        });
        break;
      case resType.HBase.value:
        assign(item, {
          authType:
            item?.auth?.authType ===
              DataMap.HDFS_Clusters_Auth_Type.ldap.value ||
            item?.auth?.authType === 2
              ? DataMap.HDFS_Clusters_Auth_Type.kerberos.value
              : item?.auth?.authType
        });
        break;
      case resType.MongoDB.value:
        assign(item, {
          clusterType: item.extendInfo?.clusterType
        });
        break;
      case resType.kubernetesNamespaceCommon.value:
        assign(item, {
          environmentEndpoint: item.environment?.endpoint
        });
        break;
      case resType.NASShare.value:
        assign(item, {
          ip: item.extendInfo?.ip,
          shareMode: item.extendInfo?.shareMode,
          parentName:
            item.environment &&
            item.environment['subType'] !==
              DataMap.Device_Storage_Type.Other.value
              ? item.environment['name']
              : ''
        });
        break;
      case resType.NASFileSystem.value:
        if (this.appType === ResourceSetType.StorageEquipment) {
          assign(item, {
            wwn:
              includes(
                [
                  DataMap.Device_Storage_Type.DoradoV7.value,
                  DataMap.Device_Storage_Type.OceanStorDoradoV7.value,
                  DataMap.Device_Storage_Type.OceanStorDorado_6_1_3.value,
                  DataMap.Device_Storage_Type.OceanStor_6_1_3.value,
                  DataMap.Device_Storage_Type.OceanStor_v5.value,
                  DataMap.Device_Storage_Type.OceanProtect.value
                ],
                item.subType
              ) && trim(item['endpoint']) !== '0'
                ? item.extendInfo?.wwn
                : '',
            esn: includes(
              [DataMap.Device_Storage_Type.NetApp.value],
              item.subType
            )
              ? ''
              : item.uuid
          });
        } else {
          assign(item, {
            tenantName: item.extendInfo?.tenantName,
            protocol: item.extendInfo?.protocol
          });
        }
        break;
      case resType.ndmp.value:
        assign(item, {
          tenantName: item.extendInfo?.tenantName
        });
        break;
      case resType.fileset.value:
        assign(item, {
          osType: item.environment?.osType,
          templateName: item.extendInfo?.templateName
        });
        break;
      case resType.PostgreSQLInstance.value:
        assign(item, {
          linkStatus: item.extendInfo?.linkStatus,
          environmentEndpoint: item.environment?.endpoint
        });
        break;
      case resType.PostgreSQLCluster.value:
        assign(item, {
          clusterType: item.extendInfo?.clusterType,
          installDeployType:
            item.extendInfo?.installDeployType ||
            DataMap.PostgreSqlDeployType.Pgpool.value
        });
        break;
      case resType.MySQLDatabase.value:
        assign(item, {
          environmentName: item.environment?.name,
          auth_status: item.extendInfo?.authStatus
        });
        break;
      case resType.MySQLInstance.value:
        assign(item, {
          databaseType: includes(item.version, 'MariaDB')
            ? DataMap.mysqlDatabaseType.mariaDb.value
            : DataMap.mysqlDatabaseType.mySql.value,
          auth_status: item.extendInfo?.linkStatus,
          environmentName: item.environment?.name
        });
        break;
      case resType.MySQLCluster.value:
        assign(item, { clusterType: item.extendInfo?.clusterType });
        break;
      case resType.oracle.value:
        assign(item, {
          verify_status: item.extendInfo?.verify_status === 'true',
          linkStatus: item.extendInfo?.linkStatus,
          osType: item.environment?.osType
        });
        break;
      case resType.ActiveDirectory.value:
        assign(item, {
          environmentEndpoint: item.environment?.endpoint,
          environmentName: item.environment?.name
        });
        break;
      case resType.volume.value:
        assign(item, {
          environmentEndpoint: item.environment?.endpoint
        });
        break;
      case resType.SQLServerDatabase.value:
        assign(item, {
          ownedInstance: item.extendInfo?.instanceName,
          clusterOrHostName: item.extendInfo?.hostName
        });
        break;
      case resType.SQLServerInstance.value:
        assign(item, {
          nodeIpAddress:
            item.subType === resType.SQLServerClusterInstance.value
              ? item.path
              : item.environment?.endpoint
        });
        break;
      case resType.dbTwoCluster.value:
        assign(item, {
          clusterType: item?.extendInfo?.clusterType
        });
        break;
      case resType.dbTwoInstance.value:
      case resType.dbTwoClusterInstance.value:
        assign(item, {
          environmentName: !!get(item, 'environment.extendInfo.clusterType')
            ? item.environment?.name
            : `${item.environment?.name}(${item.environment?.endpoint})`,
          linkStatus: item.extendInfo?.linkStatus
        });
        break;
      case resType.dbTwoDatabase.value:
        assign(item, {
          environmentName: !!get(item, 'environment.extendInfo.clusterType')
            ? item.environment?.name
            : `${item.environment?.name}(${item.environment?.endpoint})`
        });
        break;
      case resType.dbTwoTableSet.value:
        assign(item, {
          environmentName: has(item.environment, 'extendInfo.clusterType')
            ? item.environment?.name
            : `${item.environment?.name}(${item.environment?.endpoint})`,
          instance: item.extendInfo?.instance
        });
        break;
      case resType.goldendbCluter.value:
        assign(item, {
          nodeIpAddress: item.endpoint
        });
        break;
      case resType.goldendbInstance.value:
        assign(item, {
          nodeIpAddress: item.environment?.endpoint,
          linkStatus: item.extendInfo?.linkStatus
        });
        break;
      case resType.gaussdbForOpengaussInstance.value:
        assign(item, {
          status: item.extendInfo?.status,
          region: item.extendInfo?.region
        });
        break;
      case resType.informixInstance.value:
        assign(item, {
          environmentName: item.extendInfo?.clusterName,
          linkStatus: item.extendInfo?.linkStatus
        });
        break;
      case resType.OceanBaseTenant.value:
        assign(item, {
          environmentName: item.environment?.name,
          linkStatus: item.extendInfo?.linkStatus
        });
        break;
      case resType.informixService.value:
        assign(item, {
          logBackup: item.extendInfo?.logBackup
        });
        break;
      case resType.tdsqlInstance.value:
      case resType.tdsqlDistributedInstance.value:
      case resType.tidbDatabase.value:
        assign(item, {
          linkStatus: item.extendInfo?.linkStatus,
          parentName: item.environment?.name
        });
        break;
      case resType.tidbTable.value:
        assign(item, {
          linkStatus: item.extendInfo?.linkStatus
        });
        break;
      case resType.ObjectStorage.value:
        assign(item, {
          storageType: Number(item.extendInfo?.storageType)
        });
      case resType.ObjectSet.value:
        assign(item, {
          AK: item.extendInfo?.ak,
          storageType: Number(item.extendInfo?.storageType),
          protocol: item.extendInfo.useHttps
        });
        break;
      case resType.lightCloudGaussdbProject.value:
      case resType.lightCloudGaussdbInstance.value:
        assign(item, {
          status: item.extendInfo?.status,
          region: item.extendInfo?.region,
          pmAddress: item.extendInfo?.pmAddress,
          isAllowRestore: get(item, 'extendInfo.isAllowRestore', 'false'),
          instanceStatus: get(item, 'extendInfo.instanceStatus', '1')
        });
        break;
      case resType.ExchangeEmail.value:
        assign(item, {
          address: item.extendInfo?.PrimarySmtpAddress
        });
        break;
      default:
        break;
    }
  }
}
