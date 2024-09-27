import {
  AfterViewInit,
  ChangeDetectorRef,
  Component,
  EventEmitter,
  Input,
  OnInit,
  Output,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  DataMap,
  DataMapService,
  I18NService,
  JobAPIService,
  MODAL_COMMON,
  CommonConsts
} from 'app/shared';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  isString,
  isEmpty,
  size,
  isUndefined,
  first,
  get
} from 'lodash';

@Component({
  selector: 'aui-job-event',
  templateUrl: './job-event.component.html',
  styleUrls: ['./job-event.component.less']
})
export class JobEventComponent implements OnInit, AfterViewInit {
  logDetailInfo;
  @Input() job;
  @Input() timeZone;
  _isString = isString;
  tableConfig: TableConfig;
  tableData: TableData;
  jobLogLevel = DataMap.Job_Log_Level;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('logInfoTpl', { static: true }) logInfoTpl: TemplateRef<any>;
  @ViewChild('statusTpl', { static: true }) statusTpl: TemplateRef<any>;
  @ViewChild('logDetailInfoTpl', { static: true })
  logDetailInfoTpl: TemplateRef<any>;
  @ViewChild('startTimeTpl', { static: true }) startTimeTpl: TemplateRef<any>;

  @Output() showManualFeedback = new EventEmitter<any>();
  constructor(
    public i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private drawModalService: DrawModalService,
    private dataMapService: DataMapService,
    private jobApiService: JobAPIService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.initConfig();
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'level',
        name: this.i18n.get('common_status_label'),
        width: '105px',
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Job_Log_Level')
        },
        cellRender: this.statusTpl
      },
      {
        key: 'startTime',
        width: '200px',
        name: this.i18n.get('common_time_label'),
        sort: true,
        cellRender: this.startTimeTpl
      },
      {
        key: 'logInfo',
        name: this.i18n.get('common_desc_label'),
        cellRender: this.logInfoTpl
      }
    ];
    this.tableConfig = {
      table: {
        autoPolling: 5 * 1e3,
        compareWith: 'jobId',
        size: 'small',
        columns: cols,
        showLoading: false,
        colDisplayControl: false,
        fetchData: (filter: Filters, args) => {
          this.getData(filter, args);
        },
        trackByFn: (index, item) => {
          return item.jobId;
        }
      },
      pagination: {
        mode: 'simple',
        winTablePagination: true
      }
    };
  }

  viewLogInfo(item) {
    this.logDetailInfo = item.logDetailInfo;
    this.drawModalService.create(
      assign({}, MODAL_COMMON.drawerOptions, {
        lvModalKey: 'job-log-detail-info',
        lvWidth: MODAL_COMMON.normalWidth + 100,
        lvHeader: this.i18n.get('common_error_detail_label'),
        lvContent: this.logDetailInfoTpl,
        lvFooter: [
          {
            label: this.i18n.get('common_close_label'),
            onClick: modal => modal.close()
          }
        ]
      })
    );
  }

  getData(filter, args) {
    this.jobApiService
      .queryJobLogsUsingGET({
        ...this.getParams(filter, args)
      })
      .subscribe(res => {
        this.tableData = {
          data: res.records,
          total: res.totalCount
        };

        this.showManualFeedback.emit(first(res.records));
        this.cdr.detectChanges();
      });
  }

  getParams(filter, args) {
    // getMultiClusterJobList接口返回的job会携带role和clusterId
    // 在'所有集群'时需要传递这两个参数才能查看jobLog
    const params = {
      jobId: this.job.jobId,
      orderBy: 'startTime',
      orderType: 'desc',
      startPage: filter.paginator.pageIndex + 1,
      pageSize: filter.paginator.pageSize,
      clustersType: this.job?.role || null, // 为null时默认使用cookie里的字段
      clustersId: this.job?.clusterId || null,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };

    if (!isEmpty(filter.conditions)) {
      const conditions = JSON.parse(filter.conditions);
      assign(params, {
        levels: conditions.level
      });
    }

    if (!!size(filter.orders)) {
      assign(params, {
        orderType: filter.sort.direction,
        orderBy: filter.sort.key
      });
    }

    return params;
  }
}
