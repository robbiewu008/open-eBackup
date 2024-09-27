import {
  AfterViewInit,
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  OnInit,
  ViewChild,
  Input
} from '@angular/core';
import {
  DataMap,
  DataMapService,
  I18NService,
  ProtectedResourceApiService
} from 'app/shared';
import {
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { assign, cloneDeep, each, get, size } from 'lodash';
import { map } from 'rxjs/operators';

@Component({
  selector: 'aui-king-base-proxy-host',
  templateUrl: './king-base-proxy-host.component.html',
  styleUrls: ['./king-base-proxy-host.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class KingBaseProxyHostComponent implements OnInit, AfterViewInit {
  tableConfig: TableConfig;
  tableData: TableData;
  dataDetails;

  @Input() data;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngAfterViewInit(): void {
    this.getHosts();
  }

  ngOnInit(): void {
    this.initConfig();
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('protection_host_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'endpoint',
        name: this.i18n.get('common_ip_address_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'clientPath',
        name: this.i18n.get('common_client_path_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'serviceIp',
        name: this.i18n.get('common_business_ip_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'port',
        name: this.i18n.get('common_port_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'role',
        hidden:
          this.data.sub_type === DataMap.Resource_Type.KingBaseInstance.value,
        name: this.i18n.get('protection_running_mode_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Redis_Node_Type')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Redis_Node_Type')
        }
      }
    ];

    this.tableConfig = {
      table: {
        async: false,
        columns: cols,
        size: 'small',
        colDisplayControl: false
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  getHosts() {
    this.protectedResourceApiService
      .ShowResource({
        resourceId: this.data.uuid,
        akDoException: false
      })
      .pipe(
        map(res => {
          this.dataDetails = cloneDeep(res);
          if (
            this.data.sub_type ===
            DataMap.Resource_Type.KingBaseClusterInstance.value
          ) {
            each(res['dependencies']['children'], item => {
              assign(item, {
                serviceIp: item['extendInfo']['serviceIp'],
                port: item['extendInfo']['instancePort'],
                clientPath: item['extendInfo']['clientPath'],
                endpoint: item['dependencies']['agents'][0]['endpoint'],
                name: item['dependencies']['agents'][0]['name'],
                role: get(item, 'extendInfo.role')
              });
            });
          } else {
            assign(res['extendInfo'], {
              endpoint: res['environment']['endpoint'],
              name: res['environment']['name'],
              port: res['extendInfo']['instancePort']
            });
          }
          return res;
        })
      )
      .subscribe(res => {
        this.tableData = {
          data: res['dependencies']['children'] || [res['extendInfo']],
          total:
            size(res['dependencies']['children']) || size([res['extendInfo']])
        };
        this.cdr.detectChanges();
      });
  }

  refresh() {
    this.protectedResourceApiService
      .UpdateResource({
        resourceId: this.data.uuid,
        UpdateResourceRequestBody: this.dataDetails
      })
      .subscribe(() => {
        this.getHosts();
      });
  }
}
