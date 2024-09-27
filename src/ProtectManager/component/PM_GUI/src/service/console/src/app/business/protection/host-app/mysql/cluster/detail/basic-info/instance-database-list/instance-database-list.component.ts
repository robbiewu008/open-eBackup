import {
  AfterViewInit,
  ChangeDetectorRef,
  Component,
  Input,
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
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService
} from 'app/shared';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { SlaService } from 'app/shared/services/sla.service';
import {
  assign,
  each,
  first,
  includes,
  isEmpty,
  map as _map,
  size
} from 'lodash';
import { map } from 'rxjs/operators';

@Component({
  selector: 'aui-instance-database-list',
  templateUrl: './instance-database-list.component.html',
  styleUrls: ['./instance-database-list.component.less']
})
export class InstanceDatabaseListComponent implements OnInit, AfterViewInit {
  name;
  tableConfig: TableConfig;
  tableData: TableData;
  optsConfig;
  selectionData = [];
  optItems = [];
  dataMap = DataMap;

  @Input() data;
  @Input() activeIndex;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('slaComplianceExtraTpl', { static: true })
  slaComplianceExtraTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private slaService: SlaService,
    private dataMapService: DataMapService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.initConfig();
  }

  onChange() {
    this.ngOnInit();
    this.ngAfterViewInit();
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'uuid',
        name: this.i18n.get('protection_resource_id_label'),
        hidden: true,
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'auth_status',
        name: this.i18n.get('common_auth_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Verify_Status')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Verify_Status')
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
            overflow: true,
            click: data => {
              this.slaService.getDetail({
                uuid: data.sla_id,
                name: data.sla_name
              });
            }
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
        compareWith: 'uuid',
        columns: cols.filter(col => {
          if (this.activeIndex !== 'instance') {
            return !includes(['auth_status'], col.key);
          }
          return col;
        }),
        size: 'small',
        colDisplayControl: false,
        fetchData: (filter: Filters, args) => {
          this.getData(filter, args);
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  getData(filters: Filters, args) {
    if (this.activeIndex === 'instance') {
      this.getClusterInstance(filters, args);
    } else {
      this.getDataBase(filters, args);
    }
  }

  getClusterInstance(filters: Filters, args) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize
    };
    const defaultConditions = {
      subType: [DataMap.Resource_Type.MySQLClusterInstance.value],
      parentUuid: this.data?.uuid
    };

    if (!isEmpty(filters.conditions_v2)) {
      const conditionsTemp = JSON.parse(filters.conditions_v2);
      assign(defaultConditions, conditionsTemp);
    }

    assign(params, { conditions: JSON.stringify(defaultConditions) });

    if (!!size(filters.sort)) {
      assign(params, { orders: filters.orders });
    }

    this.protectedResourceApiService
      .ListResources(params)
      .pipe(
        map(res => {
          each(res.records, item => {
            assign(item, {
              auth_status: DataMap.Verify_Status.true.value
            });
            extendSlaInfo(item);
          });
          return res;
        })
      )
      .subscribe(res => {
        this.tableData = {
          total: res.totalCount,
          data: res.records
        };
        this.cdr.detectChanges();
      });
  }

  getDataBase(filters: Filters, args) {
    const params = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        subType: [DataMap.Resource_Type.MySQLClusterInstance.value],
        parentUuid: this.data?.uuid
      })
    };

    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      const instance = first(res.records);

      if (!!instance) {
        this.protectedResourceApiService
          .ShowResource({ resourceId: instance?.uuid })
          .subscribe(ins => {
            const idsArry = _map(ins['dependencies']['children'], 'uuid');
            const dataBaseParams = {
              pageNo: filters.paginator.pageIndex,
              pageSize: filters.paginator.pageSize
            };
            const defaultConditions = {
              subType: [DataMap.Resource_Type.MySQLDatabase.value],
              parentUuid: idsArry
            };

            if (!isEmpty(filters.conditions_v2)) {
              const conditionsTemp = JSON.parse(filters.conditions_v2);
              assign(defaultConditions, conditionsTemp);
            }

            assign(dataBaseParams, {
              conditions: JSON.stringify(defaultConditions)
            });

            if (!!size(filters.sort)) {
              assign(dataBaseParams, { orders: filters.orders });
            }

            this.protectedResourceApiService
              .ListResources(dataBaseParams)
              .pipe(
                map(data => {
                  each(data.records, item => {
                    assign(item, {
                      auth_status: DataMap.Verify_Status.true.value
                    });
                    extendSlaInfo(item);
                  });
                  return data;
                })
              )
              .subscribe(data => {
                this.tableData = {
                  total: data.totalCount,
                  data: data.records
                };
                this.cdr.detectChanges();
              });
          });
      }
    });
  }
}
