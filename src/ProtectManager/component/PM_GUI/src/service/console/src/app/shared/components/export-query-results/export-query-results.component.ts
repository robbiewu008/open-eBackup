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
  OnDestroy,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { MessageService } from '@iux/live';
import {
  ApiExportFilesApiService as ExportFileApiService,
  CAPACITY_UNIT,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  getPermissionMenuItem,
  GROUP_COMMON,
  I18NService,
  OperateItems,
  WarningMessageService,
  MultiCluster
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import {
  assign,
  cloneDeep,
  each,
  find,
  includes,
  isEmpty,
  isNull,
  map,
  trim
} from 'lodash';
import { Subject, Subscription, timer } from 'rxjs';
import { finalize, switchMap, takeUntil } from 'rxjs/operators';
import { TableCols, TableConfig, TableData } from '../pro-table';

@Component({
  selector: 'aui-export-query-results',
  templateUrl: './export-query-results.component.html',
  styleUrls: ['./export-query-results.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class ExportQueryResultsComponent implements OnInit, OnDestroy {
  data;
  orderBy;
  orderType;
  fileName;
  timeSub$: Subscription;
  exportQueryStatus = DataMap.exportLogStatus;
  filterParams: any = {};
  destroy$ = new Subject();
  tableConfig: TableConfig;
  tableData: TableData;

  dataMap = DataMap;
  unitconst = CAPACITY_UNIT;
  total = CommonConsts.PAGE_TOTAL;
  startPage = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE_SMALL;
  sizeOptions = CommonConsts.SIMPLE_PAGE_SIZE_OPTIONS;
  disableBtn = true;
  selection = [];
  nonClient = [
    DataMap.Deploy_Type.cyberengine.value,
    DataMap.Deploy_Type.hyperdetect.value,
    DataMap.Deploy_Type.cloudbackup.value,
    DataMap.Deploy_Type.cloudbackup2.value
  ].includes(this.i18n.get('deploy_type'));
  isCyberEngine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;
  isDataBackup = this.appUtilsService.isDataBackup;
  exportQueryTips: string;

  columns = [
    {
      key: 'nodeName',
      label: this.i18n.get('common_home_node_name_label'),
      hidden: !this.isDataBackup
    },
    {
      key: 'fileName',
      label: this.i18n.get('common_name_label')
    },
    {
      key: 'type',
      label: this.i18n.get('common_type_label'),
      filter: true,
      filterMap: this.isCyberEngine
        ? this.dataMapService.toArray('exportCyberLogType')
        : this.dataMapService.toArray('exportLogType').filter(item => {
            if (
              includes(
                [
                  DataMap.Deploy_Type.cloudbackup.value,
                  DataMap.Deploy_Type.cloudbackup2.value
                ],
                this.i18n.get('deploy_type')
              )
            ) {
              return !includes(
                [
                  DataMap.exportLogType.agentLog.value,
                  DataMap.exportLogType.detectionReport.value,
                  DataMap.exportLogType.config.value
                ],
                item.value
              );
            } else if (
              includes(
                [DataMap.Deploy_Type.hyperdetect.value],
                this.i18n.get('deploy_type')
              )
            ) {
              return !includes(
                [
                  DataMap.exportLogType.agentLog.value,
                  DataMap.exportLogType.config.value
                ],
                item.value
              );
            } else if (this.appUtilsService.isDecouple) {
              return !includes(
                [DataMap.exportLogType.config.value],
                item.value
              );
            } else {
              return true;
            }
          })
    },
    {
      key: 'size',
      label: this.i18n.get('common_size_label')
    },
    {
      key: 'status',
      label: this.i18n.get('common_status_label'),
      filter: true,
      filterMap: this.dataMapService.toArray('Export_Query_Status')
    },
    {
      key: 'createTime',
      label: this.i18n.get('common_create_time_label')
    },
    {
      key: 'expireTime',
      label: this.i18n.get('common_expriration_time_label')
    }
  ];

  groupCommon = GROUP_COMMON;

  @ViewChild('sizeTpl', { static: true })
  sizeTpl: TemplateRef<any>;
  @ViewChild('statusTpl', { static: true })
  statusTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private messageService: MessageService,
    private cookieService: CookieService,
    private dataMapService: DataMapService,
    private appUtilsService: AppUtilsService,
    private warningMessageService: WarningMessageService,
    private exportFilesApi: ExportFileApiService,
    private batchOperateService: BatchOperateService
  ) {}

  ngOnDestroy() {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  onChange() {
    this.getDatas();
  }

  ngOnInit() {
    this.initExportQueryTips();
    this.getDatas();
    this.initTable();
  }

  initExportQueryTips() {
    if (this.isDataBackup || this.appUtilsService.isDistributed) {
      this.exportQueryTips = this.i18n.get('common_export_query_desc_label');
    } else if (this.appUtilsService.isDecouple) {
      this.exportQueryTips = this.i18n.get(
        'common_export_query_other_desc_label'
      );
    } else {
      this.exportQueryTips = this.i18n.get(
        'common_export_query_special_desc_label'
      );
    }
  }

  initTable() {
    const cols: TableCols[] = [
      {
        key: 'agentName',
        name: this.i18n.get('common_name_label')
      },
      {
        key: 'agentEndpoint',
        name: this.i18n.get('common_ip_address_label')
      },
      {
        key: 'size',
        name: this.i18n.get('common_size_label'),
        cellRender: this.sizeTpl
      },
      {
        key: 'status',
        name: this.i18n.get('common_status_label'),
        cellRender: this.statusTpl
      },
      {
        key: 'operate',
        width: '120px',
        name: this.i18n.get('common_operation_label'),
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 2,
            items: [
              {
                id: 'export',
                label: this.i18n.get('common_download_label'),
                disableCheck: data => {
                  return (
                    data[0].status !== DataMap.exportLogStatus.success.value
                  );
                },
                onClick: data => {
                  this.exportSubItem(data[0]);
                }
              }
            ]
          }
        }
      }
    ];

    this.tableConfig = {
      table: {
        columns: cols,
        colDisplayControl: false,
        size: 'small',
        fetchData: data => {
          this.getSubData(data);
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  getSubData(data, loading = true) {
    this.exportFilesApi
      .getAgentExportLogFile({ id: data.id, akLoading: loading })
      .subscribe(res => {
        each(res.items, item => {
          if (!isNull(item?.errorParams)) {
            item.errorParams = JSON.parse(item.errorParams);
          }
        });
        const logData = [];
        each(res.items, item => {
          logData.push({
            ...item,
            agentName: item.agentName,
            parentId: data.id
          });
        });
        assign(data, {
          tableData: {
            data: logData,
            total: res.total
          }
        });
        this.cdr.detectChanges();
      });
  }

  getDatas() {
    if (this.timeSub$) {
      this.timeSub$.unsubscribe();
    }
    this.timeSub$ = timer(0, CommonConsts.TIME_INTERVAL)
      .pipe(
        switchMap(index => {
          const params = this.getParams();
          return this.exportFilesApi.GetExportFiles({
            ...params,
            akLoading: !index
          });
        }),
        takeUntil(this.destroy$)
      )
      .subscribe(res => {
        each(res.items, (item: any) => {
          if (find(this.data, data => data.expand && data.id === item.id)) {
            assign(item, { expand: true });
            this.getSubData(item, false);
          }
        });
        this.total = res.total;
        this.data = res.items;

        this.cdr.detectChanges();
      });
  }

  getParams() {
    const params = {
      pageNo: this.startPage + 1,
      pageSize: this.pageSize
    };

    each(this.filterParams, (value, key) => {
      if (isEmpty(value)) {
        delete this.filterParams[key];
      }
    });

    if (!isEmpty(this.filterParams)) {
      assign(params, {
        ...this.filterParams
      });
    }

    if (this.orderBy && this.orderType) {
      assign(params, {
        orderBy: this.orderBy,
        orderType: this.orderType
      });
    }

    return params;
  }

  pageChange = event => {
    this.pageSize = event.pageSize;
    this.startPage = event.pageIndex;
    this.getDatas();
  };

  sortChange(source) {
    this.orderBy = source.key;
    this.orderType = source.direction;
    this.getDatas();
  }

  filterChange(e) {
    if (e.key === 'type') {
      assign(this.filterParams, {
        types: e.value
      });
    } else if (e.key === 'status') {
      assign(this.filterParams, {
        statuses: e.value
      });
    } else {
      assign(this.filterParams, {
        [e.key]: e.value
      });
    }
    this.getDatas();
  }

  searchByName(value) {
    assign(this.filterParams, {
      fileName: trim(value)
    });
    this.getDatas();
  }

  optsCallback = data => {
    const menus = [
      {
        id: 'download',
        disabled: !includes(
          [
            DataMap.Export_Query_Status.success.value,
            DataMap.Export_Query_Status.partSucess.value
          ],
          data.status
        ),
        label: this.i18n.get('common_download_label'),
        permission: OperateItems.DownloadExportQuery,
        onClick: (d: any) => {
          this.exportItem(data);
        }
      },
      {
        id: 'delete',
        disabled: includes(
          [DataMap.Export_Query_Status.generating.value],
          data.status
        ),
        label: this.i18n.get('common_delete_label'),
        permission: OperateItems.DeleteExportQuery,
        onClick: (d: any) => {
          this.deleteItem(data);
        }
      }
    ];
    return getPermissionMenuItem(menus, this.cookieService.role);
  };

  deleteItem(item) {
    this.warningMessageService.create({
      content: this.i18n.get('common_export_query_delete_label', [
        item.fileName
      ]),
      onOK: () =>
        this.exportFilesApi
          .DeleteExportFiles({
            id: item.id,
            memberEsn: item.esn || ''
          })
          .subscribe(res => this.getDatas())
    });
  }

  selectionChange() {
    this.disableBtn =
      isEmpty(this.selection) ||
      !isEmpty(
        find(this.selection, item =>
          includes([DataMap.Export_Query_Status.generating.value], item.status)
        )
      );
  }

  batchDelete() {
    this.warningMessageService.create({
      content: this.i18n.get('common_export_query_delete_label', [
        map(this.selection, 'fileName').join(',')
      ]),
      onOK: () =>
        this.batchOperateService.selfGetResults(
          item => {
            return this.exportFilesApi.DeleteExportFiles({
              id: item.id,
              akDoException: false,
              akOperationTips: false,
              akLoading: false,
              memberEsn: item.esn || ''
            });
          },
          map(cloneDeep(this.selection), item => {
            return assign(item, {
              name: item.fileName
            });
          }),
          () => {
            this.selection = [];
            this.getDatas();
          }
        )
    });
  }

  exportItem(item) {
    const params = {
      id: item.id,
      akLoading: false
    };
    if (this.isDataBackup) {
      assign(params, { memberEsn: item.esn });
    }
    if (MultiCluster.isMulti) {
      this.messageService.info(
        this.i18n.get('common_file_download_processing_label'),
        {
          lvDuration: 0,
          lvShowCloseButton: true,
          lvMessageKey: 'downloadKey'
        }
      );
      this.exportFilesApi
        .DownloadExportFile(params)
        .pipe(finalize(() => this.messageService.destroy('downloadKey')))
        .subscribe((res: any) => {
          const bf = new Blob([res], {
            type: 'application/zip'
          });
          this.appUtilsService.downloadFile(`${item.fileName}.zip`, bf);
        });
    } else {
      this.appUtilsService.downloadUseAElement(
        `/v1/export-files/${encodeURIComponent(
          String(params.id)
        )}/action/download`,
        `${item.fileName}.zip`
      );
    }
    setTimeout(() => this.getDatas(), 2e3);
  }

  exportSubItem(data) {
    const fileName = data?.agentEndpoint
      ? `${data.agentEndpoint}_${data.uuid}`
      : `${data.uuid}`;
    const parentNode = find(this.data, item => item.id === data.parentId);
    if (MultiCluster.isMulti && parentNode?.esn) {
      this.messageService.info(
        this.i18n.get('common_file_download_processing_label'),
        {
          lvDuration: 0,
          lvShowCloseButton: true,
          lvMessageKey: 'downloadKey'
        }
      );
      this.exportFilesApi
        .DownloadExportFile({
          id: data?.parentId,
          subId: data?.uuid,
          memberEsn: parentNode?.esn,
          akLoading: false
        })
        .pipe(finalize(() => this.messageService.destroy('downloadKey')))
        .subscribe((res: any) => {
          const bf = new Blob([res], {
            type: 'application/zip'
          });
          const fileName = data?.agentEndpoint
            ? `${data.agentEndpoint}_${data.uuid}`
            : `${data.uuid}`;
          this.appUtilsService.downloadFile(`${fileName}.zip`, bf);
        });
    } else {
      this.appUtilsService.downloadUseAElement(
        `/v1/export-files/${encodeURIComponent(
          data?.parentId
        )}/action/download?subId=${data?.uuid}`,
        `${fileName}.zip`
      );
    }
  }

  trackById = (index, item) => {
    return item.id;
  };
}
