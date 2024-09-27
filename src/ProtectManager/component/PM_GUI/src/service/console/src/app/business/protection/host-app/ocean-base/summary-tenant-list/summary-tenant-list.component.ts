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
  ProtectedEnvironmentApiService
} from 'app/shared';
import {
  ProTableComponent,
  TableCols,
  TableData,
  TableConfig
} from 'app/shared/components/pro-table';
import { SlaService } from 'app/shared/services/sla.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { size, get, map, each, assign } from 'lodash';

@Component({
  selector: 'aui-summary-tenant-list',
  templateUrl: './summary-tenant-list.component.html',
  styleUrls: ['./summary-tenant-list.component.less']
})
export class SummaryTenantListComponent implements OnInit {
  data = {} as any;
  formItems = [];
  resSubType;
  tableConfig: TableConfig;
  tableData: TableData;

  @ViewChild('slaComplianceExtraTpl', { static: true })
  slaComplianceExtraTpl: TemplateRef<any>;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private dataMapService: DataMapService,
    private slaService: SlaService,
    private virtualScroll: VirtualScrollService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.initConfig();
    this.getDetails();
  }

  initDetailData(data: any) {
    this.data = data;
    this.resSubType = data.sub_type || data.subType;
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label')
      },
      {
        key: 'linkStatus',
        name: this.i18n.get('common_status_label'),
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('resource_LinkStatus_Special')
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
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Sla_Compliance')
        }
      },
      {
        key: 'protection_status',
        name: this.i18n.get('protection_protected_status_label'),
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Protection_Status')
        }
      }
    ];

    this.tableConfig = {
      table: {
        autoPolling: CommonConsts.TIME_INTERVAL_RESOURCE,
        compareWith: 'uuid',
        columns: cols,
        virtualScroll: true,
        scrollFixed: true,
        scroll: this.virtualScroll.scrollParam,
        colDisplayControl: false
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        pageSize: CommonConsts.PAGE_SIZE_SMALL,
        winTablePagination: true
      }
    };
  }

  getDetails() {
    if (!this.data) {
      return;
    }
    const params: any = {
      envId: this.data.uuid,
      pageNo: CommonConsts.PAGE_START,
      pageSize: 200,
      resourceType: DataMap.Resource_Type.OceanBaseTenant.value
    };
    this.protectedEnvironmentApiService
      .ListEnvironmentResource(params)
      .subscribe(res => {
        const tenantsName = get(res, 'records');
        each(tenantsName, item => {
          assign(item, {
            protection_status: this.data?.protectionStatus,
            sla_id: this.data?.protectedObject?.slaId,
            sla_name: this.data?.protectedObject?.slaName,
            sla_compliance: this.data?.protectedObject?.slaCompliance
          });
        });
        this.tableData = {
          data: map(tenantsName, item => {
            return {
              name: item.name,
              linkStatus: '1',
              ...item
            };
          }),
          total: size(tenantsName)
        };
        this.cdr.detectChanges();
      });
  }
}
