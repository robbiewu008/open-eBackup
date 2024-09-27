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
import { assign, each, size } from 'lodash';
import { map } from 'rxjs/operators';

@Component({
  selector: 'aui-King-base-proxy-host',
  templateUrl: './king-base-proxy-host.component.html',
  styleUrls: ['./king-base-proxy-host.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class KingBaseProxyHostComponent implements OnInit, AfterViewInit {
  tableConfig: TableConfig;
  tableData: TableData;

  @Input() data;
  @Input() isInstance;
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
        key: 'linkStatus',
        name: this.i18n.get('common_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('resource_LinkStatus_Special')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('resource_LinkStatus_Special')
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
        resourceId: this.isInstance ? this.data.rootUuid : this.data.uuid
      })
      .pipe(
        map(res => {
          each(res['dependencies']['agents'], item => {
            assign(item, { type: res['extendInfo']['clusterType'] });
          });
          return res;
        })
      )
      .subscribe(res => {
        this.tableData = {
          data: res['dependencies']['agents'],
          total: size(res['dependencies']['agents'])
        };
        this.cdr.detectChanges();
      });
  }
}
