/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
import { DatePipe } from '@angular/common';
import { Component, OnInit, TemplateRef, ViewChild } from '@angular/core';
import { FormBuilder, FormGroup } from '@angular/forms';
import { MessageService, ModalRef } from '@iux/live';
import {
  ApplicationType,
  BaseUtilService,
  DataMap,
  DataMapService,
  I18NService,
  PolicyType,
  RetentionType
} from 'app/shared';
import { SlaValidatorService } from 'app/shared/services/sla-validator.service';
import {
  assign,
  each,
  filter,
  includes,
  map,
  set,
  size,
  uniq,
  uniqBy
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-specified-archival-policy',
  templateUrl: './specified-archival-policy.component.html',
  styleUrls: ['./specified-archival-policy.component.less'],
  providers: [DatePipe]
})
export class SpecifiedArchivalPolicyComponent implements OnInit {
  sla;
  action;
  activeIndex;
  archivalData;
  applicationData;
  formGroup: FormGroup;
  applicationType = ApplicationType;
  isDataBackup = includes(
    [
      DataMap.Deploy_Type.a8000.value,
      DataMap.Deploy_Type.x3000.value,
      DataMap.Deploy_Type.x6000.value,
      DataMap.Deploy_Type.x8000.value,
      DataMap.Deploy_Type.x9000.value
    ],
    this.i18n.get('deploy_type')
  );

  @ViewChild('headerTpl', { static: true }) headerTpl: TemplateRef<any>;

  constructor(
    public baseUtilService: BaseUtilService,
    public i18n: I18NService,
    public fb: FormBuilder,
    public modal: ModalRef,
    public datePipe: DatePipe,
    public dataMapService: DataMapService,
    private messageService: MessageService,
    private slaValidatorService: SlaValidatorService
  ) {}

  ngOnInit() {
    this.getModalHeader();
    this.formGroup = this.fb.group({});
    setTimeout(() => {
      this.modal.getInstance().lvOkDisabled = this.formGroup.invalid;
    }, 0);
  }

  getModalHeader() {
    this.modal.setProperty({ lvHeader: this.headerTpl });
  }

  validFormArray() {
    const errorMsgs = [];
    const archiveTeams = this.formGroup.value.archiveTeams;
    const nameItems = uniqBy(this.formGroup.value.archiveTeams, 'name');
    if (size(nameItems) !== size(archiveTeams)) {
      this.messageService.error(
        this.i18n.get('protection_archive_name_repeats_label'),
        {
          lvMessageKey: 'lvMsg_sla_valid_error',
          lvShowCloseButton: true
        }
      );
      return false;
    }
    let storageItems = map(this.formGroup.value.archiveTeams, 'storage_id');
    storageItems = filter(storageItems, item => !!item);
    if (size(uniq(storageItems)) !== size(storageItems)) {
      this.messageService.error(
        this.i18n.get('protection_archive_storage_name_repeats_label'),
        {
          lvMessageKey: 'lvMsg_sla_valid_error',
          lvShowCloseButton: true
        }
      );
      return false;
    }
    const tapeArr = filter(
      archiveTeams,
      item => item.protocol === DataMap.Archival_Protocol.tapeLibrary.value
    );
    let arr = map(tapeArr, 'mediaSet');
    if (size(uniq(arr)) !== size(arr)) {
      this.messageService.error(
        this.i18n.get('protection_archive_storage_name_repeats_label'),
        {
          lvMessageKey: 'lvMsg_sla_valid_error',
          lvShowCloseButton: true
        }
      );
      return false;
    }

    for (const item of archiveTeams) {
      if (
        item.trigger === DataMap.Archive_Trigger.periodArchive.value &&
        item.archive_target_type ===
          DataMap.Archive_Target_Type.archiveAllCopies.value
      ) {
        const invalidIntervalResult = this.slaValidatorService.validBackupInterval(
          'interval',
          'interval_unit',
          'retention_duration',
          'duration_unit',
          item
        );
        if (invalidIntervalResult) {
          errorMsgs.push(invalidIntervalResult);
          this.messageService.error(
            this.i18n.get(invalidIntervalResult, [item.name]),
            {
              lvMessageKey: 'lvMsg_sla_valid_error',
              lvShowCloseButton: true
            }
          );
          break;
        }
      }

      const invalidInterval2Result =
        item.trigger === DataMap.Archive_Trigger.periodArchive.value
          ? this.slaValidatorService.validArchivalInterval(item)
          : null;
      if (invalidInterval2Result) {
        errorMsgs.push(invalidInterval2Result);
        this.messageService.error(
          this.i18n.get(invalidInterval2Result, [item.name]),
          {
            lvMessageKey: 'lvMsg_sla_valid_error',
            lvShowCloseButton: true
          }
        );
        break;
      }
    }

    return !size(errorMsgs);
  }

  getArchiveParams() {
    const archiveTeams = [];
    each(this.formGroup.value.archiveTeams, item => {
      archiveTeams.push(this.getParams(item));
    });
    return archiveTeams;
  }

