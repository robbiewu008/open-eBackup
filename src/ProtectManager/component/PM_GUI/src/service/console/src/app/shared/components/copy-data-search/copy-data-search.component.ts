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
import { Component, OnInit, TemplateRef, ViewChild } from '@angular/core';
import { MessageboxService, MessageService } from '@iux/live';
import { FileDetailComponent } from 'app/business/search/file-list/file-detail/file-detail.component';
import {
  CommonConsts,
  CookieService,
  CopiesService,
  CopyControllerService,
  DataMap,
  extendSummaryCopiesParams,
  FileLevelSearchManagementService,
  FilterType,
  I18NService,
  MODAL_COMMON,
  NodeType,
  RestoreType,
  SYSTEM_TIME
} from 'app/shared';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  RestoreParams,
  RestoreService
} from 'app/shared/services/restore.service';
import {
  assign,
  each,
  extend,
  first,
  forEach,
  includes,
  isEmpty,
  isNull,
  trim
} from 'lodash';
import { map } from 'rxjs/operators';
import { DownloadFlrFilesComponent } from '../download-flr-files/download-flr-files.component';
import { ExportFilesService } from '../export-files/export-files.component';

@Component({
  selector: 'aui-copy-data-search',
  templateUrl: './copy-data-search.component.html',
  styleUrls: ['./copy-data-search.component.less']
})
export class CopyDataSearchComponent implements OnInit {
  resourceData;
  orderBy = '';
  orderType = '';
  tableData = [];
  filterParams = {};
  nodeType = NodeType;
  _includes = includes;
  resourceType = DataMap.Resource_Type;
  startIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  total = CommonConsts.PAGE_TOTAL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  columns = [
    {
      key: 'nodeName',
      label: this.i18n.get('common_file_object_name_label')
    },
    {
      key: 'path',
      label: this.i18n.get('common_path_label')
    },
    {
      key: 'nodeType',
      label: this.i18n.get('common_type_label')
    },
    {
      showSort: true,
      key: 'nodeLastModifiedTime',
      label: this.i18n.get('common_modified_time_label')
    },
    {
      key: 'copy_count',
      label: this.i18n.get('common_number_of_index_copy_label')
    }
  ];

  timeZone = SYSTEM_TIME.timeZone;

  @ViewChild('fileDownloadCompletedTpl', { static: true })
  fileDownloadCompletedTpl: TemplateRef<any>;
  fileDownloadCompletedLabel = this.i18n.get(
    'common_file_download_completed_label'
  );

  downloadFlrFilesComponent = new DownloadFlrFilesComponent(
    this.exportFilesService
  );

  constructor(
    private i18n: I18NService,
    private cookieService: CookieService,
    private messageService: MessageService,
    private restoreService: RestoreService,
    private copiesApiService: CopiesService,
    private drawModalService: DrawModalService,
    private messageboxService: MessageboxService,
    private fileLevelSearchRestApiService: FileLevelSearchManagementService,
    private exportFilesService: ExportFilesService,
    private copyControllerService: CopyControllerService
  ) {}

  ngOnInit() {
    this.getFiles();
  }

