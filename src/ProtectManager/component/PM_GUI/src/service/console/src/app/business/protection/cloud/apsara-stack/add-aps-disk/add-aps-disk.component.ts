import {
  ChangeDetectorRef,
  Component,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { ModalRef } from '@iux/live';
import {
  AppService,
  CommonConsts,
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
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  cloneDeep,
  each,
  filter,
  first,
  includes,
  isEmpty,
  isNumber,
  map,
  reject,
  size
} from 'lodash';

@Component({
  selector: 'aui-add-aps-disk',
  templateUrl: './add-aps-disk.component.html',
  styleUrls: ['./add-aps-disk.component.less']
})
export class AddApsDiskComponent implements OnInit {
  data;
  leftTableConfig: TableConfig;
  rightTableConfig: TableConfig;

  totalTableData: TableData;
  selectionData: TableData;
  selectData;

  @ViewChild('operationTpl', { static: true }) operationTpl: TemplateRef<any>;
  @ViewChild('sizeTpl', { static: true }) sizeTpl: TemplateRef<any>;
  @ViewChild('totalDataTable', { static: false })
  totalDataTable: ProTableComponent;
  @ViewChild('selectDataTable', { static: false })
  selectDataTable: ProTableComponent;

  constructor(
    private modal: ModalRef,
    public i18n: I18NService,
    private appService: AppService,
    private virtualScroll: VirtualScrollService,
    private dataMapService: DataMapService,
    private cdr: ChangeDetectorRef,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit(): void {
    this.initConfig();
    this.getResourceDetail();
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
        key: 'mode',
        name: this.i18n.get('common_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Disk_Mode')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Disk_Mode')
        }
      },
      {
        key: 'kinds',
        name: this.i18n.get('protection_kind_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('aliDiskType')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('aliDiskType')
        }
      },
      {
        key: 'size',
        name: this.i18n.get('common_capacity_label'),
        cellRender: this.sizeTpl
      }
    ];

    const col = cloneDeep(cols);
    col.pop();

    const cols1: TableCols[] = [
      {
        key: 'size',
        name: this.i18n.get('common_capacity_label'),
        cellRender: this.operationTpl
      }
    ];

    this.leftTableConfig = {
      table: {
        async: false,
        compareWith: 'uuid',
        columns: cols,
        colDisplayControl: false,
        virtualScroll: true,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        scrollFixed: true,
        scroll: this.virtualScroll.scrollParam,
        selectionChange: selection => {
          this.selectData = selection;
          this.selectionData = {
            data: this.selectData,
            total: size(this.selectData)
          };
          this.disableOkBtn();
        },
        trackByFn: (index, item) => {
          return item.uuid;
        }
      },
      pagination: {
        winTablePagination: true,
        showPageSizeOptions: false,
        mode: 'simple',
        showTotal: true
      }
    };

    this.rightTableConfig = {
      table: {
        async: false,
        compareWith: 'uuid',
        columns: [...col, ...cols1],
        virtualScroll: true,
        colDisplayControl: false,
        scrollFixed: true,
        scroll: this.virtualScroll.scrollParam,
        trackByFn: (index, item) => {
          return item.uuid;
        }
      },
      pagination: {
        winTablePagination: true,
        showPageSizeOptions: false,
        mode: 'simple',
        showTotal: true
      }
    };
  }

  updateDiskData() {
    setTimeout(() => {
      const sts = cloneDeep(this.totalTableData.data);

      const showData = !isEmpty(this.data.diskInfo)
        ? filter(sts, v => {
            return includes(this.data.diskInfo, v.uuid);
          })
        : !isEmpty(this.data.protectedObject)
        ? filter(sts, item => {
            return includes(
              this.data.protectedObject?.extParameters?.disk_info,
              item.uuid
            );
          })
        : [];

      if (showData.length) {
        this.selectData = showData;
        this.totalDataTable.setSelections(showData);
        this.cdr.detectChanges();
      }
      this.selectionData = {
        data: showData,
        total: size(showData)
      };

      this.disableOkBtn();
    });
  }

  getResourceDetail() {
    this.protectedResourceApiService
      .ListResources({
        pageNo: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        queryDependency: true,
        conditions: JSON.stringify({
          uuid: this.data.rootUuid || this.data.root_uuid
        })
      })
      .subscribe((res: any) => {
        if (first(res.records)) {
          const onlineAgents = res.records[0]?.dependencies?.agents?.filter(
            item =>
              item.linkStatus ===
              DataMap.resource_LinkStatus_Special.normal.value
          );
          if (isEmpty(onlineAgents)) {
            return;
          }
          const agentsId = onlineAgents[0].uuid;
          this.getDisk(agentsId);
        }
      });
  }

  getDisk(agentsId, recordsTemp?: any[], startPage?: number) {
    const params = {
      agentId: agentsId,
      envId: this.data.rootUuid || this.data.root_uuid,
      resourceIds: [this.data.uuid || this.data.root_uuid],
      pageNo: startPage || 1,
      pageSize: 200,
      conditions: JSON.stringify({
        resourceType: 'APS-disk',
        uuid: this.data.uuid,
        regionId: this.data.extendInfo.regionId
      })
    };

    this.appService.ListResourcesDetails(params).subscribe(res => {
      if (!recordsTemp) {
        recordsTemp = [];
      }
      if (!isNumber(startPage)) {
        startPage = 1;
      }
      recordsTemp = [...recordsTemp, ...res.records];
      if (
        startPage === Math.ceil(res.totalCount / 200) ||
        res.totalCount === 0
      ) {
        each(recordsTemp, item => {
          assign(item, {
            size: item.extendInfo?.size,
            mode: item.extendInfo?.type === 'data' ? 'false' : 'true',
            kinds: item.extendInfo?.category,
            sla: false
          });
        });
        this.totalTableData = {
          data: recordsTemp,
          total: size(recordsTemp)
        };
        this.updateDiskData();
        return;
      }
      startPage++;
      this.getDisk(agentsId, recordsTemp, startPage);
    });
  }

  clearSelected() {
    this.selectionData = null;

    this.totalDataTable.setSelections([]);
    this.disableOkBtn();
  }

  removeSingle(item) {
    const newSelectData = reject(this.selectionData.data, (value: any) => {
      return item.uuid === value.uuid;
    });

    this.selectionData = {
      data: newSelectData,
      total: size(newSelectData)
    };
    this.totalDataTable.setSelections(newSelectData);
    this.disableOkBtn();
  }

  disableOkBtn() {
    this.modal.getInstance().lvOkDisabled = !size(this.selectionData.data);
  }

  onOK() {
    return cloneDeep(map(this.selectionData.data, item => item.uuid));
  }
}
