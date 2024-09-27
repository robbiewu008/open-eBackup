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
  CapacityCalculateLabel,
  CAPACITY_UNIT,
  CommonConsts,
  DataMap,
  I18NService,
  ProtectedResourceApiService
} from 'app/shared';
import {
  ProTableComponent,
  TableCols,
  TableConfig
} from 'app/shared/components/pro-table';
import {
  assign,
  cloneDeep,
  each,
  filter,
  find,
  isEmpty,
  isString,
  reject,
  size,
  take
} from 'lodash';
import { forkJoin, Observable, of } from 'rxjs';
import { mergeMap } from 'rxjs/operators';

@Component({
  selector: 'aui-add-disk-fc',
  templateUrl: './add-disk.component.html',
  styleUrls: ['./add-disk.component.less'],
  providers: [CapacityCalculateLabel]
})
export class AddDiskComponent implements OnInit {
  data;
  leftTableConfig: TableConfig;
  rightTableConfig: TableConfig;
  unitconst = CAPACITY_UNIT;

  totalTableData = {
    data: [],
    total: 0
  };
  selectionData = {
    data: [],
    total: 0
  };
  selectData;
  dataMap = DataMap;

  @ViewChild('operationTpl', { static: true }) operationTpl: TemplateRef<any>;
  @ViewChild('capacityTpl', { static: true }) capacityTpl: TemplateRef<any>;
  @ViewChild('totalDataTable', { static: false })
  totalDataTable: ProTableComponent;
  @ViewChild('selectDataTable', { static: false })
  selectDataTable: ProTableComponent;

  constructor(
    private modal: ModalRef,
    private appService: AppService,
    public i18n: I18NService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private cdr: ChangeDetectorRef
  ) {}

  ngOnInit() {
    this.initConfig();
    this.initData();
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        name: this.i18n.get('common_name_label'),
        key: 'name',
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        name: this.i18n.get('common_slot_label'),
        key: 'slot',
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        name: this.i18n.get('common_datastore_label'),
        key: 'datastore',
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        name: this.i18n.get('protection_fc_disk_capacity_label'),
        key: 'capacity',
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        cellRender: this.capacityTpl
      }
    ];

    const col = take(cols, 3);
    const cols1: TableCols[] = [
      {
        name: this.i18n.get('protection_fc_disk_capacity_label'),
        key: 'capacity',
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
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
        scroll: { y: '640px' },
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
        scroll: { y: '640px' },
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

  initData() {
    const params = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      queryDependency: true,
      conditions: JSON.stringify({
        subType:
          this.data?.subType || DataMap.Resource_Type.FusionCompute.value,
        uuid: this.data.environment.uuid
      })
    };
    this.protectedResourceApiService
      .ListResources(params)
      .subscribe((res: any) => {
        if (res.records?.length) {
          const onlineAgents = res.records[0]?.dependencies?.agents?.filter(
            item =>
              item.linkStatus ===
              DataMap.resource_LinkStatus_Special.normal.value
          );
          if (isEmpty(onlineAgents)) {
            this.totalTableData = {
              data: [],
              total: 0
            };
            this.disableOkBtn();
            return;
          }
          const agentsId = onlineAgents[0].uuid;
          this.getShowData(agentsId).subscribe(response => {
            const totalData = [];
            for (const item of response) {
              totalData.push(...item.records);
            }
            each(totalData, (item: any) => {
              assign(item, {
                slot: `${item?.extendInfo.pciType}(${item?.extendInfo.sequenceNum})`,
                datastore: item?.extendInfo?.datastoreName,
                capacity: item?.extendInfo?.quantityGB
              });
            });
            this.totalTableData = {
              data: totalData,
              total: size(totalData)
            };

            const selectData = !isEmpty(this.data.diskInfo)
              ? filter(this.totalTableData.data, item => {
                  return find(
                    this.data.diskInfo.map(curData =>
                      isString(curData) ? JSON.parse(curData) : curData
                    ),
                    { id: item.uuid }
                  );
                })
              : [];

            if (selectData.length) {
              this.selectData = selectData;
              this.totalDataTable.setSelections(selectData);
              this.cdr.detectChanges();
            }

            this.selectionData = {
              data: selectData,
              total: size(selectData)
            };
            this.disableOkBtn();
          });
        }
      });
  }

  getShowData(agentsId): Observable<any> {
    const params = {
      agentId: agentsId,
      envId: this.data.environment.uuid,
      pageNo: 1,
      pageSize: CommonConsts.PAGE_SIZE,
      resourceIds: [this.data.uuid]
    };
    let curData = [];
    return this.appService.ListResourcesDetails(params).pipe(
      mergeMap((response: any) => {
        curData = [of(response)];

        const totalCount = response.totalCount;
        const pageCount = Math.ceil(totalCount / CommonConsts.PAGE_SIZE);
        for (let i = 2; i <= pageCount; i++) {
          curData.push(
            this.appService.ListResourcesDetails({
              agentId: agentsId,
              envId: this.data.environment.uuid,
              pageNo: i,
              pageSize: CommonConsts.PAGE_SIZE,
              resourceIds: [this.data.uuid]
            })
          );
        }
        return forkJoin(curData);
      })
    );
  }

  clearSelected() {
    this.selectionData = {
      data: [],
      total: 0
    };

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
    return cloneDeep(this.selectionData.data);
  }
}
