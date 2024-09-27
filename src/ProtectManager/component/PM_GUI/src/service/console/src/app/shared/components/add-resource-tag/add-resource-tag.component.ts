import {
  Component,
  Input,
  OnDestroy,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { Router } from '@angular/router';
import {
  FilterType,
  ModalRef,
  TransferColumnItem,
  TransferComponent,
  TransferTableComponent
} from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  I18NService,
  LabelApiService,
  RouterUrl
} from 'app/shared';
import { assign, each, isEmpty, isUndefined, size, unionBy } from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';

@Component({
  selector: 'aui-add-resource-tag',
  templateUrl: './add-resource-tag.component.html',
  styleUrls: ['./add-resource-tag.component.less']
})
export class AddResourceTagComponent implements OnInit, OnDestroy {
  isEmpty = isEmpty;
  visible;
  sourceColumns: TransferColumnItem[] = [];
  sourceData = [];
  sourceSelection = [];
  targetColumns: TransferColumnItem[] = [];
  total = 0;
  headerLabel = this.i18n.get('common_add_tag_label');
  tagTipLabel;
  selectValid$ = new Subject<boolean>();
  formGroup: FormGroup;
  popFormGroup: FormGroup;
  @Input() isAdd;
  @Input() rowDatas;

  @ViewChild('transfer') transfer: TransferComponent;
  @ViewChild('headerTpl', { static: true })
  headerTpl: TemplateRef<any>;

  constructor(
    public i18n: I18NService,
    private modal: ModalRef,
    public fb: FormBuilder,
    private router: Router,
    private labelApiService: LabelApiService,
    public baseUtilService: BaseUtilService
  ) {}

  ngOnDestroy() {
    this.popFormGroup.reset();
  }

  ngOnInit(): void {
    this.initColumns();
    if (this.isAdd) {
      this.tagTipLabel = this.i18n.get('common_add_tag_tip_label', [
        this.rowDatas.length
      ]);
      this.initData();
    } else {
      this.tagTipLabel = this.i18n.get('common_remove_tag_tip_label', [
        this.rowDatas.length
      ]);
      this.headerLabel = this.i18n.get('common_remove_tag_label');
      let arr = [];
      each(this.rowDatas, item => {
        if (!!size(item.labelList)) {
          arr = unionBy(arr, item.labelList, 'uuid');
        }
      });
      this.sourceData = arr;
    }
    this.modal && this.modal.setProperty({ lvHeader: this.headerTpl });

    this.formGroup = this.fb.group({});
  }
  initColumns() {
    this.sourceColumns = [
      {
        key: 'name',
        label: this.i18n.get('system_tag_name_label'),
        isHidden: false,
        filterType: FilterType.SEARCH
      }
    ];
    this.targetColumns = [
      {
        key: 'name',
        label: this.i18n.get('system_tag_name_label'),
        isHidden: false
      }
    ];
  }

  initData(e?, args?: any) {
    const params = {
      startPage: e?.paginator?.pageIndex + 1 || CommonConsts.PAGE_START_EXTRA,
      pageSize: e?.paginator?.pageSize || CommonConsts.PAGE_SIZE,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };

    if (!isEmpty(e?.filters)) {
      if (!isEmpty(e.filters.name)) {
        assign(params, { name: e.filters.name });
      }
    }

    this.labelApiService.queryLabelUsingGET(params).subscribe(res => {
      const data = [];
      each(res.records, item => {
        data.push({
          ...item
        });
      });
      this.sourceData = data;
      this.total = res?.totalCount;
    });
  }

  createTag() {
    this.visible = true;
    this.popFormGroup = this.fb.group({
      tagName: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name(),
          this.baseUtilService.VALID.maxLength(64)
        ]
      })
    });
  }

  cancel() {
    this.visible = false;
  }

  confirm() {
    if (!isEmpty(this.popFormGroup.value.tagName)) {
      this.createTagFunc();
    }
    this.visible = false;
  }

  stateChange(e) {
    this.initData(e.params);
  }

  change(e) {
    this.sourceSelection = e.selection;
    this.selectValid$.next(!!size(this.sourceSelection));
  }

  jumpTagUrl() {
    this.router.navigateByUrl(RouterUrl.SystemTagManagement);
  }

  createTagFunc() {
    this.labelApiService
      .createLabelUsingPOST({
        CreateOrUpdateLabelRequest: {
          name: this.popFormGroup.value.tagName
        }
      })
      .subscribe(() => this.initData(null, true));
  }

  getParams() {
    const params = {
      labelIdList: this.sourceSelection.map(item => item.uuid),
      resourceObject: {
        resourceObjectIdList: this.rowDatas.map(item => item.uuid),
        type: ''
      }
    };
    return params;
  }
  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params: any = this.getParams();
      if (this.isAdd) {
        // 调用增加接口
        this.labelApiService
          .createResourceLabelUsingPOST({
            CreateLabelResourceObjectRequest: params
          })
          .subscribe(
            res => {
              observer.next();
              observer.complete();
            },
            err => {
              observer.error(err);
              observer.complete();
            }
          );
      } else {
        //调用删除接口
        this.labelApiService
          .deleteResourceLabelUsingDELETE({
            CreateLabelResourceObjectRequest: params
          })
          .subscribe(
            res => {
              observer.next();
              observer.complete();
            },
            err => {
              observer.error(err);
              observer.complete();
            }
          );
      }
    });
  }
}
