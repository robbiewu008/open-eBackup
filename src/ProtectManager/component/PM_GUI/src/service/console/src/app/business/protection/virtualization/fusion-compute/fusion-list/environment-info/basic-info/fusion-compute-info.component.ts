import {
  Component,
  OnInit,
  Input,
  AfterViewInit,
  ChangeDetectorRef,
  ViewChild
} from '@angular/core';
import {
  CommonConsts,
  DataMap,
  DataMapService,
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
import { size } from 'lodash';

@Component({
  selector: 'aui-fusion-compute-basic-info',
  templateUrl: './fusion-compute-basic-info.component.html',
  styleUrls: ['./fusion-compute-basic-info.component.less']
})
export class FusionComputeBasicInfoComponent implements OnInit, AfterViewInit {
  @Input() data;
  online = true;
  formItems;
  tableConfig: TableConfig;
  tableData: TableData;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.initConfig();
  }

  ngAfterViewInit() {
    this.formItems = [
      [
        {
          label: this.i18n.get('common_name_label'),
          content: this.data.name
        },
        {
          label: this.i18n.get('common_ip_label'),
          content: this.data.endpoint
        },
        {
          label: this.i18n.get('common_username_label'),
          content: this.data.auth.authKey
        }
      ],
      [
        {
          label: this.i18n.get('common_port_label'),
          content: this.data.port
        }
      ]
    ];

    this.getHosts();
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label')
      },
      {
        key: 'endpoint',
        name: this.i18n.get('common_ip_address_label')
      },
      {
        key: 'linkStatus',
        name: this.i18n.get('common_status_label'),
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('resource_LinkStatus_Special')
        }
      }
    ];

    this.tableConfig = {
      table: {
        async: false,
        showLoading: false,
        columns: cols,
        size: 'small',
        colDisplayControl: false,
        autoPolling: CommonConsts.TIME_INTERVAL_RESOURCE,
        fetchData: (filter: Filters) => {
          this.getHosts();
        }
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
        akLoading: false
      })
      .subscribe((res: any) => {
        this.tableData = {
          data: res['dependencies']['agents'],
          total: size(res['dependencies']['agents'])
        };
        this.online =
          res.linkStatus === DataMap.resource_LinkStatus_Special.normal.value;
        this.cdr.detectChanges();
      });
  }
}
