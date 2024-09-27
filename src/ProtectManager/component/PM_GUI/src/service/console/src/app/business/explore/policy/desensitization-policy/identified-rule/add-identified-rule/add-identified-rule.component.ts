import { Component, OnInit, ViewChild } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup
} from '@angular/forms';
import { DatatableComponent } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  ExpressionMoal,
  I18NService,
  IdentRuleControllerService,
  MODAL_COMMON,
  MaskRuleControllerService
} from 'app/shared';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { assign, each, filter, find, isEmpty, trim } from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';
import { AddRuleComponent } from '../../desensitization-rule/add-rule/add-rule.component';

@Component({
  selector: 'aui-add-identified-rule',
  templateUrl: './add-identified-rule.component.html',
  styleUrls: ['./add-identified-rule.component.less']
})
export class AddIdentifiedRuleComponent implements OnInit {
  rowItem;
  formGroup: FormGroup;
  expressionMoal = ExpressionMoal;
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  total = CommonConsts.PAGE_TOTAL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  ruleData = [];
  selectedRule;
  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  formatErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('common_valid_name_label')
  };
  valid$ = new Subject<boolean>();
  ruleName;
  filterParams = {};
  methodFilterMap = this.dataMapService.toArray('Senesitization_Create_Method');
  typeFilterMap = this.dataMapService.toArray('Desensitization_Rule_Type');
  @ViewChild(DatatableComponent, { static: false }) lvTable: DatatableComponent;
  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    public baseUtilService: BaseUtilService,
    private drawModalService: DrawModalService,
    private policyManagerApiService: IdentRuleControllerService,
    public dataMapService: DataMapService,
    private maskRuleControllerService: MaskRuleControllerService,
    private identRuleControllerService: IdentRuleControllerService
  ) {}

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl(this.rowItem ? this.rowItem.name : '', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(64)
        ],
        asyncValidators: this.asyncValidName()
      }),
      expression: new FormControl(this.rowItem ? this.rowItem.expression : '', {
        validators: [this.baseUtilService.VALID.required()]
      })
    });
    if (this.rowItem) {
      this.selectedRule = { name: this.rowItem.mask_name };
    }
  }

  asyncValidName() {
    return (
      control: AbstractControl
    ): Promise<{ [key: string]: any } | null> => {
      return new Promise(resolve => {
        this.policyManagerApiService
          .getPageIdentRulesUsingGET({
            pageNo: CommonConsts.PAGE_START,
            pageSize: CommonConsts.PAGE_SIZE * 5,
            name: control.value,
            akDoException: false,
            akLoading: false
          })
          .subscribe(res => {
            let sameName = filter(res.items, item => {
              return item.name === control.value;
            });
            // 修改不校验本身
            if (this.rowItem && !this.rowItem.isClone) {
              sameName = filter(res.items, item => {
                return (
                  item.name === control.value &&
                  this.rowItem.name !== control.value
                );
              });
            }
            if (sameName.length > 0) {
              resolve({ invalidRepeat: { value: control.value } });
            }
            resolve(null);
          });
      });
    };
  }

  addRules() {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('explore_add_desensitize_rule_label'),
      lvContent: AddRuleComponent,
      lvOkDisabled: true,
      lvWidth: MODAL_COMMON.normalWidth + 100,
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as AddRuleComponent;
        content.formGroup.statusChanges.subscribe(res => {
          modal.lvOkDisabled = res !== 'VALID';
        });
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as AddRuleComponent;
          content.create().subscribe(
            res => {
              resolve(true);
              this.getDesensitizeRule();
            },
            error => resolve(false)
          );
        });
      }
    });
  }

  getDesensitizeRule() {
    const params = {
      pageNo: this.pageIndex,
      pageSize: this.pageSize
    };
    if (trim(this.ruleName)) {
      assign(params, {
        name: trim(this.ruleName)
      });
    }
    each(this.filterParams, (value, key) => {
      if (isEmpty(value)) {
        delete this.filterParams[key];
      }
    });
    this.maskRuleControllerService
      .getPageMaskRulesUsingGET({
        ...params,
        ...this.filterParams
      })
      .subscribe(res => {
        this.ruleData = res.items;
        this.total = res.total;
        if (
          this.selectedRule &&
          find(res.items, { name: this.selectedRule.name })
        ) {
          this.selectionRow(find(res.items, { name: this.selectedRule.name }));
        }
      });
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex = page.pageIndex;
    this.getDesensitizeRule();
  }

  searchByName(name) {
    this.ruleName = name;
    this.getDesensitizeRule();
  }

  filterChange(e) {
    if (e.key === 'create_method') {
      assign(this.filterParams, {
        createMethod: e.value
      });
    }
    if (e.key === 'type') {
      assign(this.filterParams, {
        maskType: e.value
      });
    }
    this.getDesensitizeRule();
  }

  selectionRow(source) {
    this.selectedRule = source;
    this.lvTable.toggleSelection(source);
    this.valid$.next(true);
  }

  create(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.identRuleControllerService
        .createIdentRuleUsingPOST({
          createRequest: {
            name: this.formGroup.value.name,
            mask_rule_name: this.selectedRule.name,
            expression: this.formGroup.value.expression,
            format: 'regular',
            create_method: DataMap.Senesitization_Create_Method.customized.value
          }
        })
        .subscribe(
          res => {
            observer.next();
            observer.complete();
          },
          error => {
            observer.error(error);
            observer.complete();
          }
        );
    });
  }

  modify(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.identRuleControllerService
        .modifyIdentRuleUsingPUT({
          updateRequest: {
            id: this.rowItem.id,
            name: this.formGroup.value.name,
            mask_rule_name: this.selectedRule.name,
            expression: this.formGroup.value.expression,
            format: 'regular',
            create_method: DataMap.Senesitization_Create_Method.customized.value
          }
        })
        .subscribe(
          res => {
            observer.next();
            observer.complete();
          },
          error => {
            observer.error(error);
            observer.complete();
          }
        );
    });
  }

  ngOnInit() {
    this.initForm();
    this.getDesensitizeRule();
  }
}
