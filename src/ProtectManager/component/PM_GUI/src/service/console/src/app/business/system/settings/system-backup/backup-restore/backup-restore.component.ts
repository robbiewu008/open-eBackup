import { Component, OnInit } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  BaseUtilService,
  SysbackupApiService,
  I18NService,
  WarningMessageService,
  DataMap
} from 'app/shared';
import { Observable, Observer } from 'rxjs';
import { DatePipe } from '@angular/common';

@Component({
  selector: 'aui-backup-restore',
  templateUrl: './backup-restore.component.html',
  styleUrls: ['./backup-restore.component.less'],
  providers: [DatePipe]
})
export class BackupRestoreComponent implements OnInit {
  data;
  formGroup: FormGroup;

  isCyberengine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;

  constructor(
    public fb: FormBuilder,
    public i18n: I18NService,
    private datePipe: DatePipe,
    public baseUtilService: BaseUtilService,
    private sysbackupApiService: SysbackupApiService,
    private warningMessageService: WarningMessageService
  ) {}

  ngOnInit() {
    this.initForm();
  }

  initForm() {
    this.formGroup = this.fb.group({
      password: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()],
        updateOn: 'change'
      })
    });
  }

  onOk(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.warningMessageService.create({
        content: this.i18n.get(
          this.isCyberengine
            ? 'system_cyber_backup_restore_warn_label'
            : 'system_backup_restore_warn_label',
          [this.datePipe.transform(this.data.backupTime, 'yyyy-MM-dd HH:mm:ss')]
        ),
        onOK: () => {
          if (this.formGroup.invalid) {
            return;
          }
          const params = {
            rsq: {
              password: this.formGroup.value.password
            },
            imagesId: this.data.id
          };
          this.sysbackupApiService.recoveryUsingPOST(params).subscribe(
            () => {
              observer.next();
              observer.complete();
            },
            error => {
              observer.error(error);
              observer.complete();
            }
          );
        },
        onCancel: () => {
          observer.error(null);
          observer.complete();
        },
        lvAfterClose: result => {
          if (result && result.trigger === 'close') {
            observer.error(null);
            observer.complete();
          }
        }
      });
    });
  }
}
