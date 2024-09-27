import { ChangeDetectorRef, Component, OnInit, ViewChild } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import {
  BaseUtilService,
  FileExtensionFilterManagementService,
  I18NService
} from 'app/shared';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { assign, isEmpty, isUndefined, map, size } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-add-blocking-rule',
  templateUrl: './add-blocking-rule.component.html',
  styleUrls: ['./add-blocking-rule.component.less']
})
export class AddBlockingRuleComponent implements OnInit {
  formGroup: FormGroup;
  tableData: TableData;
  tableConfig: TableConfig;
  selectionData = [];
  associateVstoreLabel = this.i18n.get('explore_associate_vstore_label');
  blockingFileLabel = this.i18n.get('explore_blocking_file_extension_label');
  extensionPlaceholder = this.i18n.get('explore_file_extension_help_label');
  extensionTipTpl = this.i18n.get('explore_blocking_file_extension_tip_label');
  extensionErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [127]),
    invalidExtension: this.i18n.get(
      'explore_valid_blocking_file_extension_label'
    )
  };

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private baseUtilService: BaseUtilService,
    public virtualScroll: VirtualScrollService,
    private fileExtensionFilterManagementService: FileExtensionFilterManagementService
  ) {}

  ngOnInit() {
    this.initForm();
    this.initConfig();
  }

  initForm() {
    this.formGroup = this.fb.group({
      extension: new FormControl('', {
        validators: [
          this.validExtension(),
          this.baseUtilService.VALID.maxLength(127)
        ],
        updateOn: 'change'
      }),
      associateVstore: new FormControl(false)
    });
    this.formGroup.get('associateVstore').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }
      setTimeout(() => {
        this.dataTable.fetchData();
      }, 0);
    });
  }

  validExtension(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup)) {
        return null;
      }
      if (!control.value) {
        return { required: { value: control.value } };
      }

      const values = control.value.split(',');
      const regAll = /^[a-zA-Z0-9~!@%#$&\"\'\(\)\*\+\-\.\/\:\;\<\=\>\?\[\\\]\^\_\`\{\|\}\u0020]+$/;
      for (let i = 0; i < size(values); i++) {
        const value = values[i];
        if (!regAll.test(value)) {
          return { invalidExtension: { value: control.value } };
        }
        if (
          value === '*' ||
          (value.indexOf('*') !== -1 && value.indexOf('*') !== value.length - 1)
        ) {
          return {
            invalidExtension: {
              value: control.value
            }
          };
        }
      }
      return null;
    };
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.fileExtensionFilterManagementService
        .addCustomizationFileExtensionFilterUsingPost({
          addCustomFileExtensionRequest: {
            fileExtensionName: this.formGroup.value.extension
          }
        })
        .subscribe(
          res => {
            if (
              !this.formGroup.value.associateVstore ||
              !size(this.selectionData)
            ) {
              observer.next();
              observer.complete();
              return;
            }
            this.fileExtensionFilterManagementService
              .importFileExtensionFilterUsingPOST({
                importUserSuffixRequest: {
                  extensions: this.formGroup.value.extension.split(','),
                  vstoreIds: map(this.selectionData, 'vstoreId')
                }
              })
              .subscribe(res => {
                observer.next();
                observer.complete();
              });
          },
          err => {
            observer.error(err);
            observer.complete();
          }
        );
    });
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'vstoreName',
        name: this.i18n.get('common_tenant_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      }
    ];

    this.tableConfig = {
      table: {
        compareWith: 'vstoreId',
        columns: cols,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        virtualScroll: true,
        scrollFixed: true,
        scroll: { y: '56vh' },
        colDisplayControl: false,
        fetchData: (filter: Filters) => {
          this.getData(filter);
        },
        selectionChange: selection => {
          this.selectionData = selection;
        }
      },
      pagination: {
        winTablePagination: true,
        showPageSizeOptions: false,
        mode: 'simple'
      }
    };
  }

  getData(filters?: Filters) {
    const params = {
      pageNum: filters.paginator.pageIndex + 1,
      pageSize: filters.paginator.pageSize
    };
    if (!isEmpty(filters.conditions)) {
      const conditions = JSON.parse(filters.conditions);
      assign(params, { vstoreName: conditions.vstoreName });
    }
    this.fileExtensionFilterManagementService
      .getVstoreInfoUsingGET(params)
      .subscribe(res => {
        this.tableData = {
          data: res.records,
          total: res.totalCount
        };
        this.cdr.detectChanges();
      });
  }
}
