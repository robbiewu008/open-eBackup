import { Component, OnInit, ViewChild } from '@angular/core';
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
  ProjectedObjectApiService,
  SnapshotRetenTion
} from 'app/shared';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import {
  assign,
  cloneDeep,
  every,
  isEmpty,
  map,
  now,
  size,
  uniqueId
} from 'lodash';
import { Observable, Observer } from 'rxjs';
import { DetectUpperBoundComponent } from '../../backup-policy/detect-upper-bound/detect-upper-bound.component';

@Component({
  selector: 'aui-manual-detecte',
  templateUrl: './manual-detecte.component.html',
  styleUrls: ['./manual-detecte.component.less']
})
export class ManualDetecteComponent implements OnInit {
  rowData: any;
  formGroup: FormGroup;
  snapshotRetenTion = SnapshotRetenTion;
  durationUnitOptions = this.dataMapService
    .toArray('Interval_Unit')
    .filter(item => {
      return [
        DataMap.Interval_Unit.day.value,
        DataMap.Interval_Unit.week.value,
        DataMap.Interval_Unit.month.value,
        DataMap.Interval_Unit.year.value,
        DataMap.Interval_Unit.persistent.value
      ].includes(item.value);
    })
    .filter(item => {
      return (item.isLeaf = true);
    });
  isOceanProtect = false;
  defaultUpperBound = 6;

