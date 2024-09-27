import {
  AfterViewInit,
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  OnInit,
  ViewChild,
  Input
} from '@angular/core';
import { DataMap, I18NService, ProtectedResourceApiService } from 'app/shared';
import {
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { assign, each, size } from 'lodash';
import { map } from 'rxjs/operators';

@Component({
  selector: 'aui-postgre-proxy-host',
  templateUrl: './postgre-proxy-host.component.html',
  styleUrls: ['./postgre-proxy-host.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class PostgreProxyHostComponent implements OnInit, AfterViewInit {
  tableConfig: TableConfig;
  tableData: TableData;

  @Input() data;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngAfterViewInit(): void {
    this.getHosts();
  }

  ngOnInit(): void {
    this.initConfig();
  }

  initConfig() {
    const installDeployType = this.data.extendInfo?.installDeployType;
    // 没有相应字段时默认为pgpool
    const isPgpool =
      !installDeployType ||
      installDeployType === DataMap.PostgreSqlDeployType.Pgpool.value;
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
        key: 'endpoint',
        name: this.i18n.get('common_ip_address_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'clientPath',
        name: this.i18n.get('common_database_client_path_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'pgpoolClientPath',
        name: this.i18n.get(
          isPgpool ? 'common_pg_pool_path_label' : 'common_patroni_path_label'
        ),
        hidden:
          this.data.extendInfo?.installDeployType ===
          DataMap.PostgreSqlDeployType.CLup.value,
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'serviceIp',
        name: this.i18n.get('common_dataplane_ip_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'port',
        name: this.i18n.get('common_database_port_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
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
          if (
            this.data.sub_type ===
            DataMap.Resource_Type.PostgreSQLClusterInstance.value
          ) {
            each(res['dependencies']['children'], item => {
              assign(item, {
                serviceIp: item['extendInfo']['serviceIp'],
                port: item['extendInfo']['instancePort'],
                clientPath: item['extendInfo']['clientPath'],
                pgpoolClientPath: item['extendInfo']['pgpoolClientPath'],
                endpoint: item['dependencies']['agents'][0]['endpoint'],
                name: item['dependencies']['agents'][0]['name']
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
}
