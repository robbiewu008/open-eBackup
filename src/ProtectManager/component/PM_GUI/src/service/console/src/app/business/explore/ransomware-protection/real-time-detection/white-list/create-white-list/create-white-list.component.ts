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
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  IODETECTWHITELISTService
} from 'app/shared';
import { includes, isEmpty, isUndefined, size, split, trim } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-create-white-list',
  templateUrl: './create-white-list.component.html',
  styleUrls: ['./create-white-list.component.less']
})
export class CreateWhiteListComponent implements OnInit {
  rowData;
  formGroup: FormGroup;
  dataMap = DataMap;
  typeOptions = this.dataMapService.toArray('whitelistType').filter(item => {
    return (item.isLeaf = true);
  });

  errorTip = {
    ...this.baseUtilService.requiredErrorTip,
    pathError: this.i18n.get('common_path_error_label'),
    samePathError: this.i18n.get('protection_same_path_error_label'),
    unsupportPathError: this.i18n.get(
      'protection_unsupport_fileset_linux_path_label'
    ),
    invalidExtension: this.i18n.get(
      'explore_whitelist_file_extension_valid_label'
    )
  };

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private baseUtilService: BaseUtilService,
    private ioDetectWhitelistService: IODETECTWHITELISTService
  ) {}

  ngOnInit(): void {
    this.initForm();
  }

  initForm() {
    this.formGroup = this.fb.group({
      type: new FormControl(DataMap.whitelistType.dir.value),
      name: new FormControl('', {
        validators: [this.baseUtilService.VALID.required(), this.validPath()]
      })
    });
    this.formGroup.get('type').valueChanges.subscribe(res => {
      if (res === DataMap.whitelistType.dir.value) {
        this.formGroup
          .get('name')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.validPath()
          ]);
      } else {
        this.formGroup
          .get('name')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.validExtension()
          ]);
      }
      this.formGroup.get('name').updateValueAndValidity();
    });
  }

  validExtension(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup)) {
        return null;
      }

      const values = split(control.value, ',')?.filter(item => {
        return !isEmpty(item);
      });
      const regAll = /^[a-zA-Z0-9~!@%#$&\"\'\(\)\*\+\-\.\/\:\;\<\=\>\?\[\\\]\^\_\`\{\|\}\u0020]+$/;
      for (let i = 0; i < size(values); i++) {
        const value = values[i];
        if (!regAll.test(value) || value.length > 127) {
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

  validPath(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (
        !trim(control.value) ||
        size(trim(control.value)) !== size(control.value)
      ) {
        return { pathError: { value: control.value } };
      }
      const values = split(control.value, ',')?.filter(item => {
        return !isEmpty(item);
      });
      for (let i = 0; i < size(values); i++) {
        const value = values[i];
        if (
          !CommonConsts.REGEX.templatLinuxPath.test(value) ||
          value.length > 2048
        ) {
          return { pathError: { value: control.value } };
        }
        if (includes(['proc', 'dev', 'run'], value.split('/')[1])) {
          return { unsupportPathError: { value: control.value } };
        }
      }
      return null;
    };
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = {
        content: this.formGroup.value.name,
        type: this.formGroup.value.type
      };
      this.ioDetectWhitelistService
        .createWhiteListInfo({ createWhiteListReq: params })
        .subscribe(
          () => {
            observer.next();
            observer.complete();
          },
          err => {
            observer.error(err);
            observer.complete();
          }
        );
    });
  }
}
