import {
  ChangeDetectorRef,
  Component,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { ModalRef } from '@iux/live';
import { DataMap, DataMapService, I18NService } from 'app/shared';
import {
  ProTableComponent,
  TableCols,
  TableConfig
} from 'app/shared/components/pro-table';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  cloneDeep,
  each,
  every,
  filter,
  get,
  includes,
  isEmpty,
  reject,
  size
} from 'lodash';

@Component({
  selector: 'aui-add-disk-hcs',
  templateUrl: './add-disk.component.html',
  styleUrls: ['./add-disk.component.less']
})
export class AddDiskComponent implements OnInit {
  data;
  leftTableConfig: TableConfig;
  rightTableConfig: TableConfig;

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
  @ViewChild('sizeTpl', { static: true }) sizeTpl: TemplateRef<any>;
  @ViewChild('totalDataTable', { static: false })
  totalDataTable: ProTableComponent;
  @ViewChild('selectDataTable', { static: false })
  selectDataTable: ProTableComponent;
  constructor(
    private modal: ModalRef,
    public i18n: I18NService,
    private virtualScroll: VirtualScrollService,
    private dataMapService: DataMapService,
    private cdr: ChangeDetectorRef
  ) {}

  ngOnInit() {
    this.initConfig();
    this.initData();
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
        key: 'attr',
        name: this.i18n.get('protection_incremental_mode_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Disk_Status')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Disk_Status')
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
        compareWith: 'id',
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
          return item.id;
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
        compareWith: 'id',
        columns: [...col, ...cols1],
        virtualScroll: true,
        colDisplayControl: false,
        scrollFixed: true,
        scroll: this.virtualScroll.scrollParam,
        trackByFn: (index, item) => {
          return item.id;
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
    if (!this.data?.extendInfo?.host) {
      return;
    }
    const sts = JSON.parse(get(this.data, ['extendInfo', 'host'])).diskInfo;
    const enableType = ['FusionStorage', 'OceanStor', 'Dorado'];
    each(sts, disk => {
      assign(disk, {
        disabled: every(enableType, v => disk.storageType.indexOf(v) === -1)
      });
    });
    setTimeout(() => {
      this.totalTableData = {
        data: cloneDeep(sts),
        total: size(sts)
      };

      const showData = !isEmpty(this.data.diskInfo)
        ? filter(sts, v => {
            return includes(this.data.diskInfo, v.id);
          })
        : !isEmpty(this.data.protectedObject)
        ? filter(sts, item => {
            return includes(
              this.data.protectedObject?.extParameters?.disk_info,
              item.id
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
      return item.id === value.id;
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
    return cloneDeep(this.selectionData.data.map(item => item.id));
  }
}
