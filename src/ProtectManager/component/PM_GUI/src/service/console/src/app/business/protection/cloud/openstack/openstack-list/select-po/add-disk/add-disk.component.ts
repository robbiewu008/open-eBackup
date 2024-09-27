import { Component, OnInit, TemplateRef, ViewChild } from '@angular/core';
import { ModalRef } from '@iux/live';
import { CommonConsts, DataMap, DataMapService, I18NService } from 'app/shared';
import {
  ProTableComponent,
  TableCols,
  TableConfig
} from 'app/shared/components/pro-table';
import {
  cloneDeep,
  filter,
  get,
  includes,
  isEmpty,
  reject,
  size
} from 'lodash';

@Component({
  selector: 'aui-add-disk-openstack',
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
    private dataMapService: DataMapService
  ) {}

  ngOnInit(): void {
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
        key: 'bootable',
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
        compareWith: 'id',
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
          return item.id;
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
        compareWith: 'id',
        columns: [...cols, ...rcols],
        colDisplayControl: false,
        scrollFixed: true,
        trackByFn: (_, item) => {
          return item.id;
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

  initData() {
    const allDisk = JSON.parse(get(this.data, ['extendInfo', 'volInfo']));
    setTimeout(() => {
      this.totalTableData = {
        data: cloneDeep(allDisk),
        total: size(allDisk)
      };

      const showData = !isEmpty(this.data.diskInfo)
        ? filter(allDisk, v => {
            return includes(this.data.diskInfo, v.id);
          })
        : !isEmpty(this.data.protectedObject)
        ? filter(allDisk, item => {
            return includes(
              this.data.protectedObject?.extParameters?.disk_info,
              item.id
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
