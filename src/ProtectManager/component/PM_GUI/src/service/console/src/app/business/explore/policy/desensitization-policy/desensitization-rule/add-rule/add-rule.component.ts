import { Component, OnInit } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import { OptionItem } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  MaskRuleControllerService
} from 'app/shared';
import {
  assign,
  filter,
  includes,
  isEmpty,
  isString,
  toString as _toString,
  trim
} from 'lodash';
import { Observable, Observer } from 'rxjs';
import { pairwise } from 'rxjs/operators';

@Component({
  selector: 'aui-add-rule',
  templateUrl: './add-rule.component.html',
  styleUrls: ['./add-rule.component.less']
})
export class AddRuleComponent implements OnInit {
  rowItem;
  formGroup: FormGroup;
  dataMap = DataMap.Desensitization_Rule_Type;
  includes = includes;
  typeOptions = this.dataMapService
    .toArray('Desensitization_Rule_Type')
    .filter((v: OptionItem) => {
      v.isLeaf = true;
      return includes(
        [
          DataMap.Desensitization_Rule_Type.fullMask.value,
          DataMap.Desensitization_Rule_Type.partialMask.value,
          DataMap.Desensitization_Rule_Type.numberic.value,
          DataMap.Desensitization_Rule_Type.fixedNumber.value
        ],
        v.value
      );
    });
  ruleExplanation = this.i18n.get(
    DataMap.Desensitization_Rule_Type.fullMask.desc
  );
  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  alphabetErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  maskErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [1])
  };
  maskLengthErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.integerErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [0, 32])
  };
  lenErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.integerErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 32])
  };
  fixedErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.integerErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [-65535, 65536])
  };
  startErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.integerErrorTip,
    startErrorTip: this.i18n.get('explore_mask_start_error_label'),
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 32])
  };
  endErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.integerErrorTip,
    endErrorTip: this.i18n.get('explore_mask_end_error_label'),
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 32])
  };
  minErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.integerErrorTip,
    minErrorTip: this.i18n.get('explore_min_number_error_label'),
    invalidRang: this.i18n.get('common_valid_rang_label', [-65535, 65536])
  };
  maxErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.integerErrorTip,
    maxErrorTip: this.i18n.get('explore_max_number_error_label'),
    invalidRang: this.i18n.get('common_valid_rang_label', [-65535, 65536])
  };
  testDataErrorTip = {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    public baseUtilService: BaseUtilService,
    private dataMapService: DataMapService,
    private policyManagerApiService: MaskRuleControllerService
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
      type: new FormControl(DataMap.Desensitization_Rule_Type.fullMask.value, {
        validators: [this.baseUtilService.VALID.required()]
      }),
      maskAlphabet: new FormControl(''),
      maskCharacter: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(1)
        ]
      }),
      maskLength: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 32)
        ]
      }),
      maskCharacterLength: new FormControl(''),
      startIndex: new FormControl(''),
      stopIndex: new FormControl(''),
      minNumber: new FormControl(''),
      maxNumber: new FormControl(''),
      fixedNumber: new FormControl(''),
      originalData: new FormControl('', {
        validators: [this.baseUtilService.VALID.maxLength(64)]
      }),
      description: new FormControl('')
    });
    this.formGroup.get('type').valueChanges.subscribe(res => {
      const config = this.dataMapService.getValueConfig(
        'Desensitization_Rule_Type',
        res
      );
      this.ruleExplanation = this.i18n.get(config.desc);
      switch (res) {
        case DataMap.Desensitization_Rule_Type.format.value:
          this.formGroup
            .get('maskAlphabet')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.maxLength(64)
            ]);
          this.formGroup.get('maskCharacter').clearValidators();
          this.formGroup.get('maskLength').clearValidators();
          this.formGroup.get('maskCharacterLength').clearValidators();
          this.formGroup.get('startIndex').clearValidators();
          this.formGroup.get('stopIndex').clearValidators();
          this.formGroup.get('minNumber').clearValidators();
          this.formGroup.get('maxNumber').clearValidators();
          this.formGroup.get('fixedNumber').clearValidators();
          break;
        case DataMap.Desensitization_Rule_Type.fullMask.value:
          this.formGroup
            .get('maskCharacter')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.maxLength(1)
            ]);
          this.formGroup
            .get('maskLength')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(1, 32)
            ]);
          this.formGroup.get('maskAlphabet').clearValidators();
          this.formGroup.get('maskCharacterLength').clearValidators();
          this.formGroup.get('startIndex').clearValidators();
          this.formGroup.get('stopIndex').clearValidators();
          this.formGroup.get('minNumber').clearValidators();
          this.formGroup.get('maxNumber').clearValidators();
          this.formGroup.get('fixedNumber').clearValidators();
          break;
        case DataMap.Desensitization_Rule_Type.partialMask.value:
          this.formGroup
            .get('maskCharacter')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.maxLength(1)
            ]);
          this.formGroup.get('maskLength').clearValidators();
          this.formGroup
            .get('maskCharacterLength')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(0, 32)
            ]);
          this.formGroup
            .get('startIndex')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(1, 32),
              this.validMaskStart()
            ]);
          this.formGroup
            .get('stopIndex')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(1, 32),
              this.validMaskEnd()
            ]);
          this.formGroup.get('maskAlphabet').clearValidators();
          this.formGroup.get('minNumber').clearValidators();
          this.formGroup.get('maxNumber').clearValidators();
          this.formGroup.get('fixedNumber').clearValidators();
          break;
        case DataMap.Desensitization_Rule_Type.numberic.value:
          this.formGroup.get('maskAlphabet').clearValidators();
          this.formGroup.get('maskCharacter').clearValidators();
          this.formGroup.get('maskLength').clearValidators();
          this.formGroup.get('maskCharacterLength').clearValidators();
          this.formGroup.get('startIndex').clearValidators();
          this.formGroup.get('stopIndex').clearValidators();
          this.formGroup
            .get('minNumber')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(-65535, 65536),
              this.validMinNumber()
            ]);
          this.formGroup
            .get('maxNumber')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(-65535, 65536),
              this.validMaxNumber()
            ]);
          this.formGroup.get('fixedNumber').clearValidators();
          break;
        case DataMap.Desensitization_Rule_Type.fixedNumber.value:
          this.formGroup.get('maskAlphabet').clearValidators();
          this.formGroup.get('maskCharacter').clearValidators();
          this.formGroup.get('maskLength').clearValidators();
          this.formGroup.get('maskCharacterLength').clearValidators();
          this.formGroup.get('startIndex').clearValidators();
          this.formGroup.get('stopIndex').clearValidators();
          this.formGroup.get('minNumber').clearValidators();
          this.formGroup.get('maxNumber').clearValidators();
          this.formGroup
            .get('fixedNumber')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(-65535, 65536)
            ]);
          break;
      }
      this.formGroup.get('maskAlphabet').updateValueAndValidity();
      this.formGroup.get('maskCharacter').updateValueAndValidity();
      this.formGroup.get('maskLength').updateValueAndValidity();
      this.formGroup.get('maskCharacterLength').updateValueAndValidity();
      this.formGroup.get('startIndex').updateValueAndValidity();
      this.formGroup.get('stopIndex').updateValueAndValidity();
      this.formGroup.get('minNumber').updateValueAndValidity();
      this.formGroup.get('maxNumber').updateValueAndValidity();
      this.formGroup.get('fixedNumber').updateValueAndValidity();
      this.formGroup.get('originalData').setValue('');
      this.formGroup.get('description').setValue('');
    });
    if (this.rowItem) {
      this.formGroup.get('type').setValue(this.rowItem.type);
      if (this.rowItem.example) {
        this.formGroup
          .get('originalData')
          .setValue(trim(this.rowItem.example.split('->')[0]));
        this.formGroup
          .get('description')
          .setValue(trim(this.rowItem.example.split('->')[1]));
      }
      this.setFormValue();
    }
    this.formGroup
      .get('startIndex')
      .valueChanges.pipe(pairwise())
      .subscribe(res => {
        if (_toString(res[0]) === _toString(res[1])) {
          return;
        }
        setTimeout(() => {
          if (!this.formGroup.get('stopIndex').value) {
            return;
          }
          this.formGroup.get('stopIndex').markAsTouched();
          this.formGroup.get('stopIndex').updateValueAndValidity();
        }, 0);
      });
    this.formGroup
      .get('stopIndex')
      .valueChanges.pipe(pairwise())
      .subscribe(res => {
        if (_toString(res[0]) === _toString(res[1])) {
          return;
        }
        setTimeout(() => {
          if (!this.formGroup.get('startIndex').value) {
            return;
          }
          this.formGroup.get('startIndex').markAsTouched();
          this.formGroup.get('startIndex').updateValueAndValidity();
        }, 0);
      });
    this.formGroup
      .get('minNumber')
      .valueChanges.pipe(pairwise())
      .subscribe(res => {
        if (_toString(res[0]) === _toString(res[1])) {
          return;
        }
        setTimeout(() => {
          if (!this.formGroup.get('maxNumber').value) {
            return;
          }
          this.formGroup.get('maxNumber').markAsTouched();
          this.formGroup.get('maxNumber').updateValueAndValidity();
        }, 0);
      });
    this.formGroup
      .get('maxNumber')
      .valueChanges.pipe(pairwise())
      .subscribe(res => {
        if (_toString(res[0]) === _toString(res[1])) {
          return;
        }
        setTimeout(() => {
          if (!this.formGroup.get('minNumber').value) {
            return;
          }
          this.formGroup.get('minNumber').markAsTouched();
          this.formGroup.get('minNumber').updateValueAndValidity();
        }, 0);
      });
    this.formGroup.get('originalData').valueChanges.subscribe(res => {
      this.formGroup.get('description').setValue('');
    });
  }

  asyncValidName() {
    return (
      control: AbstractControl
    ): Promise<{ [key: string]: any } | null> => {
      return new Promise(resolve => {
        this.policyManagerApiService
          .getPageMaskRulesUsingGET({
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

  validMaskStart(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (
        !isEmpty(this.formGroup.value.stopIndex) &&
        this.formGroup.value.stopIndex < +control.value
      ) {
        return { startErrorTip: { value: control.value } };
      }
      return null;
    };
  }

  validMaskEnd(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (
        !isEmpty(this.formGroup.value.startIndex) &&
        +this.formGroup.value.startIndex > +control.value
      ) {
        return { endErrorTip: { value: control.value } };
      }
      return null;
    };
  }

  validMinNumber(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (
        !isEmpty(this.formGroup.value.maxNumber) &&
        +this.formGroup.value.maxNumber < +control.value
      ) {
        return { minErrorTip: { value: control.value } };
      }
      return null;
    };
  }

  validMaxNumber(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (
        !isEmpty(this.formGroup.value.minNumber) &&
        +this.formGroup.value.minNumber > +control.value
      ) {
        return { maxErrorTip: { value: control.value } };
      }
      return null;
    };
  }

  setFormValue() {
    const expression = isString(this.rowItem.expression)
      ? JSON.parse(this.rowItem.expression)
      : this.rowItem.expression;
    switch (this.rowItem.type) {
      case DataMap.Desensitization_Rule_Type.format.value:
        this.formGroup.get('maskAlphabet').setValue(expression.alphabet);
        break;
      case DataMap.Desensitization_Rule_Type.fullMask.value:
        this.formGroup.get('maskCharacter').setValue(expression.mask);
        this.formGroup.get('maskLength').setValue(expression.len);
        break;
      case DataMap.Desensitization_Rule_Type.partialMask.value:
        this.formGroup.get('maskCharacter').setValue(expression.mask);
        this.formGroup.get('maskCharacterLength').setValue(expression.len);
        this.formGroup.get('startIndex').setValue(expression.start);
        this.formGroup.get('stopIndex').setValue(expression.end);
        break;
      case DataMap.Desensitization_Rule_Type.numberic.value:
        this.formGroup.get('minNumber').setValue(expression.min);
        this.formGroup.get('maxNumber').setValue(expression.max);
        break;
      case DataMap.Desensitization_Rule_Type.fixedNumber.value:
        this.formGroup.get('fixedNumber').setValue(expression.number);
        break;
      default:
        break;
    }
  }

  generateData() {
    this.policyManagerApiService
      .generateMaskRuleExampleUsingPOST({
        exampleRequest: {
          expression: this.getExpression(),
          input: this.formGroup.value.originalData,
          type: this.formGroup.value.type
        }
      })
      .subscribe(res => {
        this.formGroup.get('description').setValue(res);
      });
  }

  getExpression() {
    const expression = {};
    switch (this.formGroup.value.type) {
      case DataMap.Desensitization_Rule_Type.format.value:
        assign(expression, {
          alphabet: this.formGroup.value.maskAlphabet
        });
        break;
      case DataMap.Desensitization_Rule_Type.fullMask.value:
        assign(expression, {
          mask: this.formGroup.value.maskCharacter,
          len: +this.formGroup.value.maskLength
        });
        break;
      case DataMap.Desensitization_Rule_Type.partialMask.value:
        assign(expression, {
          mask: this.formGroup.value.maskCharacter,
          start: +this.formGroup.value.startIndex,
          end: +this.formGroup.value.stopIndex,
          len: +this.formGroup.value.maskCharacterLength
        });
        break;
      case DataMap.Desensitization_Rule_Type.numberic.value:
        assign(expression, {
          min: +this.formGroup.value.minNumber,
          max: +this.formGroup.value.maxNumber
        });
        break;
      case DataMap.Desensitization_Rule_Type.fixedNumber.value:
        assign(expression, {
          number: +this.formGroup.value.fixedNumber
        });
        break;
    }
    return JSON.stringify(expression);
  }

  create(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.policyManagerApiService
        .createMaskRuleUsingPOST({
          createRequest: {
            create_method:
              DataMap.Senesitization_Create_Method.customized.value,
            example: this.formGroup.value.description || '',
            expression: this.getExpression(),
            name: this.formGroup.value.name,
            type: this.formGroup.value.type,
            type_description: '',
            input: this.formGroup.value.originalData || ''
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
      this.policyManagerApiService
        .modifyMaskRuleUsingPUT({
          updateRequest: {
            id: this.rowItem.id,
            create_method:
              DataMap.Senesitization_Create_Method.customized.value,
            example: this.formGroup.value.description || '',
            expression: this.getExpression(),
            name: this.formGroup.value.name,
            type: this.formGroup.value.type,
            type_description: '',
            input: this.formGroup.value.originalData || ''
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
  }
}
