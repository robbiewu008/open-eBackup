import { Component, OnInit } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  AsmAuthType,
  HostService,
  I18NService,
  BaseUtilService
} from 'app/shared';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-modify-asm',
  templateUrl: './modify-asm.component.html',
  styleUrls: ['./modify-asm.component.less']
})
export class ModifyAsmComponent implements OnInit {
  data;
  asmAuthType = AsmAuthType;
  formGroup: FormGroup;
  authTypeOptions = [
    {
      key: AsmAuthType.Database,
      value: AsmAuthType.Database,
      label: this.i18n.get('protection_database_auth_label'),
      isLeaf: true
    },
    {
      key: AsmAuthType.Os,
      value: AsmAuthType.Os,
      label: this.i18n.get('protection_os_auth_label'),
      isLeaf: true
    }
  ];
  asmInstLabel;
  requiredErrorTip = this.baseUtilService.requiredErrorTip;

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private hostApiService: HostService,
    private baseUtilService: BaseUtilService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getAsmInfo();
  }

  initForm() {
    this.formGroup = this.fb.group({
      auth_type: new FormControl(AsmAuthType.Database, {
        validators: [this.baseUtilService.VALID.required()],
        updateOn: 'change'
      }),
      asm_insts: [, this.baseUtilService.VALID.required()],
      username: [, this.baseUtilService.VALID.required()],
      password: [, this.baseUtilService.VALID.required()]
    });
    this.formGroup.get('auth_type').valueChanges.subscribe(res => {
      if (res === this.asmAuthType.Os) {
        this.formGroup.removeControl('asm_insts');
        this.formGroup.removeControl('username');
        this.formGroup.removeControl('password');
      } else {
        this.formGroup.addControl(
          'asm_insts',
          new FormControl('', {
            validators: [this.baseUtilService.VALID.required()],
            updateOn: 'change'
          })
        );
        this.formGroup.addControl(
          'username',
          new FormControl('', {
            validators: [this.baseUtilService.VALID.required()],
            updateOn: 'change'
          })
        );
        this.formGroup.addControl(
          'password',
          new FormControl('', {
            validators: [this.baseUtilService.VALID.required()],
            updateOn: 'change'
          })
        );
        this.getAsmInfo();
      }
      this.formGroup.updateValueAndValidity();
    });
  }

  getAsmInfo() {
    const hostId = this.data.host_id;
    this.hostApiService
      .asmInfoV1ResourceHostHostIdAsmInfoGet({ hostId })
      .subscribe(res => {
        const asmInst = [];
        res.forEach(item => {
          asmInst.push(item.inst_name);
        });
        this.asmInstLabel = asmInst.join(';');
        this.formGroup.get('asm_insts').setValue(res.length ? res : '');
      });
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const hostId = this.data.host_id;
      const body = this.formGroup.value;
      this.hostApiService
        .asmAuthV1ResourceHostHostIdAsmAuthPost({
          hostId,
          body
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
}
