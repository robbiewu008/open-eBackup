import {
  Component,
  OnInit,
  ViewChild,
  AfterViewInit,
  ChangeDetectionStrategy,
  Input
} from '@angular/core';
import {
  ProTableComponent,
  TableConfig,
  TableData,
  TableCols
} from 'app/shared/components/pro-table';
import { I18NService } from 'app/shared';

@Component({
  selector: 'aui-instance-list',
  templateUrl: './instance-list.component.html',
  styleUrls: ['./instance-list.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class InstanceListComponent implements OnInit, AfterViewInit {
  tableConfig: TableConfig;
  tableData: TableData;

  @Input() source;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(private i18n: I18NService) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.initConfig();
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_host_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'ip',
        name: this.i18n.get('protection_host_cluster_ip_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      }
    ];

    this.tableConfig = {
      table: {
        columns: cols,
        showLoading: false,
        colDisplayControl: false,
        size: 'small',
        async: false
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }
}
