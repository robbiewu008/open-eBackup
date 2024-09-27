import { Component, OnInit } from '@angular/core';
import {
  I18NService,
  UsersApiService,
  BaseUtilService,
  WarningMessageService
} from 'app/shared';
import { FormGroup, FormControl, FormBuilder } from '@angular/forms';
import { Observable, Observer } from 'rxjs';
import { assign } from 'lodash';

@Component({
  selector: 'aui-unlock',
  templateUrl: './unlock.component.html',
  styleUrls: ['./unlock.component.less']
})
export class UnlockComponent implements OnInit {
  user;
  formGroup: FormGroup;
  passwordErrorTip = assign({}, this.baseUtilService.pwdErrorTip, {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  });

  constructor(
    public fb: FormBuilder,
    public i18n: I18NService,
    public baseUtilService: BaseUtilService,
    public usersApiService: UsersApiService
  ) {}

  ngOnInit() {
    this.initForm();
  }

  initForm() {
    this.formGroup = this.fb.group({
      password: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(64)
        ],
        updateOn: 'change'
      })
    });
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const userId = this.user.userId;
      const password = this.formGroup.value.password;
      this.usersApiService.unlockUsingPUT({ userId, password }).subscribe({
        next: () => {
          observer.next();
          observer.complete();
        },
        error: error => {
          observer.error(error);
          observer.complete();
        }
      });
    });
  }
}
