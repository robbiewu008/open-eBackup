import { AfterViewInit, Component, OnInit, ViewChild } from '@angular/core';
import { FormGroup } from '@angular/forms';
import {
  AntiRansomwareAirgapApiService,
  ApiService,
  DataMap,
  DataMapService,
  I18NService
} from 'app/shared';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { assign, isEmpty, isUndefined, map } from 'lodash';

@Component({
  selector: 'aui-airgap-detail',
  templateUrl: './airgap-detail.component.html',
  styleUrls: ['./airgap-detail.component.less']
})
export class AirgapDetailComponent implements OnInit, AfterViewInit {
  isName;
  tableConfig: TableConfig;
  tableData: TableData;
  activeIndex = 'summary';
  formGroup: FormGroup;
  data;
  dataMap = DataMap;
  airGapPorts = [];

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private i18n: I18NService,
    private apiService: ApiService,
    private dataMapService: DataMapService,
    private antiRansomwareAirgapApiService: AntiRansomwareAirgapApiService
  ) {}

  ngAfterViewInit() {
    if (this.isName) {
      return;
    } else {
      this.dataTable.fetchData();
    }
  }

  change() {
    if (this.activeIndex === 'device') {
      this.dataTable.fetchData();
    }
  }

  ngOnInit(): void {
    if (!this.isName) {
      this.activeIndex = 'device';
    }
    this.initConfig();
    this.airGapPorts = map(this.data.airGapPorts, item => {
      return {
        name: `${item.name}(${item.ip})`,
        isActive: item.isActive
      };
    });
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label')
      },
      {
        key: 'linkStatus',
        name: this.i18n.get('common_device_status_label'),
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
      {
        key: 'esn',
        name: this.i18n.get('ESN')
      }
    ];

    this.tableConfig = {
      table: {
        columns: cols,
        colDisplayControl: false,
        fetchData: (filter: Filters, args) => {
          this.getData(filter, args);
        }
      },
      pagination: {
        winTablePagination: true,
        mode: 'simple',
        showTotal: true
      }
    };
  }

  getData(filters: Filters, args: { isAutoPolling: any }) {
    const params = {
      policyId: this.data.id,
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };
    if (!isEmpty(filters.conditions)) {
      const conditionsTemp = JSON.parse(filters.conditions);
      if (conditionsTemp?.linkStatus) {
        assign(params, {
          linkStatus: conditionsTemp.linkStatus
        });
      }
    }
    this.antiRansomwareAirgapApiService
      .ShowPolicyRelatePage(params)
      .subscribe((res: any) => {
        this.tableData = {
          data: res.records,
          total: res.totalCount
        };
      });
  }
}