  getParams(item) {
    const params = {
      uuid: item.uuid,
      name: item.name,
      type: PolicyType.ARCHIVING,
      action: PolicyType.ARCHIVING,
      ext_parameters: {
        specified_scope: [],
        qos_id: item.qos_id,
        protocol: item.protocol,
        auto_index: item.auto_index,
        storage_id: item.storage_id,
        archiving_scope: item.archiving_scope,
        network_access: item.network_access,
        auto_retry: item.auto_retry,
        auto_retry_times: +item.auto_retry_times,
        archive_target_type: +item.archive_target_type,
        auto_retry_wait_minutes: +item.auto_retry_wait_minutes,
        delete_import_copy: item.delete_import_copy,
        alarm_after_failure: item.alarm_after_failure
      },
      retention: {
        retention_type:
          item.protocol === DataMap.Archival_Protocol.tapeLibrary.value
            ? RetentionType.PERMANENTLY_RETAINED
            : DataMap.Interval_Unit.persistent.value === item.duration_unit
            ? RetentionType.PERMANENTLY_RETAINED
            : RetentionType.TEMPORARY_RESERVATION,
        retention_duration: +item.retention_duration,
        duration_unit: item.duration_unit
      },
      schedule: {
        trigger: item.trigger,
        interval: +item.interval,
        interval_unit: item.interval_unit,
        start_time: this.datePipe.transform(
          item.start_time,
          'yyyy-MM-dd HH:mm:ss'
        )
      }
    };

    if (
      item.archive_target_type ===
      DataMap.Archive_Target_Type.specifiedDate.value
    ) {
      delete params.retention.duration_unit;
      delete params.retention.retention_duration;
      delete params.ext_parameters.archiving_scope;

      const specified_scope = [];
      if (item.copy_type_year) {
        specified_scope.push({
          copy_type: 'year',
          generate_time_range: item.generate_time_range_year,
          retention_unit:
            item.protocol === DataMap.Archival_Protocol.tapeLibrary.value
              ? null
              : 'y',
          retention_duration:
            item.protocol === DataMap.Archival_Protocol.tapeLibrary.value
              ? null
              : +item.retention_duration_year
        });
      }
      if (item.copy_type_month) {
        specified_scope.push({
          copy_type: 'month',
          generate_time_range: item.generate_time_range_month,
          retention_unit:
            item.protocol === DataMap.Archival_Protocol.tapeLibrary.value
              ? null
              : 'MO',
          retention_duration:
            item.protocol === DataMap.Archival_Protocol.tapeLibrary.value
              ? null
              : +item.retention_duration_month
        });
      }
      if (item.copy_type_week) {
        specified_scope.push({
          copy_type: 'week',
          generate_time_range: item.generate_time_range_week,
          retention_unit:
            item.protocol === DataMap.Archival_Protocol.tapeLibrary.value
              ? null
              : 'w',
          retention_duration:
            item.protocol === DataMap.Archival_Protocol.tapeLibrary.value
              ? null
              : +item.retention_duration_week
        });
      }
      params.ext_parameters.specified_scope = specified_scope;
      params.retention.retention_type =
        item.protocol === DataMap.Archival_Protocol.tapeLibrary.value
          ? RetentionType.PERMANENTLY_RETAINED
          : RetentionType.TEMPORARY_RESERVATION;
    } else if (
      item.archive_target_type ===
      DataMap.Archive_Target_Type.archiveAllCopies.value
    ) {
      delete params.ext_parameters.specified_scope;
      if (item.duration_unit === DataMap.Interval_Unit.persistent.value) {
        delete params.retention.retention_duration;
        delete params.retention.duration_unit;
      }
    }

    if (!includes([this.applicationType.ImportCopy], this.applicationData)) {
      delete params.ext_parameters.delete_import_copy;
    }

    if (item.protocol !== DataMap.Archival_Protocol.objectStorage.value) {
      params.ext_parameters.network_access = false;
    }
    if (item.protocol === DataMap.Archival_Protocol.tapeLibrary.value) {
      set(params, 'ext_parameters.storage_list', [
        { esn: item.esn, storage_id: item.mediaSet }
      ]);
      assign(params.ext_parameters, {
        driverCount: +item.driverCount
      });
    }

    if (includes([this.applicationType.TDSQL], this.applicationData)) {
      set(params, 'ext_parameters.log_archive', item?.log_archive || false);
    }

    if (
      !(
        includes(
          [
            this.applicationType.HDFS,
            this.applicationType.NASShare,
            this.applicationType.NASFileSystem,
            this.applicationType.ImportCopy,
            this.applicationType.Fileset,
            this.applicationType.Ndmp
          ],
          this.applicationData
        ) && item.protocol === DataMap.Archival_Protocol.objectStorage.value
      ) &&
      !(
        includes(
          [
            this.applicationType.NASShare,
            this.applicationType.NASFileSystem,
            this.applicationType.Fileset,
            this.applicationType.ObjectStorage,
            this.applicationType.Ndmp
          ],
          this.applicationData
        ) && item.protocol === DataMap.Archival_Protocol.tapeLibrary.value
      )
    ) {
      delete params.ext_parameters.auto_index;
    }

    if (!item.auto_retry) {
      delete params.ext_parameters.auto_retry_times;
      delete params.ext_parameters.auto_retry_wait_minutes;
    } else {
      params.ext_parameters.auto_retry_times = +params.ext_parameters
        .auto_retry_times;
      params.ext_parameters.auto_retry_wait_minutes = +params.ext_parameters
        .auto_retry_wait_minutes;
    }

    if (item.trigger === DataMap.Archive_Trigger.immediatelyBackup.value) {
      delete params.schedule.interval;
      delete params.schedule.interval_unit;
      delete params.schedule.start_time;
    } else if (
      item.trigger === DataMap.Archive_Trigger.archiveSpecifiedTime.value
    ) {
      delete params.schedule.start_time;
      params.schedule.interval = +item.backup_generation;
      params.schedule.interval_unit = item.backup_generation_unit;
    }

    return params;
  }

  onOK(): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      if (this.formGroup.invalid) {
        return;
      }

      if (!this.validFormArray()) {
        observer.error(false);
        observer.complete();
        return;
      }

      const params = {
        newData: this.getArchiveParams(),
        originalData: this.formGroup.value.archiveTeams
      };

      observer.next(params);
      observer.complete();
    });
  }
}
