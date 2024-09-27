import { DatePipe } from '@angular/common';
import {
  AfterViewInit,
  Component,
  Input,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { ModalRef } from '@iux/live';
import { DataMapService, I18NService } from 'app/shared/services';
import { Subject } from 'rxjs';
import { Filters, ProTableComponent, TableConfig } from '../pro-table';

@Component({
  selector: 'aui-manual-index',
  templateUrl: './manual-index.component.html',
  styleUrls: ['./manual-index.component.less'],
  providers: [DatePipe]
})
export class ManualIndexComponent implements OnInit, AfterViewInit {
  @Input() rowItem;
  @Input() create;
  expanded = true;
  confirmChecked = false;
  confrimLable;
  copyTableData;
  copyTableConfig: TableConfig;
  isChecked$ = new Subject<boolean>();

  @ViewChild('copyTable', { static: false }) copyTable: ProTableComponent;
  @ViewChild('displayTimeTpl', { static: true })
  displayTimeTpl: TemplateRef<any>;
  @ViewChild('deleteHeaderTpl', { static: true })
  deleteHeaderTpl: TemplateRef<any>;

  constructor(
    private modal: ModalRef,
    private i18n: I18NService,
    private datePipe: DatePipe,
    private dataMapService: DataMapService
  ) {}

  ngAfterViewInit() {
    this.copyTable.fetchData();
  }

  ngOnInit() {
    this.initHeader();
    this.initMessage();
    this.initTableConfig();
  }

  initHeader() {
    if (!this.create) {
      this.modal.setProperty({ lvHeader: this.deleteHeaderTpl });
    }
  }

  confirmChange(e) {
    this.isChecked$.next(e);
  }

  initMessage() {
    this.confrimLable = this.create
      ? this.i18n.get('protection_manual_index_confirm_label', [
          this.datePipe.transform(
            this.rowItem.display_timestamp,
            'yyyy-MM-dd HH:mm:ss'
          ),
          5
        ])
      : this.i18n.get('protection_delete_index_confirm_label', [
          this.datePipe.transform(
            this.rowItem.display_timestamp,
            'yyyy-MM-dd HH:mm:ss'
          ),
          5
        ]);
  }

  afterExpanded = data => {
    this.expanded = true;
  };

  afterCollapsed = data => {
    this.expanded = false;
  };

  initTableConfig() {
    this.copyTableConfig = {
      table: {
        async: false,
        columns: [
          {
            key: 'display_timestamp',
            name: this.i18n.get('common_time_stamp_label'),
            cellRender: this.displayTimeTpl
          },
          {
            key: 'backup_type',
            name: this.i18n.get('common_copy_type_label'),
            cellRender: {
              type: 'status',
              config: this.dataMapService.toArray('CopyData_Backup_Type')
            }
          }
        ],
        compareWith: 'uuid',
        showLoading: false,
        colDisplayControl: false,
        size: 'small',
        fetchData: (filter: Filters) => {
          this.getCopyData(filter);
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  getCopyData(filter) {
    this.copyTableData = {
      data: [
        {
          uuid: 'd29af0ff-b130-4af3-96ed-394d6d6df35e',
          backup_type: 2,
          display_timestamp: '2021-07-06T22:12:08.681000',
          isBase: true
        },
        {
          uuid: 'd29af0ff-b130-5af3-96ed-394d6d6df35e',
          backup_type: 2,
          display_timestamp: '2021-07-06T22:12:08.681000'
        },
        {
          uuid: 'd29af0ff-b130-6af3-96ed-394d6d6df35e',
          backup_type: 2,
          display_timestamp: '2021-07-06T22:12:08.681000'
        },
        {
          uuid: 'd29af0ff-b130-7af3-96ed-394d6d6df35e',
          backup_type: 2,
          display_timestamp: '2021-07-06T22:12:08.681000'
        },
        {
          uuid: 'd29af0ff-b130-8af3-96ed-394d6d6df35e',
          backup_type: 2,
          display_timestamp: '2021-07-06T22:12:08.681000'
        }
      ],
      total: 5
    };
  }
}
