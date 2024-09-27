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
  TemplateRef,
  ViewChild
} from '@angular/core';
import { MessageboxService, MessageService } from '@iux/live';
import { FilesetsComponent } from 'app/business/protection/big-data/hdfs/filesets/filesets.component';
import { HuaWeiStackListComponent } from 'app/business/protection/cloud/huawei-stack/stack-list/huawei-stack-list.component';
import { DatabaseTemplateComponent } from 'app/business/protection/host-app/database-template/database-template.component';
import { FilesetComponent } from 'app/business/protection/host-app/fileset/fileset.component';
import { DoradoFileSystemComponent } from 'app/business/protection/storage/dorado-file-system/dorado-file-system.component';
import { NasSharedComponent } from 'app/business/protection/storage/nas-shared/nas-shared.component';
import { FusionListComponent } from 'app/business/protection/virtualization/fusion-compute/fusion-list/fusion-list.component';
import { BaseTableComponent } from 'app/business/protection/virtualization/virtualization-base/base-table/base-table.component';
import { VmListComponent } from 'app/business/protection/virtualization/vmware/vm-list/vm-list.component';
import {
  CommonConsts,
  CookieService,
  CopyControllerService,
  DataMap,
  DataMapService,
  extendSlaInfo,
  extendSummaryCopiesParams,
  FileLevelSearchManagementService,
  FilterType,
  GlobalService,
  I18NService,
  MODAL_COMMON,
  NodeType,
  Page_Size_Options,
  ProjectedObjectApiService,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType,
  RestoreManagerService as RestoreServiceApi,
  RestoreType,
  SearchRange,
  Table_Size,
  VirtualResourceService,
  WarningMessageService
} from 'app/shared';
import { CopiesService } from 'app/shared/api/services/copies.service';
import { DownloadFlrFilesComponent } from 'app/shared/components/download-flr-files/download-flr-files.component';
import { ExportFilesService } from 'app/shared/components/export-files/export-files.component';
import { WarningBatchConfirmsService } from 'app/shared/components/warning-batch-confirm/warning-batch-confirm.component';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { InfoMessageService } from 'app/shared/services/info-message.service';
import { ProtectService } from 'app/shared/services/protect.service';
import { RegisterService } from 'app/shared/services/register.service';
import { RememberColumnsService } from 'app/shared/services/remember-columns.service';
import { ResourceDetailService } from 'app/shared/services/resource-detail.service';
import {
  RestoreParams,
  RestoreService
} from 'app/shared/services/restore.service';
import { SetResourceTagService } from 'app/shared/services/set-resource-tag.service';
import { SlaService } from 'app/shared/services/sla.service';
import { TakeManualBackupService } from 'app/shared/services/take-manual-backup.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  each,
  extend,
  first,
  forEach,
  get,
  includes,
  isEmpty,
  isNull,
  mapValues,
  omit,
  size
} from 'lodash';
import { Subscription } from 'rxjs';
import { map } from 'rxjs/operators';
import { FileDetailComponent } from './file-detail/file-detail.component';