  nameErrorTip = {
    invalidNameBegin: this.i18n.get('common_valid_name_begin_label'),
    invalidNameCombination: this.i18n.get(
      'common_valid_name_combination_label'
    ),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [550])
  };
  retentionErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 365])
  };
  copyNameLable: string;

  @ViewChild(DetectUpperBoundComponent, { static: false })
  DetectUpperBoundComponent: DetectUpperBoundComponent;

  constructor(
    private fb: FormBuilder,
    public i18n: I18NService,
    private dataMapService: DataMapService,
    private baseUtilService: BaseUtilService,
    private batchOperateService: BatchOperateService,
    private projectedObjectApiService: ProjectedObjectApiService
  ) {}

  ngOnInit() {
    this.initForm();
  }

  initForm() {
    this.isOceanProtect = every(
      this.rowData,
      item =>
        item.environment?.subType ===
        DataMap.cyberDeviceStorageType.OceanProtect.value
    );
    this.copyNameLable =
      size(this.rowData) > 1
        ? this.i18n.get('protection_hyperdetect_copy_name_prefix_label')
        : this.i18n.get('protection_hyperdetect_copy_name_label');
    const name = `snapshot_${now()}`;
    this.formGroup = this.fb.group({
      copy_name: new FormControl(name, {
        validators: [
          this.validCopyName(),
          this.baseUtilService.VALID.maxLength(550)
        ]
      }),
      retention_type: new FormControl(SnapshotRetenTion.permanent),
      retention_duration: new FormControl('1', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 365)
        ]
      }),
      duration_unit: new FormControl(DataMap.Interval_Unit.day.value),
      is_backup_detect_enable: new FormControl(this.isOceanProtect),
      is_security_snap: new FormControl(false)
    });
    this.listenForm();
  }

  listenForm() {
    this.formGroup
      .get('duration_unit')
      .valueChanges.subscribe(res => this.changeUnit(res));
  }

  validCopyName(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isEmpty(control.value)) {
        return null;
      }

      const value = control.value;
      const reg1 = CommonConsts.REGEX.nameBegin;
      if (!reg1.test(value)) {
        return { invalidNameBegin: { value: control.value } };
      }

      const reg2 = CommonConsts.REGEX.nameCombination;
      if (!reg2.test(value)) {
        return { invalidNameCombination: { value: control.value } };
      }

      return null;
    };
  }

  changeUnit(unit) {
    this.formGroup.get('retention_duration').enable();
    switch (unit) {
      case DataMap.Interval_Unit.day.value:
        this.formGroup
          .get('retention_duration')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 365)
          ]);
        this.retentionErrorTip = {
          ...this.baseUtilService.rangeErrorTip,
          invalidRang: this.i18n.get('common_valid_rang_label', [1, 365])
        };
        break;
      case DataMap.Interval_Unit.week.value:
        this.formGroup
          .get('retention_duration')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 54)
          ]);
        this.retentionErrorTip = {
          ...this.baseUtilService.rangeErrorTip,
          invalidRang: this.i18n.get('common_valid_rang_label', [1, 54])
        };
        break;
      case DataMap.Interval_Unit.month.value:
        this.formGroup
          .get('retention_duration')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 24)
          ]);
        this.retentionErrorTip = {
          ...this.baseUtilService.rangeErrorTip,
          invalidRang: this.i18n.get('common_valid_rang_label', [1, 24])
        };
        break;
      case DataMap.Interval_Unit.year.value:
        this.formGroup
          .get('retention_duration')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 10)
          ]);
        this.retentionErrorTip = {
          ...this.baseUtilService.rangeErrorTip,
          invalidRang: this.i18n.get('common_valid_rang_label', [1, 10])
        };
        break;
      default:
        this.formGroup.get('retention_duration').clearValidators();
        this.formGroup.get('retention_duration').disable();
        break;
    }
    this.formGroup.get('retention_duration').updateValueAndValidity();
  }

  addCopyNameSuffix(copy_name: string): string {
    const randomStr: string = uniqueId();
    if (`${copy_name}_${randomStr}`.length > 550) {
      return `${copy_name.substring(
        0,
        copy_name.length - randomStr.length - 2
      )}_${randomStr}`;
    } else {
      return `${copy_name}_${randomStr}`;
    }
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (size(this.rowData) > 1) {
        this.batchOperateService.selfGetResults(
          item => {
            const extParams = { ...this.formGroup.value };
            if (extParams.upper_bound) {
              extParams.upper_bound =
                this.DetectUpperBoundComponent?.getUpperBound(
                  extParams.upper_bound
                ) || this.defaultUpperBound;
            }
            if (!isEmpty(extParams.copy_name)) {
              extParams.copy_name = this.addCopyNameSuffix(extParams.copy_name);
            }
            if (
              this.formGroup.value.duration_unit ===
              DataMap.Interval_Unit.persistent.value
            ) {
              delete extParams.retention_duration;
              extParams.retention_type = SnapshotRetenTion.permanent;
            } else {
              extParams.retention_type = SnapshotRetenTion.specify;
            }
            return this.projectedObjectApiService.manualBackupV1ProtectedObjectsResourceIdActionBackupCyberPost(
              {
                resourceId: item.uuid,
                body: {
                  action: 'snapshot',
                  sla_id: item.sla_id,
                  ...extParams
                },
                akDoException: false,
                akOperationTips: false,
                akLoading: false
              }
            );
          },
          map(cloneDeep(this.rowData), item => {
            return assign(item, {
              isAsyn: true
            });
          }),
          () => {
            observer.next();
            observer.complete();
          },
          '',
          true
        );
      } else {
        const extParams = { ...this.formGroup.value };
        if (extParams.upper_bound) {
          extParams.upper_bound =
            this.DetectUpperBoundComponent?.getUpperBound(
              extParams.upper_bound
            ) || this.defaultUpperBound;
        }
        if (
          this.formGroup.value.duration_unit ===
          DataMap.Interval_Unit.persistent.value
        ) {
          delete extParams.retention_duration;
          extParams.retention_type = SnapshotRetenTion.permanent;
        } else {
          extParams.retention_type = SnapshotRetenTion.specify;
        }
        const params = {
          resourceId: this.rowData[0]?.uuid,
          body: {
            ...extParams,
            action: 'snapshot',
            sla_id: this.rowData[0]?.sla_id
          }
        };
        this.projectedObjectApiService
          .manualBackupV1ProtectedObjectsResourceIdActionBackupCyberPost(params)
          .subscribe({
            next: () => {
              observer.next();
              observer.complete();
            },
            error: err => {
              observer.error(err);
              observer.complete();
            }
          });
      }
    });
  }
}
