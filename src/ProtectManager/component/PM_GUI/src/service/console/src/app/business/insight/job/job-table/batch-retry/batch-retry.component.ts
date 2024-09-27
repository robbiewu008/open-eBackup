import {
  Component,
  EventEmitter,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  CommonConsts,
  I18NService,
  JOB_ORIGIN_TYPE,
  JobAPIService,
  WarningMessageService
} from 'app/shared';
import { assign, size } from 'lodash';
import { Observable, Observer } from 'rxjs';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { TableCols, TableConfig } from 'app/shared/components/pro-table';

@Component({
  selector: 'aui-batch-retry',
  templateUrl: './batch-retry.component.html',
  styleUrls: ['./batch-retry.component.css']
})
export class BatchRetryComponent implements OnInit {
  jobOriginType = JOB_ORIGIN_TYPE;
  selections;
  invalidEmitter = new EventEmitter<boolean>();
  tableConfig: TableConfig;
  tableData = {
    data: [],
    total: 0
  };
  @ViewChild('warningWindowTpl', { static: true })
  warningWindowTpl: TemplateRef<void>;

  constructor(
    private jobApiService: JobAPIService,
    public i18n: I18NService,
    private batchOperateService: BatchOperateService,
    private warningMessageService: WarningMessageService
  ) {}

  ngOnInit() {
    this.initTable();
  }

  initTable() {
    const cols: TableCols[] = [
      {
        key: 'jobId',
        name: this.i18n.get('common_job_id_label')
      },
      {
        key: 'targetLocation',
        name: this.i18n.get('protection_restore_target_label')
      }
    ];
    this.tableConfig = {
      table: {
        async: false,
        compareWith: 'jobId',
        columns: cols
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        pageSize: CommonConsts.PAGE_SIZE_SMALL,
        showTotal: true
      }
    };
  }

  updateSelections(res) {
    this.selections = res;
    this.invalidEmitter.emit(!size(this.selections));
  }

  onOK() {
    this.tableData = {
      data: this.selections,
      total: size(this.selections)
    };
    return new Observable((observer: Observer<void>) => {
      this.warningMessageService.create({
        content: this.warningWindowTpl,
        onOK: () => {
          this.batchOperateService.selfGetResults(
            item =>
              this.jobApiService.retryJobUsingPost({
                jobId: item.jobId
              }),
            this.selections.map(item => assign(item, { name: item.jobId })),
            () => {
              observer.next();
              observer.complete();
            }
          );
        },
        onCancel: () => {
          observer.error(null);
          observer.complete();
        }
      });
    });
  }
}
