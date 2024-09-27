import { Component, OnInit, TemplateRef, ViewChild } from '@angular/core';
import { ModalRef } from '@iux/live';
import {
  CAPACITY_UNIT,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService
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
  get,
  includes,
  isEmpty,
  reject,
  size
} from 'lodash';

@Component({
  selector: 'aui-select-disk',
  templateUrl: './select-disk.component.html',
  styleUrls: ['./select-disk.component.less']
})
export class SelectDiskComponent implements OnInit {
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
  unitconst = CAPACITY_UNIT;
  @ViewChild('operationTpl', { static: true }) operationTpl: TemplateRef<any>;
  @ViewChild('sizeTpl', { static: true }) sizeTpl: TemplateRef<any>;
  @ViewChild('totalDataTable', { static: false })
  totalDataTable: ProTableComponent;
  @ViewChild('selectDataTable', { static: false })
  selectDataTable: ProTableComponent;

  constructor(
    private modal: ModalRef,
    private i18n: I18NService,
    private dataMapService: DataMapService
  ) {}

  ngOnInit() {
    this.initConfig();
    this.initData();
  }

  compareWithKey(): string {
    if (includes([DataMap.Resource_Type.hyperVVm.value], this.data.subType)) {
      return 'uuid';
    }
    return 'id';
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
        key: 'bootable',
        name: this.i18n.get('common_type_label'),
        hidden: includes(
          [DataMap.Resource_Type.hyperVVm.value],
          this.data.subType
        ),
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
        key: 'type',
        name: this.i18n.get('common_type_label'),
        hidden: !includes(
          [DataMap.Resource_Type.hyperVVm.value],
          this.data.subType
        )
      },
      {
        key: 'format',
        name: this.i18n.get('common_format_label'),
        hidden: !includes(
          [DataMap.Resource_Type.hyperVVm.value],
          this.data.subType
        )
      }
    ];

    const lcols: TableCols[] = [
      {
        key: 'size',
        name: this.i18n.get('common_capacity_label'),
        cellRender: this.sizeTpl
      }
    ];

    const rcols: TableCols[] = [
      {
        key: 'size',
        name: this.i18n.get('common_capacity_label'),
        cellRender: this.operationTpl
      }
    ];

    this.leftTableConfig = {
      table: {
        async: false,
        compareWith: this.compareWithKey(),
        columns: [...cols, ...lcols],
        colDisplayControl: false,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        scrollFixed: true,
        selectionChange: selection => {
          this.selectData = selection;
          this.selectionData = {
            data: this.selectData,
            total: size(this.selectData)
          };
          this.disableOkBtn();
        },
        trackByFn: (_, item) => {
          return item[this.compareWithKey()];
        }
      },
      pagination: {
        pageSize: CommonConsts.PAGE_SIZE_SMALL,
        winTablePagination: true,
        showPageSizeOptions: false,
        mode: 'simple',
        showTotal: true
      }
    };

    this.rightTableConfig = {
      table: {
        async: false,
        compareWith: this.compareWithKey(),
        columns: [...cols, ...rcols],
        colDisplayControl: false,
        scrollFixed: true,
        trackByFn: (_, item) => {
          return item[this.compareWithKey()];
        }
      },
      pagination: {
        pageSize: CommonConsts.PAGE_SIZE_SMALL,
        winTablePagination: true,
        showPageSizeOptions: false,
        mode: 'simple',
        showTotal: true
      }
    };
  }

  getAllDisk() {
    if (includes([DataMap.Resource_Type.hyperVVm.value], this.data.subType)) {
      return JSON.parse(get(this.data, ['extendInfo', 'disks']) || '[]');
    }
    return JSON.parse(get(this.data, ['extendInfo', 'disk']) || '[]');
  }

  setExtParams(item) {
    if (includes([DataMap.Resource_Type.hyperVVm.value], this.data.subType)) {
      // hyperv虚拟机和VHDSet类型的磁盘不允许选择保护
      assign(item, {
        type: item.extendInfo?.Type,
        format: item.extendInfo?.Format,
        provisionedSize: item.extendInfo?.Capacity,
        disabled:
          item.extendInfo?.Format === 'VHDSet' ||
          item.extendInfo?.IsShared === 'true'
      });
    }
  }

  initData() {
    const allDisk = this.getAllDisk();
    each(allDisk, item => this.setExtParams(item));
    setTimeout(() => {
      this.totalTableData = {
        data: cloneDeep(allDisk),
        total: size(allDisk)
      };

      const showData = !isEmpty(this.data.diskInfo)
        ? filter(allDisk, v => {
            return includes(this.data.diskInfo, v[this.compareWithKey()]);
          })
        : !isEmpty(this.data.protectedObject)
        ? filter(allDisk, item => {
            return includes(
              this.data.protectedObject?.extParameters?.disk_info,
              item[this.compareWithKey()]
            );
          })
        : [];

      if (showData.length) {
        this.selectData = showData;
        this.totalDataTable.setSelections(showData);
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
      return item[this.compareWithKey()] === value[this.compareWithKey()];
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
    return cloneDeep(
      this.selectionData.data.map(item => item[this.compareWithKey()])
    );
  }
}