@Component({
  selector: 'aui-file-list',
  templateUrl: './file-list.component.html',
  styleUrls: ['./file-list.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class FileListComponent implements OnInit, OnDestroy {
  orderBy = '';
  orderType = '';
  columns = [];
  tableData = [];
  filterParams = {
    clusterId: '',
    clusterType: ''
  } as any;
  nodeType = NodeType;
  copyStore$: Subscription = new Subscription();
  startIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE_OPTIONS[1];
  total = CommonConsts.PAGE_TOTAL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;

  vmListComponent: VmListComponent;
  doradoFileSystemComponent: DoradoFileSystemComponent;
  nasSharedComponent: NasSharedComponent;
  filesetsComponent: FilesetsComponent;
  filesetComponent: FilesetComponent;
  fcListComponent: FusionListComponent;
  huaWeiStackListComponent: HuaWeiStackListComponent;
  databaseTemplateComponent: DatabaseTemplateComponent;
  baseTableComponent: BaseTableComponent;

  @Input() resourceTypeValues;
  @Output() calcTimeChange = new EventEmitter<any>();
  @ViewChild('fileDownloadCompletedTpl', { static: true })
  fileDownloadCompletedTpl: TemplateRef<any>;
  fileDownloadCompletedLabel = this.i18n.get(
    'common_file_download_completed_label'
  );

  downloadFlrFilesComponent = new DownloadFlrFilesComponent(
    this.exportFilesService
  );

  restoreChildResTypeMap = {
    [DataMap.Global_Search_Resource_Type.VirtualMachine.value]:
      DataMap.Resource_Type.virtualMachine.value,
    [DataMap.Global_Search_Resource_Type.LocalFileSystem.value]:
      DataMap.Resource_Type.LocalFileSystem.value,
    [DataMap.Global_Search_Resource_Type.NASFileSystem.value]:
      DataMap.Resource_Type.NASFileSystem.value,
    [DataMap.Global_Search_Resource_Type.NASShare.value]:
      DataMap.Resource_Type.NASShare.value,
    [DataMap.Global_Search_Resource_Type.Ndmp.value]:
      DataMap.Resource_Type.ndmp.value,
    [DataMap.Global_Search_Resource_Type.HDFSFileset.value]:
      DataMap.Resource_Type.HDFSFileset.value,
    [DataMap.Global_Search_Resource_Type.fusionCompute.value]:
      DataMap.Resource_Type.FusionCompute.value,
    [DataMap.Global_Search_Resource_Type.fusionOneCompute.value]:
      DataMap.Resource_Type.fusionOne.value,
    [DataMap.Global_Search_Resource_Type.hcsCloudhost.value]:
      DataMap.Resource_Type.HCSCloudHost.value,
    [DataMap.Global_Search_Resource_Type.ObjectStorage.value]:
      DataMap.Resource_Type.ObjectSet.value,
    [DataMap.Global_Search_Resource_Type.OpenStack.value]:
      DataMap.Resource_Type.openStackCloudServer.value,
    [DataMap.Global_Search_Resource_Type.ApsaraStack.value]:
      DataMap.Resource_Type.APSCloudServer.value,
    [DataMap.Global_Search_Resource_Type.CnwareVm.value]:
      DataMap.Resource_Type.cNwareVm.value,
    [DataMap.Global_Search_Resource_Type.HypervVm.value]:
      DataMap.Resource_Type.hyperVVm.value
  };

  constructor(
    private i18n: I18NService,
    private slaService: SlaService,
    private cookieService: CookieService,
    private messageService: MessageService,
    private protectService: ProtectService,
    private restoreServiceApi: RestoreServiceApi,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private copiesApiService: CopiesService,
    private globalService: GlobalService,
    private messageboxService: MessageboxService,
    private detailService: ResourceDetailService,
    private warningMessageService: WarningMessageService,
    private takeManualBackupService: TakeManualBackupService,
    private batchOperateService: BatchOperateService,
    private rememberColumnsService: RememberColumnsService,
    private projectedObjectApiService: ProjectedObjectApiService,
    private virtualResourceService: VirtualResourceService,
    private fileLevelSearchRestApiService: FileLevelSearchManagementService,
    public virtualScroll: VirtualScrollService,
    private cdr: ChangeDetectorRef,
    private restoreService: RestoreService,
    private registerService: RegisterService,
    private infoMessageService: InfoMessageService,
    private warningBatchConfirmsService: WarningBatchConfirmsService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    private copyControllerService: CopyControllerService,
    private exportFilesService: ExportFilesService,
    public appUtilsService: AppUtilsService,
    private setResourceTagService: SetResourceTagService
  ) {}

  ngOnDestroy() {
    this.copyStore$.unsubscribe();
  }

  ngOnInit() {
    this.getColumns();
    this.getStore();
    this.getComponent();
    this.virtualScroll.getScrollParam(
      400,
      Page_Size_Options.Three,
      Table_Size.Default,
      'search-file-table'
    );
  }

  getFiles() {
    const request = this.getParams();
    this.fileLevelSearchRestApiService
      .fileSearch({
        request,
        clustersId: this.filterParams['clusterId'],
        clustersType: this.filterParams['clusterType'],
        akOperationTips: false
      })
      .subscribe({
        next: (res: any) => {
          if (res.total > 1e5) {
            res.total = 1e5;
            this.startIndex === 0 &&
              this.messageService.info(
                this.i18n.get('search_file_max_num_label'),
                {
                  lvShowCloseButton: true,
                  lvMessageKey: 'fileMaxNumKey'
                }
              );
          }
          forEach(res.items, item => {
            const queryParams = {
              resource_id: item.resourceId,
              gn_range: [item.gnGte, item.gnLte],
              indexed: DataMap.CopyData_fileIndex.indexed.value
            };
            if (item.esn) {
              assign(queryParams, {
                device_esn: item.esn
              });
            }
            if (
              includes(
                [
                  DataMap.Global_Search_Resource_Type.NASFileSystem.value,
                  DataMap.Global_Search_Resource_Type.NASShare.value,
                  DataMap.Global_Search_Resource_Type.Ndmp.value,
                  DataMap.Global_Search_Resource_Type.LocalFileSystem.value,
                  DataMap.Global_Search_Resource_Type.HDFSFileset.value,
                  DataMap.Global_Search_Resource_Type.Fileset.value
                ],
                item.resourceType
              )
            ) {
              assign(queryParams, {
                chain_id: item['chainId'],
                generated_by: item['generatedBy']
              });
            }
            if (
              includes(
                [
                  DataMap.Global_Search_Resource_Type.ClusterComputeResource
                    .value,
                  DataMap.Global_Search_Resource_Type.HostSystem.value,
                  DataMap.Global_Search_Resource_Type.VirtualMachine.value,
                  DataMap.Global_Search_Resource_Type.fusionCompute.value,
                  DataMap.Global_Search_Resource_Type.fusionOneCompute.value,
                  DataMap.Global_Search_Resource_Type.hcsCloudhost.value,
                  DataMap.Global_Search_Resource_Type.OpenStack.value,
                  DataMap.Global_Search_Resource_Type.ApsaraStack.value,
                  DataMap.Global_Search_Resource_Type.CnwareVm.value,
                  DataMap.Global_Search_Resource_Type.HypervVm.value,
                  DataMap.Global_Search_Resource_Type.Volume.value
                ],
                item.resourceType
              )
            ) {
              assign(queryParams, {
                chain_id: item['chainId']
              });
            }
            this.copyControllerService
              .queryCopySummaryResourceV2({
                akLoading: false,
                akDoException: false,
                pageSize: 20,
                pageNo: 0,
                clustersId: this.filterParams['clusterId'],
                clustersType: this.filterParams['clusterType'],
                conditions: JSON.stringify(queryParams)
              })
              .subscribe(result => {
                each(result.records, item => {
                  extendSummaryCopiesParams(item);
                  assign(item, {
                    copy_count: item.copyCount,
                    resource_properties: item.resourceProperties
                  });
                });
                if (!!result.records.length) {
                  assign(item, {
                    copy_count: first(result.records).copy_count,
                    is_aggregation:
                      includes(
                        [
                          DataMap.Resource_Type.fileset.value,
                          DataMap.Resource_Type.NASShare.value
                        ],
                        first(result.records).resource_sub_type
                      ) &&
                      get(
                        JSON.parse(first(result.records).properties || '{}'),
                        'isAggregation'
                      ) === 'true'
                  });
                } else {
                  assign(item, {
                    copy_count: '--',
                    is_aggregation: true
                  });
                }
                this.cdr.detectChanges();
              });
          });
          this.tableData = res.items;
          this.total = res.total;
          this.cdr.detectChanges();
          this.calcTimeChange.emit({
            clusterId: this.filterParams?.clusterId,
            total: this.total,
            endTime: new Date().getTime(),
            isSearched: true
          });
        },
        error: () => {
          this.tableData = [];
          this.total = 0;
          this.cdr.detectChanges();
          this.calcTimeChange.emit({
            clusterId: this.filterParams?.clusterId,
            total: this.total,
            endTime: new Date().getTime(),
            isSearched: true
          });
        }
      });
  }

  getColumns() {
    this.columns = [
      {
        label: this.i18n.get('common_file_object_label'),
        children: [
          {
            key: 'nodeName',
            label: this.i18n.get('common_name_label')
          },
          {
            key: 'path',
            label: this.i18n.get('search_absolute_path_label')
          },
          {
            key: 'nodeType',
            label: this.i18n.get('common_type_label')
          },
          {
            showSort: true,
            key: 'nodeLastModifiedTime',
            label: this.i18n.get('common_modified_time_label')
          }
        ]
      },
      {
        label: this.i18n.get('common_resource_label'),
        children: [
          {
            key: 'resourceName',
            label: this.i18n.get('common_name_label')
          },
          {
            key: 'resourceType',
            label: this.i18n.get('common_type_label')
          }
        ]
      }
    ];
  }

  getParams() {
    const params = {
      pageNo: this.startIndex,
      pageSize: this.pageSize
    };

    if (!!this.orderBy) {
      extend(params, { orderBy: this.orderBy });
    }

    if (!!this.orderType) {
      extend(params, { orderType: this.orderType });
    }

    each(this.filterParams, (value, key) => {
      if (isNull(value)) {
        delete this.filterParams[key];
      }
      if (key === 'file_path') {
        assign(this.filterParams, {
          path: this.filterParams[key]
        });
        delete this.filterParams[key];
      }
    });

    if (!isEmpty(this.filterParams)) {
      extend(params, omit(this.filterParams, ['clusterId', 'clusterType']));
    }

    return params;
  }

  sortChange(source) {
    this.orderBy = source.key;
    this.orderType = source.direction;
    this.getFiles();
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.startIndex = page.pageIndex;
    this.getFiles();
  }

  restore(node) {
    const conditions = {
      resource_id: node.resourceId,
      gn_range: [node.gnGte, node.gnLte]
    };

    if (
      node.resourceType ===
        DataMap.Global_Search_Resource_Type.VirtualMachine.value &&
      node.copy_count > 1
    ) {
      assign(conditions, {
        indexed: DataMap.CopyData_fileIndex.indexed.value
      });
    }

    if (
      node.resourceType ===
      DataMap.Global_Search_Resource_Type.VirtualMachine.value
    ) {
      assign(conditions, { chain_id: node.chainId });
    }

    if (
      includes(
        [
          DataMap.Global_Search_Resource_Type.fusionCompute.value,
          DataMap.Global_Search_Resource_Type.fusionOneCompute.value,
          DataMap.Global_Search_Resource_Type.hcsCloudhost.value,
          DataMap.Global_Search_Resource_Type.ObjectStorage.value,
          DataMap.Global_Search_Resource_Type.OpenStack.value,
          DataMap.Global_Search_Resource_Type.ApsaraStack.value,
          DataMap.Global_Search_Resource_Type.CnwareVm.value,
          DataMap.Global_Search_Resource_Type.HypervVm.value
        ],
        node.resourceType
      )
    ) {
      assign(conditions, {
        indexed: DataMap.CopyData_fileIndex.indexed.value,
        chain_id: node.chainId
      });
    }

    if (
      includes(
        [
          DataMap.Global_Search_Resource_Type.NASFileSystem.value,
          DataMap.Global_Search_Resource_Type.NASShare.value,
          DataMap.Global_Search_Resource_Type.Ndmp.value,
          DataMap.Global_Search_Resource_Type.LocalFileSystem.value,
          DataMap.Global_Search_Resource_Type.HDFSFileset.value,
          DataMap.Global_Search_Resource_Type.Fileset.value
        ],
        node.resourceType
      )
    ) {
      assign(conditions, {
        indexed: DataMap.CopyData_fileIndex.indexed.value,
        chain_id: node.chainId,
        generated_by: node['generatedBy']
      });
    }

    this.copiesApiService
      .queryResourcesV1CopiesGet({
        pageNo: CommonConsts.PAGE_START,
        pageSize: this.pageSize,
        clustersId: this.filterParams['clusterId'],
        clustersType: this.filterParams['clusterType'],
        orders: ['-display_timestamp'],
        conditions: JSON.stringify(conditions)
      })
      .pipe(
        map(res => {
          return first(res.items);
        })
      )
      .subscribe(res => {
        if (
          isEmpty(res) &&
          node.resourceType ===
            DataMap.Global_Search_Resource_Type.VirtualMachine.value &&
          node.copy_count > 1
        ) {
          this.messageService.error(
            this.i18n.get('search_no_indexed_copy_label'),
            {
              lvMessageKey: 'no_indexed_copy_key',
              lvShowCloseButton: true
            }
          );
          return;
        }

        if (isEmpty(res)) {
          return;
        }

        const params: RestoreParams = {
          childResType: get(
            this.restoreChildResTypeMap,
            node.resourceType,
            DataMap.Resource_Type.fileset.value
          ),
          copyData: {}
        };

        if (
          node.resourceType ===
          DataMap.Global_Search_Resource_Type.VirtualMachine.value
        ) {
          params.copyData = {
            ...res,
            fileRestore: true,
            fineGrainedData: [
              node.path === '/'
                ? `/${node.nodeName}`
                : `${node.path}/${node.nodeName}`
            ]
          };
        } else {
          params.copyData = {
            ...res,
            isSearchRestore: true,
            searchRestorePath:
              node.path === '/'
                ? node.nodeType === NodeType.Folder
                  ? `/${node.nodeName}/`
                  : `/${node.nodeName}`
                : node.nodeType === NodeType.Folder
                ? `${node.path}/${node.nodeName}/`
                : `${node.path}/${node.nodeName}`
          };
        }
        this.startRestore(params);
      });
  }

  startRestore(params) {
    if (
      includes(
        [
          DataMap.Resource_Type.LocalFileSystem.value,
          DataMap.Resource_Type.NASFileSystem.value,
          DataMap.Resource_Type.NASShare.value,
          DataMap.Resource_Type.ndmp.value,
          DataMap.Resource_Type.HDFSFileset.value,
          DataMap.Resource_Type.FusionCompute.value,
          DataMap.Resource_Type.fusionOne.value,
          DataMap.Resource_Type.HCSCloudHost.value,
          DataMap.Resource_Type.ObjectSet.value,
          DataMap.Resource_Type.openStackCloudServer.value,
          DataMap.Resource_Type.cNwareVm.value,
          DataMap.Resource_Type.fileset.value,
          DataMap.Resource_Type.hyperVVm.value
        ],
        params.childResType
      )
    ) {
      this.restoreService.fileLevelRestore({
        header: this.i18n.get('common_restore_label'),
        childResType: params.childResType,
        copyData: params.copyData,
        restoreType: RestoreType.FileRestore
      });
    } else {
      this.restoreService.restore(params);
    }
  }

  download(node) {
    const paths = [
      node.path === '/'
        ? `${node.path}${node.nodeName}`
        : `${node.path}/${node.nodeName}`
    ];

    this.downloadFlrFilesComponent.getRequestId(paths, node.copyId, node.esn);
  }

  getResDetail(item) {
    const resourceType = this.getResourceType(item);
    const params = {
      pageSize: CommonConsts.PAGE_SIZE,
      pageNo: CommonConsts.PAGE_START,
      conditions: JSON.stringify({ uuid: item.resourceId })
    };

    switch (resourceType) {
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
      case DataMap.Resource_Type.HDFSFileset.value:
        this.filesetsComponent.initConfig();
        this.filesetsComponent.getResourceDetail({
          ...item,
          uuid: item.resourceId
        });
        break;
      case DataMap.Resource_Type.NASFileSystem.value:
      case DataMap.Resource_Type.LocalFileSystem.value:
      case DataMap.Resource_Type.ndmp.value:
        this.doradoFileSystemComponent.initConfig();
        this.doradoFileSystemComponent.getResourceDetail({
          ...item,
          uuid: item.resourceId
        });
        break;
      case DataMap.Resource_Type.NASShare.value:
        this.nasSharedComponent.initConfig();
        this.nasSharedComponent.getResourceDetail({
          ...item,
          uuid: item.resourceId
        });
        break;
      case DataMap.Resource_Type.fileset.value:
        this.protectedResourceApiService
          .ShowResource({
            resourceId: item.resourceId
          })
          .subscribe(res => {
            extendSlaInfo(res);
            this.filesetComponent.getDetail(res);
          });
        break;
      case DataMap.Resource_Type.FusionCompute.value:
      case DataMap.Resource_Type.fusionOne.value:
        this.protectedResourceApiService
          .ShowResource({
            resourceId: item.resourceId
          })
          .subscribe(res => {
            this.fcListComponent.tab = {
              id: ResourceType.VM,
              type: ResourceType.VM
            };
            extendSlaInfo(res);
            this.fcListComponent.getDetail(res);
          });
        break;
      case DataMap.Resource_Type.HCSCloudHost.value:
        this.protectedResourceApiService
          .ShowResource({
            resourceId: item.resourceId
          })
          .subscribe(res => {
            this.huaWeiStackListComponent.tab = {
              id: ResourceType.CLOUD_HOST,
              type: ResourceType.CLOUD_HOST,
              sub_type: 'HCSCloudHost'
            };
            extendSlaInfo(res);
            this.huaWeiStackListComponent.getDetail(res);
          });
        break;
      case DataMap.Resource_Type.volume.value:
        this.protectedResourceApiService
          .ShowResource({
            resourceId: item.resourceId
          })
          .subscribe(res => {
            this.databaseTemplateComponent.configParams = {
              activeIndex: DataMap.Resource_Type.volume.value,
              tableCols: [],
              tableOpts: []
            };
            extendSlaInfo(res);
            this.databaseTemplateComponent.getDetail(res);
          });
        break;
      case DataMap.Resource_Type.ObjectSet.value:
        this.protectedResourceApiService
          .ShowResource({
            resourceId: item.resourceId
          })
          .subscribe(res => {
            this.databaseTemplateComponent.configParams = {
              activeIndex: DataMap.Resource_Type.ObjectSet.value,
              tableCols: [],
              tableOpts: []
            };
            extendSlaInfo(res);
            this.databaseTemplateComponent.getDetail(res);
          });
        break;
      case DataMap.Resource_Type.hyperVVm.value:
        this.protectedResourceApiService
          .ShowResource({
            resourceId: item.resourceId
          })
          .subscribe(res => {
            this.baseTableComponent.subType =
              DataMap.Resource_Type.hyperVVm.value;
            this.baseTableComponent.initConfig();
            this.baseTableComponent.getResourceDetail(res);
          });
        break;
      case DataMap.Resource_Type.cNwareVm.value:
        this.protectedResourceApiService
          .ShowResource({
            resourceId: item.resourceId
          })
          .subscribe(res => {
            this.baseTableComponent.subType =
              DataMap.Resource_Type.cNwareVm.value;
            this.baseTableComponent.initConfig();
            this.baseTableComponent.getResourceDetail(res);
          });
        break;
      default:
        break;
    }
  }

  openDetail(res: any, optItems: any[]) {
    if (!res || !size(res.items)) {
      this.messageService.error(
        this.i18n.get('common_resource_not_exist_label'),
        {
          lvShowCloseButton: true,
          lvMessageKey: 'resNotExistMesageKey'
        }
      );
      return;
    }

    const item = first(res.items) as any;
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
        item.sub_type
      )
    ) {
      if (
        includes(
          [
            DataMap.Resource_Type.DBBackupAgent.value,
            DataMap.Resource_Type.VMBackupAgent.value
          ],
          item.sub_type
        )
      ) {
        item.sub_type = DataMap.Resource_Type.ABBackupClient.value;
      }
      this.detailService.openDetailModal(item.sub_type, {
        data: { ...item, optItems }
      });
    }
  }

  getCopyList(item) {
    if (item.copy_count === '--') {
      return;
    }
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'search-file-detail',
        lvWidth: MODAL_COMMON.largeWidth,
        lvContent: FileDetailComponent,
        lvHeader: this.i18n.get('common_number_of_index_copy_label'),
        lvComponentParams: {
          data: {
            ...item,
            name: item.nodeName,
            resourceType: this.getResourceType(item)
          }
        },
        lvFooter: [
          {
            label: this.i18n.get('common_close_label'),
            onClick: modal => modal.close()
          }
        ]
      })
    );
  }

  getResourceType(item) {
    let resourceType;
    const globalResourceType = this.dataMapService.getValueConfig(
      'Global_Search_Resource_Type',
      item.resourceType
    );
    const type = globalResourceType.value;
    switch (type) {
      case FilterType.DbbackupAgent:
      case FilterType.VmbackupAgent:
      case FilterType.AbbackupClient:
        resourceType = DataMap.Resource_Type.ABBackupClient.value;
        break;
      case FilterType.Fileset:
      case FilterType.DfsFileset:
        resourceType = DataMap.Resource_Type.fileset.value;
        break;
      case FilterType.Volume:
        resourceType = DataMap.Resource_Type.volume.value;
        break;
      case FilterType.Oracle:
        resourceType = DataMap.Resource_Type.oracle.value;
        break;
      case FilterType.SqlServer:
        resourceType = DataMap.Resource_Type.SQLServer.value;
        break;
      case FilterType.DB2:
        resourceType = DataMap.Resource_Type.DB2.value;
        break;
      case FilterType.Mysql:
        resourceType = DataMap.Resource_Type.MySQL.value;
        break;
      case FilterType.ClusterComputeResource:
      case FilterType.HostSystem:
      case FilterType.VimVirtualMachine:
        resourceType = DataMap.Resource_Type.virtualMachine.value;
        break;
      case FilterType.NasFileSystem:
        resourceType = DataMap.Resource_Type.NASFileSystem.value;
        break;
      case FilterType.NasShare:
        resourceType = DataMap.Resource_Type.NASShare.value;
        break;
      case FilterType.Ndmp:
        resourceType = DataMap.Resource_Type.ndmp.value;
        break;
      case FilterType.LocalFileSystem:
        resourceType = DataMap.Resource_Type.LocalFileSystem.value;
        break;
      case FilterType.HDFSFileset:
        resourceType = DataMap.Resource_Type.HDFSFileset.value;
        break;
      case FilterType.FusionCompute:
        resourceType = DataMap.Resource_Type.FusionCompute.value;
        break;
      case FilterType.FusionOneCompute:
        resourceType = DataMap.Resource_Type.fusionOne.value;
        break;
      case FilterType.HCSCloudHost:
        resourceType = DataMap.Resource_Type.HCSCloudHost.value;
        break;
      case FilterType.ObjectStorage:
        resourceType = DataMap.Resource_Type.ObjectSet.value;
        break;
      case FilterType.OpenstackCloudServer:
        resourceType = DataMap.Resource_Type.openStack.value;
        break;
      case FilterType.APSCloudServer:
        resourceType = DataMap.Resource_Type.APSCloudServer.value;
        break;
      case FilterType.CnwareVm:
        resourceType = DataMap.Resource_Type.cNwareVm.value;
        break;
      case FilterType.HyperV:
        resourceType = DataMap.Resource_Type.hyperVVm.value;
        break;
      default:
        break;
    }
    return resourceType;
  }

  optsCallback = data => {
    return this.getOptItems(data);
  };

  getOptItems(data) {
    return [
      {
        id: 'restore',
        label: this.i18n.get('common_restore_label'),
        disabled: data.copy_count === '--',
        hidden: !includes(
          [
            DataMap.Global_Search_Resource_Type.Fileset.value,
            DataMap.Global_Search_Resource_Type.VirtualMachine.value,
            DataMap.Global_Search_Resource_Type.LocalFileSystem.value,
            DataMap.Global_Search_Resource_Type.NASFileSystem.value,
            DataMap.Global_Search_Resource_Type.NASShare.value,
            DataMap.Global_Search_Resource_Type.Ndmp.value,
            DataMap.Global_Search_Resource_Type.HDFSFileset.value,
            DataMap.Global_Search_Resource_Type.fusionCompute.value,
            DataMap.Global_Search_Resource_Type.fusionOneCompute.value,
            DataMap.Global_Search_Resource_Type.hcsCloudhost.value,
            DataMap.Global_Search_Resource_Type.ObjectStorage.value,
            DataMap.Global_Search_Resource_Type.OpenStack.value,
            DataMap.Global_Search_Resource_Type.ApsaraStack.value,
            DataMap.Global_Search_Resource_Type.CnwareVm.value,
            DataMap.Global_Search_Resource_Type.HypervVm.value
          ],
          data.resourceType
        ),
        onClick: () => this.restore(data)
      },
      {
        id: 'download',
        hidden:
          includes(
            [
              DataMap.Global_Search_Resource_Type.LocalFileSystem.value,
              DataMap.Global_Search_Resource_Type.HDFSFileset.value,
              DataMap.Global_Search_Resource_Type.fusionCompute.value,
              DataMap.Global_Search_Resource_Type.fusionOneCompute.value,
              DataMap.Global_Search_Resource_Type.hcsCloudhost.value,
              DataMap.Global_Search_Resource_Type.Volume.value,
              DataMap.Global_Search_Resource_Type.ObjectStorage.value,
              DataMap.Global_Search_Resource_Type.OpenStack.value,
              DataMap.Global_Search_Resource_Type.ApsaraStack.value,
              DataMap.Global_Search_Resource_Type.CnwareVm.value,
              DataMap.Global_Search_Resource_Type.HypervVm.value,
              DataMap.Global_Search_Resource_Type.Ndmp.value
            ],
            data.resourceType
          ) ||
          includes(
            [DataMap.CopyData_generatedType.cloudArchival.value],
            data.generatedBy
          ),
        label: this.i18n.get('common_download_label'),
        disabled: data.copy_count === '--' || data.is_aggregation,
        onClick: () => this.download(data)
      }
    ];
  }

  getStore() {
    this.copyStore$ = this.globalService
      .getState(SearchRange.COPIES)
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

        if (isEmpty(params.resourceType)) {
          params.resourceType = [
            FilterType.ClusterComputeResource,
            FilterType.HostSystem,
            FilterType.VimVirtualMachine,
            FilterType.HDFSFileset,
            FilterType.NasFileSystem,
            FilterType.NasShare,
            FilterType.Ndmp,
            FilterType.Fileset,
            FilterType.Volume,
            FilterType.FusionCompute,
            FilterType.FusionOneCompute,
            FilterType.HCSCloudHost,
            FilterType.ObjectStorage,
            FilterType.OpenstackCloudServer,
            FilterType.APSCloudServer,
            FilterType.CnwareVm,
            FilterType.HyperV
          ].filter(item => {
            return (
              ((this.appUtilsService.isDistributed ||
                this.appUtilsService.isDecouple) &&
                !includes([FilterType.NasFileSystem], item)) ||
              (!this.appUtilsService.isDistributed &&
                !this.appUtilsService.isDecouple)
            );
          });
        }

        this.filterParams = !this.cookieService.isCloudBackup
          ? omit(params, ['sub_type'])
          : assign(params, {
              resourceType: [FilterType.LocalFileSystem]
            });

        this.getFiles();
      });
  }

  getComponent() {
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
      this.messageService,
      this.drawModalService,
      this.virtualScroll,
      this.cookieService,
      this.cdr,
      this.detailService,
      this.infoMessageService,
      this.takeManualBackupService,
      this.protectedResourceApiService,
      this.protectedEnvironmentApiService,
      this.setResourceTagService
    );

    this.nasSharedComponent = new NasSharedComponent(
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
      this.setResourceTagService
    );

    this.filesetsComponent = new FilesetsComponent(
      this.i18n,
      this.cdr,
      this.slaService,
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
      this.setResourceTagService
    );

    this.filesetComponent = new FilesetComponent(
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

    this.huaWeiStackListComponent = new HuaWeiStackListComponent(
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

    this.databaseTemplateComponent = new DatabaseTemplateComponent(
      this.i18n,
      this.cdr,
      this.slaService,
      this.dataMapService,
      this.protectService,
      this.messageService,
      this.registerService,
      this.drawModalService,
      this.virtualScroll,
      this.detailService,
      this.batchOperateService,
      this.warningMessageService,
      this.takeManualBackupService,
      this.protectedResourceApiService,
      this.setResourceTagService
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
      this.setResourceTagService
    );
  }
}
