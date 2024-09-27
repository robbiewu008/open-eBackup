import {
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  Input,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { DomSanitizer, SafeHtml } from '@angular/platform-browser';
import {
  CommonConsts,
  DataMap,
  PolicyType,
  ProjectedObjectApiService,
  ProtectedResourceApiService
} from 'app/shared';
import { TableCols } from 'app/shared/components/pro-table';
import { cacheGuideResource } from 'app/shared/consts/guide-config';
import { DataMapService, I18NService } from 'app/shared/services';
import { assign, each, find, random, toUpper } from 'lodash';
import { Subject } from 'rxjs';

@Component({
  selector: 'batch-results',
  templateUrl: './batch-results.component.html',
  styleUrls: ['./batch-results.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class BatchResultsComponent implements OnInit {
  desc: SafeHtml;
  isSynExecute;
  syncNumber;
  doAction;
  sourceData;
  customLabel;
  tableData = {
    data: [],
    total: 0
  };
  tableConfig;
  pageSize = CommonConsts.PAGE_SIZE_SMALL;
  resultCountLabel;
  columns = [];
  createSuccessCount = 0;
  createFailCount = 0;
  protectSuccessCount = 0;
  protectFailCount = 0;
  valid$ = new Subject<boolean>();

  @ViewChild('descTpl', { static: true }) descTpl: TemplateRef<any>;

  @Input() advanced;
  @Input() body;
  @Input() selectionHost;
  @Input() templateOptions;
  @Input() formGroup;
  @Input() postaction;
  constructor(
    public i18n: I18NService,
    private sanitizer: DomSanitizer,
    private dataMapService: DataMapService,
    private cdr: ChangeDetectorRef,
    private protectedResourceApiService: ProtectedResourceApiService,
    private projectedObjectApiService: ProjectedObjectApiService
  ) {
    this.desc = this.sanitizer.bypassSecurityTrustHtml(
      i18n.get('common_command_successfully_label')
    );
  }

  ngOnInit() {
    this.initForm();
    this.getResult();
  }

  initForm() {
    const cols: TableCols[] = [
      {
        name: this.i18n.get('common_name_label'),
        key: 'name'
      },
      {
        name: this.i18n.get('protection_create_status_label'),
        key: 'createStatus',
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Batch_Result_Status')
        }
      },
      {
        name: this.i18n.get('protection_protected_status_label'),
        key: 'protectStatus',
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Batch_Result_Status')
        }
      },
      {
        name: this.i18n.get('common_desc_label'),
        key: 'desc',
        cellRender: this.descTpl
      }
    ];

    this.tableConfig = {
      table: {
        columns: cols,
        showLoading: false,
        colDisplayControl: false,
        size: 'small',
        async: false
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  getResult() {
    each(this.selectionHost, item => {
      this.createFileset(item);
    });
  }

  batchCreatePost(params, data) {
    this.projectedObjectApiService
      .batchCreateV1ProtectedObjectsBatchPost(params)
      .subscribe({
        next: res => {
          find(
            this.tableData.data,
            item => item.name === data.name
          ).protectStatus = DataMap.Batch_Result_Status.successful.value;
          this.tableData = {
            data: this.tableData.data,
            total: this.tableData.data.length
          };
          this.protectSuccessCount++;
          this.disableOKBtn();

          this.cdr.detectChanges();
        },
        error: err => {
          find(
            this.tableData.data,
            item => item.name === data.name
          ).protectStatus = DataMap.Batch_Result_Status.fail.value;
          find(
            this.tableData.data,
            item => item.name === data.name
          ).desc = this.sanitizer.bypassSecurityTrustHtml(
            this.i18n.get(err.error?.errorCode, err.error?.parameters)
          );
          this.tableData = {
            data: this.tableData.data,
            total: this.tableData.data.length
          };
          this.protectFailCount++;

          this.disableOKBtn();
          this.cdr.detectChanges();
        }
      });
  }

  createFileset(host) {
    const randomNum = random(1000, 9999, false);
    const params = assign(this.body, {
      name: `${
        find(this.templateOptions, {
          uuid: this.formGroup.value.template_id
        })?.name
      }_${host.environment?.endpoint?.replaceAll(/[\.\:]/g, '_')}_${randomNum}`,
      parentUuid: host.environment?.uuid
    });
    const data = {
      name: params.name,
      createStatus: DataMap.Batch_Result_Status.running.value,
      protectStatus: this.formGroup.value.sla
        ? DataMap.Batch_Result_Status.running.value
        : ''
    };

    this.tableData.data.push(data);
    this.tableData.total = this.tableData.data.length;
    this.cdr.detectChanges();

    this.protectedResourceApiService
      .CreateResource({
        CreateResourceRequestBody: params
      })
      .subscribe({
        next: res => {
          cacheGuideResource(res);
          find(
            this.tableData.data,
            item => item.name === data.name
          ).createStatus = DataMap.Batch_Result_Status.successful.value;
          this.createSuccessCount++;
          this.tableData = {
            data: this.tableData.data,
            total: this.tableData.data.length
          };
          this.disableOKBtn();
          this.cdr.detectChanges();

          if (this.formGroup.value.sla) {
            const params = {
              body: {
                sla_id: this.formGroup.value.sla_id,
                resources: [{ resource_id: res.uuid }],
                ext_parameters: {
                  ...this.advanced
                }
              },
              akDoException: false,
              akOperationTips: false,
              akLoading: false
            };

            if (this.postaction) {
              assign(params.body, {
                post_action: toUpper(PolicyType.BACKUP)
              });
            }

            this.batchCreatePost(params, data);
          }
        },
        error: err => {
          find(
            this.tableData.data,
            item => item.name === data.name
          ).createStatus = DataMap.Batch_Result_Status.fail.value;
          find(
            this.tableData.data,
            item => item.name === data.name
          ).desc = this.sanitizer.bypassSecurityTrustHtml(
            this.i18n.get(err.error?.errorCode, err.error?.parameters)
          );
          if (this.formGroup.value.sla) {
            find(
              this.tableData.data,
              item => item.name === data.name
            ).protectStatus = DataMap.Batch_Result_Status.fail.value;
            this.protectFailCount++;
          }
          this.createFailCount++;
          this.tableData = {
            data: this.tableData.data,
            total: this.tableData.data.length
          };

          this.disableOKBtn();
          this.cdr.detectChanges();
        }
      });
  }

  disableOKBtn() {
    if (this.formGroup.value.sla) {
      if (
        this.createSuccessCount + this.createFailCount ===
          this.selectionHost.length &&
        this.protectSuccessCount + this.protectFailCount ===
          this.selectionHost.length
      ) {
        this.valid$.next(true);
      } else {
        this.valid$.next(false);
      }
    } else {
      if (
        this.createSuccessCount + this.createFailCount ===
        this.selectionHost.length
      ) {
        this.valid$.next(true);
      } else {
        this.valid$.next(false);
      }
    }
  }
}
