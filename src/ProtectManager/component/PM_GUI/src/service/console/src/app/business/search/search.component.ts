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
import { Component, OnDestroy, OnInit, ViewChild } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { ActivatedRoute, Router } from '@angular/router';
import {
  ClustersApiService,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  FileLevelSearchManagementService,
  GlobalService,
  I18NService,
  LabelApiService,
  ResourceService,
  SearchRange
} from 'app/shared';
import {
  FilterType,
  NodeType,
  SearchResource
} from 'app/shared/consts/search.const';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  compact,
  each,
  find,
  forEach,
  includes,
  isArray,
  isBoolean,
  isEmpty,
  isString,
  map,
  omit,
  pick,
  reject,
  size,
  trim,
  union
} from 'lodash';
import { FileListComponent } from './file-list/file-list.component';
import { ResourceListComponent } from './resource-list/resource-list.component';
import { Subscription } from 'rxjs';
@Component({
  selector: 'aui-search',
  templateUrl: './search.component.html',
  styleUrls: ['./search.component.less']
})
export class SearchComponent implements OnInit, OnDestroy {
  includes = includes;
  isClickedCluster = false;
  searchKey = '';
  currentCluster = {} as any;
  advanceParams = {};
  searchType = SearchRange.COPIES;
  searchRange = SearchRange;
  isSearched = false;
  collapsed = false;
  startTime = 0;
  calcTimeDesc = '';
  formGroup: FormGroup;
  clusterOptions = [];
  resourceTypeValues = [];
  searchOptions = SearchRange;
  activeIndex = this.searchOptions.COPIES;
  pageSize = CommonConsts.PAGE_SIZE;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  searchTypeOptions = this.dataMapService
    .toArray('Global_Search_Type')
    .map(item => {
      item.isLeaf = true;
      return item;
    });
  nodeTypeOptions = this.dataMapService
    .toArray('Global_Search_Node_Type')
    .map(item => {
      item.isLeaf = true;
      return item;
    })
    .filter((item: any) => {
      return this.cookieService.isCloudBackup
        ? item.value !== NodeType.Link
        : item;
    });
  slaComplianceOptions = this.dataMapService
    .toArray('Sla_Compliance')
    .map(item => {
      item.isLeaf = true;
      return item;
    });
  protectOptions = this.dataMapService
    .toArray('Protection_Status')
    .map(item => {
      item.isLeaf = true;
      return item;
    });
  resourceTypeFilters = {
    [this.searchOptions.COPIES]: [
      {
        value: `${FilterType.Fileset}`,
        label: this.i18n.get('common_fileset_label')
      },
      {
        value: `${FilterType.Volume}`,
        label: this.i18n.get('protection_volume_label')
      },
      {
        value: `${FilterType.ClusterComputeResource},${FilterType.HostSystem},${FilterType.VimVirtualMachine}`,
        label: this.i18n.get('common_vmware_label')
      },
      {
        value: `${FilterType.CnwareVm}`,
        label: this.i18n.get('common_cnware_label')
      },
      {
        value: `${FilterType.HyperV}`,
        label: this.i18n.get('common_hyperv_label')
      },
      {
        value: `${FilterType.NutanixVm}`,
        label: this.i18n.get('common_nutanix_label'),
        hidden: true
      },
      {
        value: `${FilterType.HDFSFileset}`,
        label: this.i18n.get('HDFS')
      },
      {
        value: `${FilterType.NasFileSystem}`,
        label: this.i18n.get('common_nas_file_system_label'),
        hidden:
          this.appUtilsService.isDistributed || this.appUtilsService.isDecouple
      },
      {
        value: `${FilterType.NasShare}`,
        label: this.i18n.get('common_nas_shared_label')
      },
      {
        value: `${FilterType.Ndmp}`,
        label: this.i18n.get('protection_ndmp_protocol_label')
      },
      {
        value: `${FilterType.ObjectStorage}`,
        label: this.i18n.get('common_object_storage_label')
      },
      {
        value: `${FilterType.FusionCompute}`,
        label: this.i18n.get('common_fusion_compute_label')
      },
      {
        value: `${FilterType.FusionOneCompute}`,
        label: this.i18n.get('protection_fusionone_label')
      },
      {
        value: `${FilterType.HCSCloudHost}`,
        label: this.i18n.get('explore_hcs_cloud_container_label')
      },
      {
        value: `${FilterType.OpenstackCloudServer}`,
        label: this.i18n.get('common_open_stack_label')
      },
      {
        value: `${FilterType.APSCloudServer}`,
        label: this.i18n.get('protection_ali_cloud_label')
      }
    ],
    [this.searchOptions.RESOURCES]: [
      {
        value: `${DataMap.Resource_Type.DBBackupAgent.value},${DataMap.Resource_Type.VMBackupAgent.value},${DataMap.Resource_Type.UBackupAgent.value}`,
        label: this.i18n.get('common_host_label')
      },
      {
        value: `${DataMap.Resource_Type.fileset.value}`,
        label: this.i18n.get('common_fileset_label')
      },
      {
        value: `${DataMap.Resource_Type.volume.value}`,
        label: this.i18n.get('protection_volume_label')
      },
      {
        value: `${DataMap.Resource_Type.AntDB.value},${DataMap.Resource_Type.AntDBInstance.value},${DataMap.Resource_Type.AntDBClusterInstance.value}`,
        label: this.i18n.get('common_antdb_label'),
        hidden: true
      },
      {
        value: DataMap.Resource_Type.oracle.value,
        label: this.i18n.get('common_oracle_label')
      },
      {
        value: `${DataMap.Resource_Type.dbTwoCluster.value},${DataMap.Resource_Type.dbTwoClusterInstance.value},${DataMap.Resource_Type.dbTwoInstance.value},${DataMap.Resource_Type.dbTwoDatabase.value},${DataMap.Resource_Type.dbTwoTableSet.value}`,
        label: this.i18n.get('protection_db_two_label')
      },
      {
        value: `${DataMap.Resource_Type.SQLServerCluster.value},${DataMap.Resource_Type.SQLServerInstance.value},${DataMap.Resource_Type.SQLServerClusterInstance.value},${DataMap.Resource_Type.SQLServerGroup.value},${DataMap.Resource_Type.SQLServerDatabase.value}`,
        label: this.i18n.get('protection_sql_server_label')
      },
      {
        value: `${DataMap.Resource_Type.MySQLCluster.value},${DataMap.Resource_Type.MySQLInstance.value},${DataMap.Resource_Type.MySQLClusterInstance.value},${DataMap.Resource_Type.MySQLDatabase.value}`,
        label: this.i18n.get('protection_mysql_label')
      },
      {
        value: `${DataMap.Resource_Type.PostgreSQLCluster.value},${DataMap.Resource_Type.PostgreSQLInstance.value},${DataMap.Resource_Type.PostgreSQLClusterInstance.value}`,
        label: this.i18n.get('PostgreSQL')
      },
      {
        value: `${DataMap.Resource_Type.OpenGauss.value},${DataMap.Resource_Type.OpenGauss_instance.value},${DataMap.Resource_Type.OpenGauss_database.value}`,
        label: this.i18n.get('resource_sub_type_open_gauss_label')
      },
      {
        value: `${DataMap.Resource_Type.GaussDB_T.value},${DataMap.Resource_Type.gaussdbTSingle.value}`,
        label: this.i18n.get('resource_sub_type_gauss_dbt_label')
      },
      {
        value: `${DataMap.Resource_Type.DWS_Cluster.value},${DataMap.Resource_Type.DWS_Database.value},${DataMap.Resource_Type.DWS_Schema.value},${DataMap.Resource_Type.DWS_Table.value}`,
        label: this.i18n.get('common_dws_label')
      },
      {
        value: `${DataMap.Resource_Type.Redis.value}`,
        label: this.i18n.get('Redis')
      },
      {
        value: `${DataMap.Resource_Type.Dameng_cluster.value},${DataMap.Resource_Type.Dameng_singleNode.value}`,
        label: this.i18n.get('protection_dameng_label')
      },
      {
        value: `${DataMap.Resource_Type.ExchangeSingle.value},${DataMap.Resource_Type.ExchangeGroup.value},${DataMap.Resource_Type.ExchangeDataBase.value},${DataMap.Resource_Type.ExchangeDataBase.value},${DataMap.Resource_Type.ExchangeEmail.value}`,
        label: this.i18n.get('common_exchange_label')
      },
      {
        value: `${DataMap.Resource_Type.KingBaseCluster.value},${DataMap.Resource_Type.KingBaseInstance.value},${DataMap.Resource_Type.KingBaseClusterInstance.value}`,
        label: this.i18n.get('Kingbase')
      },
      {
        value: `${DataMap.Resource_Type.OceanBaseCluster.value},${DataMap.Resource_Type.OceanBaseTenant.value}`,
        label: this.i18n.get('OceanBase')
      },
      {
        value: `${DataMap.Resource_Type.ClickHouseCluster.value},${DataMap.Resource_Type.ClickHouseDatabase.value},${DataMap.Resource_Type.ClickHouseTableset.value},${DataMap.Resource_Type.ClickHouse.value}`,
        label: this.i18n.get('resource_sub_type_click_house_label')
      },
      {
        value: `${DataMap.Resource_Type.MongodbClusterInstance.value},${DataMap.Resource_Type.MongodbSingleInstance.value}`,
        label: this.i18n.get('MongoDB')
      },
      {
        value: `${DataMap.Resource_Type.goldendbCluter.value},${DataMap.Resource_Type.goldendbInstance.value}`,
        label: this.i18n.get('protection_goldendb_label')
      },
      {
        value: `${DataMap.Resource_Type.informixService.value},${DataMap.Resource_Type.informixInstance.value},${DataMap.Resource_Type.informixClusterInstance.value}`,
        label: this.i18n.get('Informix/GBase 8s')
      },
      {
        value: `${DataMap.Resource_Type.tdsqlCluster.value},${DataMap.Resource_Type.tdsqlInstance.value},${DataMap.Resource_Type.tdsqlDistributedInstance.value}`,
        label: this.i18n.get('TDSQL')
      },
      {
        value: `${DataMap.Resource_Type.tidbCluster.value},${DataMap.Resource_Type.tidbDatabase.value},${DataMap.Resource_Type.tidbTable.value}`,
        label: this.i18n.get('TiDB')
      },
      {
        value: `${DataMap.Resource_Type.saphanaInstance.value},${DataMap.Resource_Type.saphanaDatabase.value}`,
        label: this.i18n.get('SAP HANA')
      },
      {
        value: `${DataMap.Resource_Type.Saponoracle.value},${DataMap.Resource_Type.saponoracleDatabase.value}`,
        label: this.i18n.get('common_sap_on_oracle_label'),
        hidden: true
      },
      {
        value: `${DataMap.Resource_Type.generalDatabase.value}`,
        label: this.i18n.get('protection_general_database_label')
      },
      {
        value: `${DataMap.Resource_Type.clusterComputeResource.value},${DataMap.Resource_Type.hostSystem.value},${DataMap.Resource_Type.virtualMachine.value}`,
        label: this.i18n.get('common_vmware_label')
      },
      {
        value: `${DataMap.Resource_Type.cNwareCluster.value},${DataMap.Resource_Type.cNwareHost.value},${DataMap.Resource_Type.cNwareVm.value}`,
        label: this.i18n.get('common_cnware_label')
      },
      {
        value: `${DataMap.Resource_Type.FusionCompute.value},${DataMap.Resource_Type.fusionComputeCNA.value},${DataMap.Resource_Type.fusionComputeCluster.value},${DataMap.Resource_Type.fusionComputeVirtualMachine.value}`,
        label: this.i18n.get('FusionCompute')
      },
      {
        value: `${DataMap.Resource_Type.fusionOne.value},${DataMap.Resource_Type.fusionComputeCNA.value},${DataMap.Resource_Type.fusionComputeCluster.value},${DataMap.Resource_Type.fusionComputeVirtualMachine.value}`,
        label: this.i18n.get('protection_fusionone_label')
      },
      {
        value: `${DataMap.Resource_Type.hyperVCluster.value},${DataMap.Resource_Type.hyperVHost.value},${DataMap.Resource_Type.hyperVVm.value},${DataMap.Resource_Type.hyperVScvmm.value}`,
        label: this.i18n.get('common_hyperv_label')
      },
      {
        value: `${DataMap.Resource_Type.nutanixCluster.value},${DataMap.Resource_Type.nutanixHost.value},${DataMap.Resource_Type.nutanixVm.value}`,
        label: this.i18n.get('common_nutanix_label'),
        hidden: true
      },
      {
        value: `${DataMap.Resource_Type.Kubernetes.value},${DataMap.Resource_Type.KubernetesNamespace.value},${DataMap.Resource_Type.KubernetesStatefulset.value}`,
        label: this.i18n.get('protection_kubernetes_flexvolume_label')
      },
      {
        value: `${DataMap.Resource_Type.kubernetesClusterCommon.value},${DataMap.Resource_Type.kubernetesNamespaceCommon.value},${DataMap.Resource_Type.kubernetesDatasetCommon.value}`,
        label: this.i18n.get('protection_kubernetes_container_label')
      },
      {
        value: `${DataMap.Resource_Type.HDFS.value},${DataMap.Resource_Type.HDFSFileset.value}`,
        label: this.i18n.get('HDFS')
      },
      {
        value: `${DataMap.Resource_Type.HBase.value},${DataMap.Resource_Type.HBaseBackupSet.value}`,
        label: this.i18n.get('HBase')
      },
      {
        value: `${DataMap.Resource_Type.Hive.value},${DataMap.Resource_Type.HiveBackupSet.value}`,
        label: this.i18n.get('Hive')
      },
      {
        value: `${DataMap.Resource_Type.Elasticsearch.value},${DataMap.Resource_Type.ElasticsearchBackupSet.value}`,
        label: this.i18n.get('ElasticSearch')
      },
      {
        value: `${DataMap.Resource_Type.NASFileSystem.value}, ${DataMap.Resource_Type.ndmp.value}`,
        label: this.i18n.get('common_nas_file_system_label'),
        hidden:
          this.appUtilsService.isDistributed || this.appUtilsService.isDecouple
      },
      {
        value: `${DataMap.Resource_Type.NASShare.value}`,
        label: this.i18n.get('common_nas_shared_label')
      },
      {
        value: `${DataMap.Resource_Type.ObjectStorage.value},${DataMap.Resource_Type.ObjectSet.value}`,
        label: this.i18n.get('common_object_storage_label')
      },
      {
        value: `${DataMap.Resource_Type.openStackProject.value},${DataMap.Resource_Type.openStackCloudServer.value}`,
        label: this.i18n.get('common_open_stack_label')
      },
      {
        value: `${DataMap.Resource_Type.HCSCloudHost.value},${DataMap.Resource_Type.HCSProject.value},${DataMap.Resource_Type.HCSTenant.value}`,
        label: this.i18n.get('common_cloud_label')
      },
      {
        value: `${DataMap.Resource_Type.lightCloudGaussdbProject.value},${DataMap.Resource_Type.lightCloudGaussdbInstance.value}`,
        label: this.i18n.get('protection_light_cloud_gaussdb_label')
      },
      {
        value: `${DataMap.Resource_Type.gaussdbForOpengaussProject.value},${DataMap.Resource_Type.gaussdbForOpengaussInstance.value}`,
        label: this.i18n.get('protection_gaussdb_for_opengauss_label')
      },
      {
        value: `${DataMap.Resource_Type.APSZone.value},${DataMap.Resource_Type.APSCloudServer.value},${DataMap.Resource_Type.APSResourceSet.value}`,
        label: this.i18n.get('protection_ali_cloud_label')
      }
    ]
  };
  resourceTypeOption = this.resourceTypeFilters[this.activeIndex];
  searchPlaceHolder = this.i18n.get('common_file_directory_keyword_label');
  labelOptions = [];
  isCloudBackup = includes(
    [
      DataMap.Deploy_Type.cloudbackup.value,
      DataMap.Deploy_Type.cloudbackup2.value
    ],
    this.i18n.get('deploy_type')
  );
  labelRefreshStore$: Subscription = new Subscription();

