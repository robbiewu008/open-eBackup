import { Component, OnInit, ViewChild } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { DetectUpperBoundComponent } from 'app/business/explore/ransomware-protection/data-backup/backup-policy/detect-upper-bound/detect-upper-bound.component';
import { AntiRansomwarePolicyApiService } from 'app/shared/api/services';
import { DataMap } from 'app/shared/consts';
import { assign } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-manual-detection',
  templateUrl: './manual-detection.component.html',
  styleUrls: ['./manual-detection.component.less']
})
export class ManualDetectionComponent implements OnInit {
  rowItem: any;
  formGroup: FormGroup;
  isOceanProtect = false;
  defaultUpperBound = 6;

  @ViewChild(DetectUpperBoundComponent, { static: false })
  DetectUpperBoundComponent: DetectUpperBoundComponent;

  constructor(
    private fb: FormBuilder,
    private antiRansomwarePolicyApiService: AntiRansomwarePolicyApiService
  ) {}

  ngOnInit() {
    this.initForm();
  }

  initForm() {
    try {
      const resoure = JSON.parse(this.rowItem.resource_properties);
      this.isOceanProtect =
        resoure.environment_sub_type ===
          DataMap.cyberDeviceStorageType.OceanProtect.value ||
        resoure.environment?.subType ===
          DataMap.cyberDeviceStorageType.OceanProtect.value;
    } catch (error) {}
    this.formGroup = this.fb.group({
      is_backup_detect_enable: new FormControl(this.isOceanProtect),
      is_security_snap: new FormControl(
        this.rowItem?.is_security_snapshot ?? false
      )
    });
    if (this.isOceanProtect) {
      this.formGroup.addControl('upper_bound', new FormControl(2));
    }
  }

  onOK(): Observable<any> {
    return new Observable((observer: Observer<any>) => {
      const params = {
        copyId: this.rowItem.uuid,
        isSecuritySnap: this.formGroup.value.is_security_snap,
        isBackupDetectEnable: this.formGroup.value.is_backup_detect_enable
      };
      if (this.isOceanProtect) {
        assign(params, {
          upper_bound: this.formGroup.value.is_backup_detect_enable
            ? this.DetectUpperBoundComponent?.getUpperBound(
                this.formGroup.value.upper_bound
              ) || this.defaultUpperBound
            : this.defaultUpperBound
        });
      }
      this.antiRansomwarePolicyApiService
        .CreateCopyDetectionCyber({
          CreateCopyDetectionRequestBody: params
        })
        .subscribe({
          next: res => {
            observer.next(res);
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