  getFiles() {
    const request = this.getParams();
    this.fileLevelSearchRestApiService
      .fileSearch({
        request,
        akOperationTips: false
      })
      .subscribe((res: any) => {
        if (res.total > 1e5) {
          res.total = 1e5;
          this.startIndex === 0 &&
            this.messageboxService.info(
              this.i18n.get('search_file_max_num_label')
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
                DataMap.Global_Search_Resource_Type.LocalFileSystem.value,
                DataMap.Global_Search_Resource_Type.HDFSFileset.value,
                DataMap.Global_Search_Resource_Type.Fileset.value,
                DataMap.Global_Search_Resource_Type.Ndmp.value
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
              assign(item, {
                copy_count: first(result.records).copy_count
              });
            });
        });
        this.tableData = res.items;
        this.total = res.total;
      });
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

    this.filterParams = {
      resourceType: this.getResourceTypes(),
      searchKey: this.resourceData.searchKey,
      resourceId: trim(this.resourceData?.uuid)
    };

    each(this.filterParams, (value, key) => {
      if (isNull(value)) {
        delete this.filterParams[key];
      }
    });

    if (!isEmpty(this.filterParams)) {
      extend(params, this.filterParams);
    }

    return params;
  }

  getResourceTypes() {
    if (
      this.resourceData.resourceType === DataMap.Resource_Type.fileset.value
    ) {
      return [FilterType.Fileset, FilterType.DfsFileset];
    } else if (
      this.resourceData.resourceType ===
      DataMap.Resource_Type.virtualMachine.value
    ) {
      return [
        FilterType.ClusterComputeResource,
        FilterType.HostSystem,
        FilterType.VimVirtualMachine
      ];
    } else if (
      this.resourceData.resourceType ===
      DataMap.Resource_Type.NASFileSystem.value
    ) {
      return [FilterType.NasFileSystem];
    } else if (
      this.resourceData.resourceType === DataMap.Resource_Type.NASShare.value
    ) {
      return [FilterType.NasShare];
    } else if (
      this.resourceData.resourceType === DataMap.Resource_Type.HDFSFileset.value
    ) {
      return [FilterType.HDFSFileset];
    } else if (
      this.resourceData.resourceType ===
      DataMap.Resource_Type.LocalFileSystem.value
    ) {
      return [FilterType.LocalFileSystem];
    } else if (
      this.resourceData.resourceType ===
      DataMap.Resource_Type.FusionCompute.value
    ) {
      return [FilterType.FusionCompute];
    } else if (
      this.resourceData.resourceType ===
      DataMap.Resource_Type.HCSCloudHost.value
    ) {
      return [FilterType.HCSCloudHost];
    } else if (
      this.resourceData.resourceType === DataMap.Resource_Type.volume.value
    ) {
      return [FilterType.Volume];
    } else if (
      this.resourceData.resourceType === DataMap.Resource_Type.ObjectSet.value
    ) {
      return [FilterType.ObjectStorage];
    } else if (
      this.resourceData.resourceType ===
      DataMap.Resource_Type.openStackCloudServer.value
    ) {
      return [FilterType.OpenstackCloudServer];
    } else if (
      this.resourceData.resourceType === DataMap.Resource_Type.hyperVVm.value
    ) {
      return [FilterType.HyperV];
    } else if (
      this.resourceData.resourceType === DataMap.Resource_Type.ndmp.value
    ) {
      return [FilterType.Ndmp];
    }
  }

  getCopyList(item) {
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
            resourceType: this.resourceData.resourceType
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
            DataMap.Global_Search_Resource_Type.hcsCloudhost.value,
            DataMap.Global_Search_Resource_Type.ObjectStorage.value,
            DataMap.Global_Search_Resource_Type.OpenStack.value,
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
              DataMap.Global_Search_Resource_Type.hcsCloudhost.value,
              DataMap.Global_Search_Resource_Type.Volume.value,
              DataMap.Global_Search_Resource_Type.ObjectStorage.value,
              DataMap.Global_Search_Resource_Type.OpenStack.value,
              DataMap.Global_Search_Resource_Type.HypervVm.value
            ],
            data.resourceType
          ) ||
          includes(
            [DataMap.CopyData_generatedType.cloudArchival.value],
            data.generatedBy
          ),
        label: this.i18n.get('common_download_label'),
        onClick: () => this.download(data)
      }
    ];
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
          DataMap.Global_Search_Resource_Type.hcsCloudhost.value,
          DataMap.Global_Search_Resource_Type.OpenStack.value,
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

        let resType;
        switch (node.resourceType) {
          case DataMap.Global_Search_Resource_Type.VirtualMachine.value:
            resType = DataMap.Resource_Type.virtualMachine.value;
            break;
          case DataMap.Global_Search_Resource_Type.LocalFileSystem.value:
            resType = DataMap.Resource_Type.LocalFileSystem.value;
            break;
          case DataMap.Global_Search_Resource_Type.NASFileSystem.value:
            resType = DataMap.Resource_Type.NASFileSystem.value;
            break;
          case DataMap.Global_Search_Resource_Type.NASShare.value:
            resType = DataMap.Resource_Type.NASShare.value;
            break;
          case DataMap.Global_Search_Resource_Type.Ndmp.value:
            resType = DataMap.Resource_Type.ndmp.value;
            break;
          case DataMap.Global_Search_Resource_Type.HDFSFileset.value:
            resType = DataMap.Resource_Type.HDFSFileset.value;
            break;
          case DataMap.Global_Search_Resource_Type.fusionCompute.value:
            resType = DataMap.Resource_Type.FusionCompute.value;
            break;
          case DataMap.Global_Search_Resource_Type.hcsCloudhost.value:
            resType = DataMap.Resource_Type.HCSCloudHost.value;
            break;
          case DataMap.Global_Search_Resource_Type.ObjectStorage.value:
            resType = DataMap.Resource_Type.ObjectSet.value;
            break;
          case DataMap.Global_Search_Resource_Type.Fileset.value:
            resType = DataMap.Resource_Type.fileset.value;
            break;
          case DataMap.Global_Search_Resource_Type.OpenStack.value:
            resType = DataMap.Resource_Type.openStackCloudServer.value;
            break;
          case DataMap.Global_Search_Resource_Type.HypervVm.value:
            resType = DataMap.Resource_Type.hyperVVm.value;
            break;
        }

        const params: RestoreParams = {
          childResType: resType,
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
          DataMap.Resource_Type.HCSCloudHost.value,
          DataMap.Resource_Type.ObjectSet.value,
          DataMap.Resource_Type.fileset.value,
          DataMap.Resource_Type.hyperVVm.value,
          DataMap.Resource_Type.openStackCloudServer.value
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
    let memberEsn = '';
    if (node.esn) {
      memberEsn = node.esn;
    }

    this.downloadFlrFilesComponent.getRequestId(paths, node.copyId, memberEsn);
  }
}