  @ViewChild(ResourceListComponent, { static: false })
  resourceListComponent: ResourceListComponent;
  @ViewChild(FileListComponent, { static: false })
  fileListComponent: FileListComponent;

  constructor(
    public router: Router,
    public fb: FormBuilder,
    public i18n: I18NService,
    public route: ActivatedRoute,
    public dataMapService: DataMapService,
    public drawModalService: DrawModalService,
    public globalService: GlobalService,
    public cookieService: CookieService,
    public clusterApiService: ClustersApiService,
    private resourcesService: ResourceService,
    private fileLevelSearchRestApiService: FileLevelSearchManagementService,
    public appUtilsService: AppUtilsService,
    private labelApiService: LabelApiService
  ) {}

  ngOnDestroy() {
    this.advanceParams = {};
    const currentCluster =
      JSON.parse(
        decodeURIComponent(this.cookieService.get('currentCluster'))
      ) || {};
    if (currentCluster?.isAllCluster === true) {
      assign(currentCluster, {
        clusterId: DataMap.Cluster_Type.local.value,
        clusterType: DataMap.Cluster_Type.local.value
      });
      this.cookieService.set(
        'currentCluster',
        encodeURIComponent(JSON.stringify(currentCluster))
      );
    }

    this.cookieService.set(
      'currentCluster',
      encodeURIComponent(JSON.stringify(this.currentCluster))
    );
    this.labelRefreshStore$.unsubscribe();
  }

