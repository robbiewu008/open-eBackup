import {
  AfterViewInit,
  Component,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  CAPACITY_UNIT,
  CommonConsts,
  I18NService,
  SoftwareType
} from 'app/shared';
import { BackupCopyDetectService } from 'app/shared/api/services/backup-copy-detect.service';
import {
  Filters,
  ProTableComponent,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { assign, includes, isEmpty } from 'lodash';

@Component({
  selector: 'aui-associated-snapshot',
  templateUrl: './associated-snapshot.component.html',
  styleUrls: ['./associated-snapshot.component.less']
})
export class AssociatedSnapshotComponent implements OnInit, AfterViewInit {
  rowData;
  software;
  snapshotId;
  formItems = [];

  tableConfig: TableConfig;
  tableData: TableData;
  unitconst = CAPACITY_UNIT;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('fileAttrTpl', { static: true }) fileAttrTpl: TemplateRef<any>;
  @ViewChild('sizeTpl', { static: true }) sizeTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private backupCopyDetectService: BackupCopyDetectService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.initBasicInfo();
    this.initTableConfig();
  }

  initBasicInfo() {
    this.formItems = [
      [
        {
          label: this.i18n.get('explore_associated_snapshot_time_label'),
          key: 'time',
          content: this.rowData?.backupTime
        },
        {
          label: this.i18n.get('explore_copy_size_label'),
          key: 'copySize',
          content: this.rowData?.copySize
        },
        {
          label: this.i18n.get('common_file_path_label'),
          content: this.rowData?.copyPath
        }
      ],
      [
        {
          label: this.i18n.get('explore_safe_status_label'),
          key: 'status',
          content: this.rowData?.status
        },
        {
          label: this.i18n.get('explore_associated_file_count_label'),
          content: this.rowData?.fileCount
        },
        {
          label: this.i18n.get('explore_infected_file_count_label'),
          content: this.rowData?.abnormalFileCount
        }
      ]
    ];
  }

  initTableConfig() {
    this.tableConfig = {
      table: {
        compareWith: 'name',
        columns: [
          {
            key: 'fileName',
            name: this.i18n.get('explore_file_name_label'),
            filter: {
              type: 'search',
              filterMode: 'contains'
            }
          },
          {
            key: 'fileSize',
            name: this.i18n.get('explore_capacity_size_label'),
            sort: true,
            cellRender: this.sizeTpl,
            hidden: includes([SoftwareType.CV], this.software)
          },
          {
            key: 'filePath',
            name: this.i18n.get('common_file_path_label')
          },
          {
            key: 'latestModifyTime',
            name: this.i18n.get('explore_infected_file_attr_label'),
            cellRender: this.fileAttrTpl,
            hidden: includes([SoftwareType.CV], this.software)
          }
        ],
        scrollFixed: true,
        colDisplayControl: false,
        fetchData: (filter: Filters) => {
          this.getData(filter);
        }
      },
      pagination: {
        winTablePagination: true,
        showPageSizeOptions: false,
        mode: 'simple',
        pageSize: CommonConsts.PAGE_SIZE_SMALL
      }
    };
  }

  getData(filters) {
    const params: any = {
      pageNum: filters.paginator.pageIndex + 1,
      pageSize: filters.paginator.pageSize,
      software: this.rowData?.software,
      backupCopyId: this.rowData?.id,
      snapshotId: this.snapshotId
    };

    if (!isEmpty(filters.conditions)) {
      const conditions = JSON.parse(filters.conditions);
      assign(params, conditions);
    }

    if (!isEmpty(filters.sort) && !isEmpty(filters.sort.key)) {
      assign(params, {
        orderType: filters.sort.direction,
        orderBy: filters.sort.key
      });
    }

    this.backupCopyDetectService.GetAbnormalFileById(params).subscribe(res => {
      this.tableData = {
        data: res.records,
        total: res.totalCount
      };
    });
  }
}
