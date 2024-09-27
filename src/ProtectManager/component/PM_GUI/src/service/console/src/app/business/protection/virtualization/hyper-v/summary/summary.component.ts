import { Component, OnInit, TemplateRef, ViewChild } from '@angular/core';
import {
  CAPACITY_UNIT,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService
} from 'app/shared';
import {
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { assign, defer, each, isEmpty, size } from 'lodash';

@Component({
  selector: 'aui-hyper-v-summary',
  templateUrl: './summary.component.html',
  styleUrls: ['./summary.component.less']
})
export class SummaryComponent implements OnInit {
  sourceType;
  source;
  tableConfig: TableConfig;
  tableData: TableData;
  unitconst = CAPACITY_UNIT;
  dataMap = DataMap;

  activeIndex = 'host';
  tabs = [
    {
      id: 'host',
      subType: DataMap.Resource_Type.hyperVHost.value,
      label: this.i18n.get(DataMap.Resource_Type.hyperVHost.label),
      hide: false
    },
    {
      id: 'vm',
      subType: DataMap.Resource_Type.hyperVVm.value,
      label: this.i18n.get(DataMap.Resource_Type.hyperVVm.label),
      hide: false
    }
  ];

  @ViewChild('sizeTpl', { static: true }) sizeTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService
  ) {}

  ngOnInit() {
    this.initConfig();
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
        key: 'type',
        name: this.i18n.get('common_type_label')
      },
      {
        key: 'format',
        name: this.i18n.get('common_format_label')
      },
      {
        key: 'sla',
        name: this.i18n.get('protection_associate_sla_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('slaAssociateStatus')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('slaAssociateStatus')
        }
      },
      {
        key: 'size',
        name: this.i18n.get('common_capacity_label'),
        cellRender: this.sizeTpl
      }
    ];

    this.tableConfig = {
      table: {
        async: false,
        size: 'small',
        columns: cols,
        compareWith: 'uuid',
        colDisplayControl: false,
        trackByFn: (_, item) => {
          return item.uuid;
        }
      },
      pagination: {
        mode: 'simple',
        pageSize: CommonConsts.PAGE_SIZE_SMALL,
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };

    defer(() => this.getDisk());
  }

  getDisk() {
    let selectAll = false;
    let selectDisk = [];
    if (!isEmpty(this.source?.protectedObject)) {
      const diskInfo = this.source.protectedObject?.extParameters?.disk_info;
      if (isEmpty(diskInfo)) {
        selectAll = this.source.protectedObject?.extParameters?.all_disk;
      } else {
        selectDisk = diskInfo;
      }
    }
    const disks = JSON.parse(this.source.extendInfo?.disks || '{}');
    if (disks.length) {
      each(disks, item => {
        assign(item, {
          sla: selectAll ? true : selectDisk.includes(item.uuid),
          type: item.extendInfo?.Type,
          format: item.extendInfo?.Format
        });
      });
      this.tableData = {
        data: disks,
        total: size(disks)
      };
    }
  }

  initDetailData(data: any) {
    this.sourceType = data.sub_type || data.subType;
    this.source = data;
  }
}
