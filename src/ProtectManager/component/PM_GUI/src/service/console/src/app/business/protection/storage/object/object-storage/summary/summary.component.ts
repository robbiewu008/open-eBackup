import {
  ChangeDetectorRef,
  Component,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  CommonConsts,
  DataMap,
  DataMapService,
  extendSlaInfo,
  I18NService,
  ProtectedResourceApiService
} from 'app/shared';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { assign, each, find, isEmpty, isUndefined } from 'lodash';
import { map } from 'rxjs/operators';

@Component({
  selector: 'aui-summary-object-storage',
  templateUrl: './summary.component.html',
  styleUrls: ['./summary.component.less']
})
export class SummaryComponent implements OnInit {
  activeIndex: string = 'baseInfo';
  tableConfig: TableConfig;
  tableData: TableData;
  dataMap = DataMap;
  leftColumns = [];
  rightColumns = [];
  data;
  agents;

  typeOptions = this.dataMapService.toArray('objectStorageType');
  protocolOptions = this.dataMapService.toArray('protocolType');

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('slaComplianceExtraTpl', { static: true })
  slaComplianceExtraTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private dataMapService: DataMapService,
    private appUtilsService: AppUtilsService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngAfterViewInit(): void {
    this.dataTable.fetchData();
  }

  ngOnInit(): void {
    this.initConfig();
    this.getAgentData();
    this.initData();
  }

  initConfig() {
    this.leftColumns = [
      {
        key: 'name',
        label: this.i18n.get('common_name_label')
      },
      {
        key: 'type',
        label: this.i18n.get('common_type_label')
      },
      {
        key: 'protocol',
        label: this.i18n.get('common_protocol_label')
      },
      {
        key: 'AK',
        label: 'AK'
      }
    ];

    this.rightColumns = [
      {
        key: 'linkStatus',
        label: this.i18n.get('common_status_label')
      },
      {
        key: 'endpoint',
        label: 'Endpoint'
      },
      {
        key: 'cert',
        label: this.i18n.get('protection_fc_cert_label')
      },
      {
        key: 'SK',
        label: 'SK'
      }
    ];

    if (
      Number(this.data.extendInfo.storageType) !==
      DataMap.objectStorageType.pacific.value
    ) {
      this.rightColumns.splice(2, 1);
    }

    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'sla_name',
        name: this.i18n.get('common_sla_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        cellRender: {
          type: 'text',
          config: {
            id: 'outerClosable',
            iconPos: 'flow-text',
            overflow: true
          }
        }
      },
      {
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
      {
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
    ];

    this.tableConfig = {
      table: {
        columns: cols,
        async: true,
        colDisplayControl: false,
        fetchData: (filter: Filters, args: {}) => {
          this.getData(filter, args);
        }
      },
      pagination: {
        pageSize: CommonConsts.PAGE_SIZE_SMALL,
        winTablePagination: true,
        mode: 'simple',
        showPageSizeOptions: false,
        pageSizeOptions: CommonConsts.SIMPLE_PAGE_SIZE_OPTIONS
      }
    };
  }

  initData() {
    each(this.leftColumns, item => {
      switch (item.key) {
        case 'agent':
          item.value = this.agents;
          break;
        case 'type':
          item.value = find(this.typeOptions, temp => {
            return temp.value === Number(this.data.extendInfo.storageType);
          }).label;
          break;
        case 'protocol':
          item.value = find(this.protocolOptions, temp => {
            return temp.value === this.data.extendInfo.useHttps;
          }).label;
          break;
        case 'AK':
          item.value = this.data.extendInfo.ak;
          break;
        default:
          item.value = this.data[item.key];
      }
    });
    each(this.rightColumns, item => {
      switch (item.key) {
        case 'SK':
          item.value = '*********';
          break;
        case 'cert':
          item.value = this.data.extendInfo?.certName;
          break;
        default:
          item.value = this.data[item.key];
      }
    });
  }

  getAgentData() {
    this.protectedResourceApiService
      .ShowResource({
        resourceId: this.data.uuid
      })
      .subscribe(res => {
        const hostArray = [];
        if (res['dependencies']?.agents) {
          each(res['dependencies']?.agents, item => {
            hostArray.push(`${item.name}(${item.endpoint})`);
          });
          this.agents = hostArray.join(', ');
        }
      });
  }

  getData(filters: Filters, args) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };

    const defaultConditions = {
      parentUuid: this.data.uuid,
      subType: DataMap.Resource_Type.ObjectSet.value
    };

    if (!isEmpty(filters.conditions_v2)) {
      const conditionsTemp = JSON.parse(filters.conditions_v2);

      assign(defaultConditions, conditionsTemp);
    }

    assign(params, { conditions: JSON.stringify(defaultConditions) });

    this.protectedResourceApiService
      .ListResources(params)
      .pipe(
        map(res => {
          each(res.records, item => {
            extendSlaInfo(item);
          });
          return res;
        })
      )
      .subscribe(res => {
        this.tableData = {
          data: res.records,
          total: res.totalCount
        };
        this.cdr.detectChanges();
      });
  }
}