  ngOnInit() {
    if (this.isCloudBackup) {
      this.searchTypeOptions = reject(
        this.searchTypeOptions,
        v => v.value === SearchRange.LABELS
      );
    }
    assign(this.resourceTypeFilters, {
      [this.searchOptions.LABELS]: this.resourceTypeFilters[
        this.searchOptions.RESOURCES
      ]
    });
    const currentCluster = JSON.parse(
      decodeURIComponent(this.cookieService.get('currentCluster'))
    );

    if (!currentCluster || currentCluster.isAllcluster) {
      assign(currentCluster, {
        clusterId: DataMap.Cluster_Type.local.value,
        clusterType: DataMap.Cluster_Type.local.value
      });
    } else {
      this.currentCluster = currentCluster;
    }
    this.getStore();
    this.initForm();
    this.initResValues();
    this.getAdvanceParams();
    this.getLabelOptions(true);
  }

  getLabelOptions(akLoading?) {
    const extParams = {
      startPage: CommonConsts.PAGE_START_EXTRA,
      akLoading: akLoading
    };

    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.labelApiService.queryLabelUsingGET(params),
      res => {
        const arr = res?.map(item => {
          return {
            id: item.uuid,
            label: item.name,
            value: item.uuid,
            isLeaf: true
          };
        });
        this.labelOptions = arr;
      }
    );
  }

  getStore() {
    this.labelRefreshStore$ = this.globalService
      .getState('labelResearch')
      .subscribe(() => this.getLabelOptions(false));
  }

  initForm() {
    this.formGroup = this.fb.group({
      cluster: new FormControl(''),
      resourceType: new FormControl([]),
      sub_type: new FormControl([]),
      nodeType: new FormControl([]),
      file_path: new FormControl(''),
      resourceName: new FormControl(''),
      protection_status: new FormControl([]),
      sla_compliance: new FormControl([]),
      path: new FormControl(''),
      labelList: new FormControl([])
    });
    this.advanceParams = this.formGroup.value;
  }

  initResValues() {
    this.resourceTypeValues = [];
    const values = compact(map(this.resourceTypeOption, 'value'));
    each(values, value => {
      if (includes(value, ',')) {
        this.resourceTypeValues = union(
          this.resourceTypeValues,
          map(value.split(','), item => (isNaN(+item) ? item : +item))
        );
      } else {
        this.resourceTypeValues.push(isNaN(+value) ? value : +value);
      }
    });
  }

  getAdvanceParams() {
    this.formGroup.valueChanges.subscribe(res => {
      let diff = {};
      for (let key in res) {
        if (
          res[key] &&
          res[key] !== this.advanceParams[key] &&
          !includes(['file_path', 'resourceName', 'path', 'cluster'], key)
        ) {
          diff[key] = res[key];
        }
      }
      if (size(diff) === 1) {
        this.advanceSearch();
      } else if (size(diff) > 1) {
        this.advanceParams = res;
      }
    });
  }

  advanceSearch() {
    this.advanceParams = this.formGroup.value;
    this.search(this.advanceParams);
  }

  search(advanceParams) {
    if (isEmpty(this.searchKey)) {
      return;
    }

    this.globalService.emitStore({
      action:
        this.activeIndex === SearchRange.LABELS
          ? SearchRange.RESOURCES
          : this.activeIndex,
      state: {
        ...this.getParams(advanceParams)
      }
    });
  }

  showFilter() {
    return !isEmpty(find(this.clusterOptions, item => item.count > 0));
  }

  clearSearch() {
    this.isSearched = false;
    this.formGroup.patchValue({
      cluster: '',
      resourceType: [],
      sub_type: [],
      nodeType: [],
      file_path: '',
      resourceName: '',
      protection_status: [],
      sla_compliance: [],
      path: '',
      labelList: []
    });
    this.globalService.emitStore({
      action:
        this.activeIndex === SearchRange.LABELS
          ? SearchRange.RESOURCES
          : this.activeIndex,
      state: {}
    });
  }

  getParams(advanceParams?) {
    let formData = this.formGroup.value;
    if (this.activeIndex === this.searchOptions.COPIES) {
      formData = {
        ...pick(formData, ['resourceType'])
      };
    } else {
      formData = {
        ...pick(formData, ['sub_type'])
      };
    }

    let params = assign(
      { searchKey: trim(this.searchKey) },
      {
        ...formData
      }
    );

    if (advanceParams) {
      params = {
        ...params,
        ...omit(advanceParams, ['searchType', 'clusters'])
      };
    }
    map(params, (value: any, key: string) => {
      if (
        (isString(value) && isEmpty(trim(value))) ||
        (isArray(value) && !size(value))
      ) {
        delete params[key];
      } else if (isString(value)) {
        params[key] = trim(value);
      } else if (isArray(value)) {
        if (includes(value, '')) {
          if (SearchRange.COPIES === this.activeIndex) {
            let arrays = [];
            forEach(value, v => {
              if (!v) {
                return;
              }
              if (includes(v, ',')) {
                arrays = union(
                  arrays,
                  map(v.split(','), item => (isNaN(+item) ? item : +item))
                );
              } else {
                arrays.push(isBoolean(v) || isNaN(v) ? v : +v);
              }
            });
            params[key] = arrays;
          } else {
            params[key] = null;
          }
        } else if (isString(value)) {
          params[key] = trim(value);
        } else {
          let arrays = [];
          forEach(value, v => {
            if (includes(v, ',')) {
              arrays = union(
                arrays,
                map(v.split(','), item => (isNaN(+item) ? item : +item))
              );
            } else {
              arrays.push(isBoolean(v) || isNaN(v) ? v : +v);
            }
          });
          params[key] = arrays;
        }
      }
    });
    this.startTime = new Date().getTime();
    return params;
  }

  clearResourceType(type, event) {
    event.stopPropagation();
    if (type === this.searchOptions.RESOURCES) {
      this.formGroup.get('sub_type').setValue([]);
    } else {
      this.formGroup.get('resourceType').setValue([]);
    }
  }

  activeChange(index) {
    this.activeIndex = index;
    this.isSearched = false;
    this.searchType = index;
    this.resourceTypeOption = this.resourceTypeFilters[index];
    this.searchPlaceHolder =
      index === this.searchOptions.COPIES
        ? this.i18n.get('common_file_directory_keyword_label')
        : index === this.searchOptions.RESOURCES
        ? this.i18n.get('search_resource_keyword_label')
        : this.i18n.get('search_label_keyword_label');
    this.clearSearch();
    this.initResValues();
  }

  checkAllCopyFilter() {
    this.formGroup.patchValue({
      resourceType: [
        '',
        `${FilterType.ClusterComputeResource},${FilterType.HostSystem},${FilterType.VimVirtualMachine}`,
        `${FilterType.HDFSFileset}`,
        `${FilterType.NasFileSystem}`,
        `${FilterType.NasShare}`,
        `${FilterType.Ndmp}`,
        `${FilterType.ObjectStorage}`,
        `${FilterType.Fileset}`,
        `${FilterType.Volume}`,
        `${FilterType.FusionCompute}`,
        `${FilterType.FusionOneCompute}`,
        `${FilterType.HCSCloudHost}`,
        `${FilterType.OpenstackCloudServer}`,
        `${FilterType.APSCloudServer}`,
        `${FilterType.CnwareVm}`,
        `${FilterType.NutanixVm}`
      ]
    });
  }

  toggleSider() {
    this.collapsed = !this.collapsed;
  }

  calcTimeChange(event) {
    this.formGroup.get('cluster').setValue(event?.clusterId);
    this.isSearched = event.isSearched;
  }
}
