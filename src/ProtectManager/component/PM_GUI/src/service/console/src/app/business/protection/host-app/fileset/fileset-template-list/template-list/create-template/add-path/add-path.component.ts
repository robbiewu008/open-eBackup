import { Component, Input, OnInit } from '@angular/core';
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
  I18NService
} from 'app/shared';
import { find, includes, isEmpty, size, startsWith, trim, uniq } from 'lodash';

@Component({
  selector: 'aui-add-path',
  templateUrl: './add-path.component.html',
  styleUrls: ['./add-path.component.less']
})
export class AddPathComponent implements OnInit {
  @Input() osType;
  formGroup: FormGroup;
  pathErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    pathError: this.i18n.get('common_path_error_label'),
    samePathError: this.i18n.get('protection_same_path_error_label'),
    unsupportPathError: this.i18n.get(
      'protection_unsupport_fileset_linux_path_label'
    ),
    unsupportWindowsPathError: this.i18n.get(
      'protection_unsupport_fileset_windows_path_label'
    )
  };

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private baseUtilService: BaseUtilService
  ) {}

  ngOnInit() {
    this.initForm();
  }

  initForm() {
    this.formGroup = this.fb.group({
      paths: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.validPath(),
          this.validLinuxPath()
        ]
      })
    });
  }

  validPath(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      const paths = control.value.split(',')?.filter(item => {
        return !isEmpty(item);
      });

      if (find(paths, path => size(path) !== size(trim(path)))) {
        return { pathError: { value: control.value } };
      }
      if (paths.length !== uniq(paths).length) {
        return { samePathError: { value: control.value } };
      }
      if (this.osType === DataMap.Os_Type.windows.value) {
        if (
          find(paths, path => {
            return !CommonConsts.REGEX.templateWindowsPath.test(path);
          }) ||
          find(paths, path => {
            return path.length > 1024;
          })
        ) {
          return { pathError: { value: control.value } };
        }

        if (
          find(
            paths,
            path =>
              startsWith(path, 'C:\\DataBackup\\') || path === 'C:\\DataBackup'
          )
        ) {
          return { unsupportWindowsPathError: { value: control.value } };
        }
      } else {
        if (
          find(paths, path => {
            return !CommonConsts.REGEX.templatLinuxPath.test(path);
          }) ||
          find(paths, path => {
            return path.length > 1024;
          })
        ) {
          return { pathError: { value: control.value } };
        }

        if (
          find(
            paths,
            path =>
              startsWith(path, '/opt/DataBackup/') || path === '/opt/DataBackup'
          )
        ) {
          return { unsupportPathError: { value: control.value } };
        }
      }
      return null;
    };
  }

  validLinuxPath(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (
        !trim(control.value) ||
        this.osType === DataMap.Os_Type.windows.value
      ) {
        return null;
      }
      const paths = control.value.split(',')?.filter(item => {
        return !isEmpty(item);
      });

      for (let path of paths) {
        if (includes(['proc', 'dev', 'run'], path.split('/')[1])) {
          return { unsupportPathError: { value: control.value } };
        }
      }

      return null;
    };
  }
}
