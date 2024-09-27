import { Component, OnInit } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup
} from '@angular/forms';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  DesensitizationMode,
  I18NService,
  IdentRuleControllerService,
  MODAL_COMMON,
  PolicyControllerService
} from 'app/shared';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { assign, each, filter, isEmpty, size, trim } from 'lodash';
import { combineLatest, Observable, Observer, Subject } from 'rxjs';
import { AddIdentifiedRuleComponent } from '../../identified-rule/add-identified-rule/add-identified-rule.component';

@Component({
  selector: 'aui-create-desensitization-policy',
  templateUrl: './create-desensitization-policy.component.html',
  styleUrls: ['./create-desensitization-policy.component.less']
})
export class CreateDesensitizationPolicyComponent implements OnInit {
  rowItem;
  formGroup: FormGroup;
  desensitizationMode = DesensitizationMode;
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  total = CommonConsts.PAGE_TOTAL;
  pageIndexT = CommonConsts.PAGE_START;
  pageSizeT = CommonConsts.PAGE_SIZE;
  totalT = CommonConsts.PAGE_TOTAL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  ruleData = [];
  addRuleData = [];
  addSelection = [];
  valid$ = new Subject<boolean>();
  ruleName;
  filterParams = {};
  filterMap = this.dataMapService.toArray('Senesitization_Create_Method');
  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  constructor(
    private i18n: I18NService,
    private fb: FormBuilder,
    public baseUtilService: BaseUtilService,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private policyManagerApiService: IdentRuleControllerService,
    private policyControllerService: PolicyControllerService
  ) {}

  getAddRuleData() {
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
    this.policyManagerApiService
      .getPageIdentRulesUsingGET({
        ...params,
        ...this.filterParams
      })
      .subscribe(res => {
        this.addRuleData = res.items;
        this.total = res.total;
      });
  }

  selectionChange(e) {
    this.valid$.next(!!size(this.addSelection));
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex = page.pageIndex;
    this.getAddRuleData();
  }

  searchByName(name) {
    this.ruleName = name;
    this.getAddRuleData();
  }

  filterChange(e) {
    assign(this.filterParams, {
      createMethod: e.value
    });
    this.getAddRuleData();
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl(this.rowItem ? this.rowItem.name : '', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(64)
        ],
        asyncValidators: this.asyncValidName()
      }),
      description: new FormControl(this.rowItem ? this.rowItem.description : '')
    });
    if (this.rowItem && this.rowItem.id) {
      this.getPolicyDetail();
    }
  }

  asyncValidName() {
    return (
      control: AbstractControl
    ): Promise<{ [key: string]: any } | null> => {
      return new Promise(resolve => {
        this.policyControllerService
          .getPagePoliciesUsingGET({
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
            if (this.rowItem && this.rowItem.name && !this.rowItem.isClone) {
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

  getPolicyDetail() {
    this.policyControllerService
      .getPolicyDetailsUsingGET({
        policyId: this.rowItem.id
      })
      .subscribe(res => {
        if (res.addition_rules) {
          const identRules = [];
          each(res.addition_rules, item => {
            identRules.push(item.IdentRule);
          });
          this.addSelection = [...identRules];
        }
      });
  }

  addRules() {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('explore_add_identified_rule_label'),
      lvContent: AddIdentifiedRuleComponent,
      lvOkDisabled: true,
      lvWidth: MODAL_COMMON.largeWidth,
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as AddIdentifiedRuleComponent;
        const combined: any = combineLatest(
          content.formGroup.statusChanges,
          content.valid$
        );
        combined.subscribe(latestValues => {
          const [formGroupStatus, valid] = latestValues;
          modal.lvOkDisabled = !(formGroupStatus === 'VALID' && valid);
        });
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as AddIdentifiedRuleComponent;
          content.create().subscribe(
            res => {
              resolve(true);
              this.getAddRuleData();
            },
            error => resolve(false)
          );
        });
      }
    });
  }

  create(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const rules = [];
      if (!!size(this.addSelection)) {
        each(this.addSelection, item => {
          rules.push({
            Action: 'add',
            IdentRule: item.name
          });
        });
      }
      const params = {
        name: this.formGroup.value.name,
        description: this.formGroup.value.description,
        template_name: '',
        rules,
        create_method: DataMap.Senesitization_Create_Method.customized.value
      };
      if (this.rowItem && !this.rowItem.isClone && this.rowItem.id) {
        assign(params, {
          id: this.rowItem.id
        });
      }
      this.policyControllerService
        .createUpdatePolicyUsingPUT({
          createUpdateRequest: params
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
    this.getAddRuleData();
  }
}
