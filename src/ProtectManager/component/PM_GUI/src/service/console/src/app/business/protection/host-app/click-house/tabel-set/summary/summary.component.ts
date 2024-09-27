import { Component, OnInit, ViewChild } from '@angular/core';
import {
  DataMap,
  GlobalService,
  I18NService,
  ProtectedResourceApiService
} from 'app/shared';
import { TableCols, ProTableComponent } from 'app/shared/components/pro-table';
import { assign, get, size, toNumber } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-summary-table-set',
  templateUrl: './summary.component.html',
  styleUrls: ['./summary.component.css']
})
export class SummaryComponent implements OnInit {
  source;
  dbInfo;
  type;
  dataMap = DataMap;
  tableConfig;
  tableData;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private i18n: I18NService,
    public globalService: GlobalService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.initTable().getData();
  }

  initDetailData(data) {
    this.source = assign(data, {
      link_status: toNumber(data.linkStatus)
    });
    this.type = DataMap.Resource_Type.ClickHouseTableset.value;
    this.getDetail(data.environment_uuid || data?.environment?.uuid).subscribe(
      item => {
        this.source.link_status = (item as any).linkStatus;
        this.globalService.emitStore({
          action: 'autoReshResource',
          state: this.source
        });
      }
    );
  }

  initTable() {
    const cols: TableCols[] = [
      {
        key: 'uuid',
        name: this.i18n.get('protection_resource_id_label'),
        hidden: true
      },
      {
        key: 'name',
        name: this.i18n.get('common_table_label'),
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
    return this;
  }

  getData() {
    const params = {
      resourceId: this.source?.uuid
    };

    this.protectedResourceApiService.ShowResource(params).subscribe(res => {
      const nodes = get(res, ['dependencies', 'children']);
      this.tableData = {
        data: nodes,
        total: size(nodes)
      };
    });
  }

  getDetail(uuid) {
    return new Observable<object>((observer: Observer<object>) => {
      this.protectedResourceApiService
        .ShowResource({
          resourceId: uuid
        })
        .subscribe(
          item => {
            observer.next(item);
            observer.complete();
          },
          error => {
            observer.error(error);
            observer.complete();
          }
        );
    });
  }
}
