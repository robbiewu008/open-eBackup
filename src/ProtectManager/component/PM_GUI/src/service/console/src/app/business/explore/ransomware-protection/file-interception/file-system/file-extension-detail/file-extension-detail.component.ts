import { AfterViewInit, Component, OnInit, ViewChild } from '@angular/core';
import { CommonConsts, DataMap, DataMapService, I18NService } from 'app/shared';
import { FsFileExtensionFilterManagementService } from 'app/shared/api/services/fs-file-extension-filter-management.service';
import {
  Filters,
  ProTableComponent,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { InfoMessageService } from 'app/shared/services/info-message.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { assign, includes, isEmpty, map, size } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-file-extension-detail',
  templateUrl: './file-extension-detail.component.html',
  styleUrls: ['./file-extension-detail.component.less']
})
export class FileExtensionDetailComponent implements OnInit, AfterViewInit {
  rowData;
  updateOperation;
  tableConfig: TableConfig;
  tableData: TableData;

  selectionData = [];

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private appUtilsService: AppUtilsService,
    private virtualScroll: VirtualScrollService,
    private infoMessageService: InfoMessageService,
    private fsFileExtensionFilterService: FsFileExtensionFilterManagementService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.virtualScroll.getScrollParam(260);
    this.initConfig();
    this.getAssociated();
    window.addEventListener('resize', () => {
      this.virtualScroll.getScrollParam(260);
      this.dataTable.setTableScroll(this.virtualScroll.scrollParam);
    });
  }

  initConfig() {
    this.tableConfig = {
      table: {
        compareWith: 'fileExtensionName',
        columns: [
          {
            key: 'fileExtensionName',
            name: this.i18n.get('explore_file_extension_label'),
            filter: {
              type: 'search',
              filterMode: 'contains'
            }
          },
          {
            key: 'fileExtensionType',
            name: this.i18n.get('common_type_label'),
            filter: {
              type: 'select',
              isMultiple: true,
              showCheckAll: true,
              options: this.dataMapService.toArray('File_Extension_Type')
            },
            cellRender: {
              type: 'status',
              config: this.dataMapService.toArray('File_Extension_Type')
            }
          },
          {
            key: 'importStatus',
            hidden: this.updateOperation,
            name: this.i18n.get(
              'explore_file_extension_associate_status_label'
            ),
            filter: {
              type: 'select',
              isMultiple: true,
              showCheckAll: true,
              options: this.dataMapService
                .toArray('File_Extension_Import_Status')
                .filter(
                  item =>
                    !includes(
                      [DataMap.File_Extension_Import_Status.deleteError.value],
                      item.value
                    )
                )
            },
            cellRender: {
              type: 'status',
              config: this.dataMapService.toArray(
                'File_Extension_Import_Status'
              )
            }
          },
          {
            key: 'updateTime',
            hidden: this.updateOperation,
            name: this.i18n.get('explore_add_tiem_label'),
            sort: true
          },
          {
            key: 'createTime',
            hidden: !this.updateOperation,
            name: this.i18n.get('common_create_time_label'),
            sort: true
          }
        ],
        colDisplayControl: false,
        scroll: this.virtualScroll.scrollParam,
        rows: this.updateOperation
          ? {
              selectionMode: 'multiple',
              selectionTrigger: 'selector',
              showSelector: true
            }
          : null,
        fetchData: (filter: Filters, args) => {
          this.getData(filter, args);
        },
        selectionChange: selection => {
          this.selectionData = selection;
        }
      },
      pagination: {
        showPageSizeOptions: true,
        pageSizeOptions: CommonConsts.SIMPLE_PAGE_SIZE_OPTIONS,
        mode: 'simple',
        pageSize: CommonConsts.PAGE_SIZE_SMALL
      }
    };
  }

  getAssociated() {
    if (!this.updateOperation) {
      return;
    }
    this.appUtilsService.getResourceByRecursion(
      {
        fsId: this.rowData?.fileSystemInfo?.fsId,
        deviceId: this.rowData?.fileSystemInfo?.deviceId
      },
      params =>
        this.fsFileExtensionFilterService.getFsExtensionFilterUsingGET(params),
      resource => {
        this.dataTable.setSelections(resource);
        this.selectionData = resource;
      },
      true
    );
  }

  getData(filters: Filters, args) {
    const params: any = {
      pageNum: filters.paginator.pageIndex + 1,
      pageSize: filters.paginator.pageSize,
      fsId: this.rowData?.fileSystemInfo?.fsId,
      deviceId: this.rowData?.fileSystemInfo?.deviceId
    };

    if (this.updateOperation) {
      delete params.fsId;
      delete params.deviceId;
    }

    if (!isEmpty(filters.conditions)) {
      const conditions = JSON.parse(filters.conditions);
      if (
        !isEmpty(conditions.importStatus) &&
        includes(
          conditions.importStatus,
          DataMap.File_Extension_Import_Status.error.value
        )
      ) {
        assign(params, {
          importStatus: [
            ...conditions.importStatus,
            DataMap.File_Extension_Import_Status.deleteError.value
          ]
        });
        delete conditions.importStatus;
      }
      assign(params, conditions);
    }

    if (!isEmpty(filters.sort) && !isEmpty(filters.sort.key)) {
      assign(params, {
        orderType: filters.sort.direction,
        orderBy: filters.sort.key
      });
    }

    this.fsFileExtensionFilterService
      .getFsExtensionFilterUsingGET(params)
      .subscribe(res => {
        this.tableData = {
          data: res.records || [],
          total: res.totalCount || 0
        };
      });
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.infoMessageService.create({
        header: this.i18n.get('explore_update_file_extension_label'),
        content: this.i18n.get('explore_update_file_extension_tip_label'),
        width: 450,
        onOK: () => {
          this.fsFileExtensionFilterService
            .modifyFsExtensionFilterUsingPUT({
              modifyFsSuffixRequest: {
                fileSystemInfo: this.rowData?.fileSystemInfo,
                extensions: map(this.selectionData, 'fileExtensionName')
              }
            })
            .subscribe(
              () => {
                observer.next();
                observer.complete();
              },
              error => {
                observer.error(error);
                observer.complete();
              }
            );
        },
        onCancel: () => {
          observer.error(null);
          observer.complete();
        },
        lvAfterClose: result => {
          if (result && result.trigger === 'close') {
            observer.error(null);
            observer.complete();
          }
        }
      });
    });
  }
}
