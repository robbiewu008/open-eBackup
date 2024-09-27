import {
  AfterViewInit,
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  OnInit,
  ViewChild,
  Input
} from '@angular/core';
import { I18NService, ProtectedResourceApiService } from 'app/shared';
import {
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { size } from 'lodash';

@Component({
  selector: 'aui-hcs-store-resource-list',
  templateUrl: './hcs-store-resource.component.html',
  styleUrls: ['./hcs-store-resource.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class HCSStoreResourceComponent implements OnInit, AfterViewInit {
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
    const cols: TableCols[] = [
      {
        key: 'ip',
        name: this.i18n.get('common_management_ip_label'),
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
        resourceId: this.data.uuid
      })
      .subscribe(res => {
        const showData = JSON.parse(res.extendInfo?.storages || '{}');
        this.tableData = {
          data: showData,
          total: size(showData)
        };
        this.cdr.detectChanges();
      });
  }
}
