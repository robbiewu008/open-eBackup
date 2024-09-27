import { Component, OnInit } from '@angular/core';
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
import { isEmpty, isUndefined, size, split } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-create-filter-rule',
  templateUrl: './create-filter-rule.component.html',
  styleUrls: ['./create-filter-rule.component.less']
})
export class CreateFilterRuleComponent implements OnInit {
  rowData;
  formGroup: FormGroup;

  extensionErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [127]),
    invalidExtension: this.i18n.get(
      'explore_valid_blocking_file_extension_label'
    )
  };

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private baseUtilService: BaseUtilService,
    private fileExtensionFilterManagementService: FileExtensionFilterManagementService
  ) {}

  ngOnInit() {
    this.initForm();
  }

  initForm() {
    this.formGroup = this.fb.group({
      extension: new FormControl(this.rowData?.fileExtensionName || '', {
        validators: [
          this.validExtension(),
          this.baseUtilService.VALID.maxLength(127)
        ]
      })
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

      const values = split(control.value, ',')?.filter(item => {
        return !isEmpty(item);
      });
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
          () => {
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
}
