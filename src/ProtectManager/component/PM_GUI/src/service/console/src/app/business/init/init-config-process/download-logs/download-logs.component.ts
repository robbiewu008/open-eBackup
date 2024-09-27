import {
  AfterViewInit,
  Component,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { ModalRef } from '@iux/live';
import {
  DataMapService,
  DataMap,
  I18NService,
  ApiExportFilesApiService as ExportFileApiService,
  CommonConsts
} from 'app/shared';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { find, isUndefined } from 'lodash';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'aui-download-logs',
  templateUrl: './download-logs.component.html',
  styleUrls: ['./download-logs.component.less']
})
export class DownloadLogsComponent implements OnInit, AfterViewInit {
  name;
  tableConfig: TableConfig;
  tableData: TableData;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('errorCodeTpl', { static: true })
  errorCodeTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private exportFilesApi: ExportFileApiService,
    private dataMapService: DataMapService,
    private modal: ModalRef,
    private appUtilsService: AppUtilsService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.initTable();
  }
  initTable() {
    const cols: TableCols[] = [
      {
        key: 'fileName',
        name: this.i18n.get('common_name_label')
      },
      {
        key: 'status',
        name: this.i18n.get('common_status_label'),
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('exportLogStatus')
        }
      },
      {
        key: 'remark',
        name: this.i18n.get('common_remarks_label'),
        cellRender: this.errorCodeTpl
      },
      {
        key: 'operation',
        width: 110,
        name: this.i18n.get('common_operation_label'),
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 2,
            items: [
              {
                id: 'download',
                type: 'primary',
                disableCheck: ([data]) => {
                  return data.status !== DataMap.exportLogStatus.success.value;
                },
                label: this.i18n.get('common_download_label'),
                onClick: data => {
                  this.downLodLog(data[0]);
                }
              }
            ]
          }
        }
      }
    ];

    this.tableConfig = {
      pagination: null,
      table: {
        autoPolling: CommonConsts.TIME_INTERVAL_TWO,
        compareWith: 'name',
        columns: cols,
        colDisplayControl: false,
        rows: null,

        size: 'small',
        fetchData: (filter, args) => {
          this.getData(filter, args);
        },
        trackByFn: (index, item) => {
          return item.name;
        }
      }
    };
  }

  downLodLog(item) {
    this.exportFilesApi
      .DownloadExportFile({ id: item.id })
      .subscribe((res: any) => {
        const bf = new Blob([res], {
          type: 'application/zip'
        });
        this.appUtilsService.downloadFile(`${item.fileName}.zip`, bf);
      });
  }
  getData(filters: Filters, args) {
    this.exportFilesApi
      .GetExportFiles({
        fileName: this.name,
        akLoading:
          !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
      })
      .subscribe(res => {
        this.tableData = {
          data: res?.items as any,
          total: 1
        };
        if (
          !find(res.items, (item: any) => {
            return item.status === DataMap.exportLogStatus.generating.value;
          })
        ) {
          this.modal.setProperty({
            lvCloseButtonDisplay: true,
            lvFooter: [
              {
                id: 'close',
                label: this.i18n.get('common_close_label'),
                onClick: (modal, button) => modal.close()
              }
            ]
          });
        }
      });
  }
}
