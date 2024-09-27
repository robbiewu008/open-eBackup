import {
  AfterViewInit,
  Component,
  Input,
  OnInit,
  ViewChild
} from '@angular/core';
import {
  CommonConsts,
  DataMapService,
  I18NService,
  IODETECTPOLICYService
} from 'app/shared';
import {
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { size } from 'lodash';

@Component({
  selector: 'aui-associated-white-list',
  templateUrl: './associated-white-list.component.html',
  styleUrls: ['./associated-white-list.component.less']
})
export class AssociatedWhiteListComponent implements OnInit, AfterViewInit {
  @Input() rowData;
  tableConfig: TableConfig;
  tableData: TableData;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private virtualScroll: VirtualScrollService,
    private ioDetectPolicyService: IODETECTPOLICYService
  ) {}

  ngAfterViewInit(): void {
    this.getData();
  }

  ngOnInit(): void {
    this.virtualScroll.getScrollParam(260);
    this.initConfig();
    window.addEventListener('resize', () => {
      this.virtualScroll.getScrollParam(260);
      this.dataTable.setTableScroll(this.virtualScroll.scrollParam);
    });
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'content',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'type',
        name: this.i18n.get('common_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('whitelistType')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('whitelistType')
        }
      },
      {
        key: 'createTime',
        name: this.i18n.get('common_create_time_label'),
        sort: true
      }
    ];

    this.tableConfig = {
      table: {
        async: false,
        compareWith: 'id',
        columns: cols,
        scrollFixed: true,
        colDisplayControl: false,
        scroll: this.virtualScroll.scrollParam,
        trackByFn: (_, item) => {
          return item.id;
        }
      },
      pagination: {
        mode: 'simple',
        pageSize: CommonConsts.PAGE_SIZE_SMALL,
        pageSizeOptions: CommonConsts.SIMPLE_PAGE_SIZE_OPTIONS,
        showPageSizeOptions: true
      }
    };
  }

  getData() {
    this.ioDetectPolicyService
      .getIoDetectPolicyById({ policyId: this.rowData?.id })
      .subscribe(res => {
        this.tableData = {
          data: <any>res.whiteListInfos,
          total: size(res.whiteListInfos)
        };
      });
  }
}
