import {
  Component,
  OnInit,
  ViewChild,
  ChangeDetectorRef,
  ChangeDetectionStrategy,
  TemplateRef
} from '@angular/core';
import {
  CommonConsts,
  I18NService,
  DataMapService,
  BaseUtilService,
  TapeLibraryApiService,
  DataMap,
  CAPACITY_UNIT
} from 'app/shared';
import {
  Filters,
  TableCols,
  ProTableComponent,
  TableData,
  TableConfig
} from 'app/shared/components/pro-table';
import { first, assign, isEmpty, isUndefined } from 'lodash';

@Component({
  selector: 'aui-storage-pool-detail',
  templateUrl: './storage-pool-detail.component.html',
  styleUrls: ['./storage-pool-detail.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class StoragePoolDetailComponent implements OnInit {
  active;
  mediaSet;
  libraries = [];
  totalTape = 0;
  totalAvailableTape = 0;
  unitconst = CAPACITY_UNIT;
  tapeTableData: TableData;
  tapeTableConfig: TableConfig;
  dataMap = DataMap;
  retentionType = '';
  retentionUnit = '';
  node;
  nameLabel = this.i18n.get('common_name_label');
  typeLabel = this.i18n.get('system_tape_type_label');
  tapeLabel = this.i18n.get('system_archive_tape_label');
  semicolonLabel = this.i18n.language === 'zh-cn' ? '；' : '; ';
  basicInfoLabel = this.i18n.get('common_basic_info_label');
  rentainLabel = this.i18n.get('common_retention_policy_label');
  controllerLabel = this.i18n.get('common_home_node_label');
  blockSizeLabel = this.i18n.get('system_block_size_label');
  compressionLabel = this.i18n.get('system_data_compression_label');
  selectedTapesLabel = this.i18n.get('system_archive_selected_tape_label');
  capacityThresholdLabel = this.i18n.get(
    'common_capacity_alarm_threshold_label'
  );
  alarmThresholdLabel = this.i18n.get('common_alarm_threshold_label');
  clearThresholdLabel = this.i18n.get('system_clear_threshold_label');
  insufficientLabel = this.i18n.get('system_archive_insufficient_tape_label');
  @ViewChild('tapeTable', { static: false }) tapeTable: ProTableComponent;
  @ViewChild('capacityTpl', { static: true }) capacityTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private tapeLibraryApiService: TapeLibraryApiService
  ) {}

  ngOnInit() {
    this.initTable();
    this.initData();
  }

  initTable() {
    const tapeCols: TableCols[] = [
      {
        key: 'tapeLabel',
        name: this.i18n.get('system_archive_tape_labe_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'writeStatus',
        name: this.i18n.get('system_archive_write_status_label'),
        filter: {
          type: 'select',
          isMultiple: false,
          showCheckAll: false,
          options: this.dataMapService.toArray('Tape_Write_Status')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Tape_Write_Status')
        }
      },
      {
        key: 'status',
        name: this.i18n.get('common_status_label'),
        filter: {
          type: 'select',
          isMultiple: false,
          showCheckAll: false,
          options: this.dataMapService.toArray('Library_Tape_Status')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Library_Tape_Status')
        }
      },
      {
        key: 'worm',
        name: this.i18n.get('system_tape_type_label'),
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Media_Pool_Type')
        }
      },
      {
        key: 'usedCapacity',
        name: this.i18n.get('common_used_capcity_label'),
        thAlign: 'right',
        cellRender: this.capacityTpl
      }
    ];
    this.tapeTableConfig = {
      table: {
        async: false,
        size: 'small',
        compareWith: 'tapeLabel',
        columns: tapeCols,
        virtualScroll: true,
        colDisplayControl: false,
        virtualItemHeight: 32,
        scrollFixed: true,
        scroll: { y: '360px' },
        fetchData: (filter: Filters) => {
          this.getTapes(filter);
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true,
        pageSize: CommonConsts.PAGE_SIZE_SMALL
      }
    };
  }

  initData() {
    this.retentionType = this.dataMapService.getLabel(
      'Tape_Retention_Type',
      this.mediaSet.retentionType
    );
    this.retentionUnit = this.dataMapService.getLabel(
      'Tape_Retention_Unit',
      this.mediaSet.retentionUnit
    );
    this.tapeLibraryApiService
      .getTapeLibrariesUsingGET({
        controllerName: this.mediaSet.node,
        memberEsn: this.node?.remoteEsn
      })
      .subscribe(res => {
        if (!res.totalCount) {
          return;
        }
        this.libraries = res.records
          .filter(item => {
            return !isUndefined(
              this.mediaSet.tapes.find(
                tape => tape.tapeLibrarySn === item.serialNo
              )
            );
          })
          .map(item => {
            return { id: item.serialNo, label: item.name, selectedTapes: [] };
          });
        if (!this.libraries.length) {
          return;
        }
        this.active = first(this.libraries).id;
        this.getTapes();
      });
    this.totalTape = this.mediaSet.totalTapeCount;
    this.totalAvailableTape = this.mediaSet.availableTapeCount;
  }

  itemClick(e) {
    this.tapeTable.fetchData();
  }

  getTapes(filters?) {
    const params = {
      tapeLibrarySn: this.active,
      pageSize: CommonConsts.PAGE_SIZE,
      pageNo: CommonConsts.PAGE_START + 1,
      memberEsn: this.node?.remoteEsn
    };

    if (filters && !isEmpty(filters.conditions)) {
      const conditions = JSON.parse(filters.conditions);
      if (conditions.tapeLabel) {
        assign(params, { tapeLabel: conditions.tapeLabel });
      }
      if (conditions.writeStatus) {
        assign(params, { writeStatus: conditions.writeStatus[0] });
      }
      if (conditions.status) {
        assign(params, { status: conditions.status[0] });
      }
    }

    this.tapeLibraryApiService
      .getLibraryTapesUsingGET(params)
      .subscribe(data => {
        assign(params, { pageSize: data.totalCount });
        this.tapeLibraryApiService
          .getLibraryTapesUsingGET(params)
          .subscribe(res => {
            const tableData = res.records.filter(item => {
              return !isUndefined(
                this.mediaSet.tapes.find(
                  tape =>
                    tape.tapeLibrarySn === item.tapeLibrarySn &&
                    tape.tapeLabel === item.tapeLabel
                )
              );
            });
            this.tapeTableData = {
              data: tableData,
              total: tableData.length
            };
            this.cdr.detectChanges();
          });
      });
  }
}
