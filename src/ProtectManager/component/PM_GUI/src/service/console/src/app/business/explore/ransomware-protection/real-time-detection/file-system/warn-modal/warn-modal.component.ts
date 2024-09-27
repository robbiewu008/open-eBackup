import { Component, OnInit } from '@angular/core';
import {
  I18NService,
  IODETECTFILESYSTEMService,
  ResourceOperationType
} from 'app/shared';
import { ProtectionFileSystemInfo } from 'app/shared/api/models';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { assign, cloneDeep, map, pick } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-warn-modal',
  templateUrl: './warn-modal.component.html',
  styleUrls: ['./warn-modal.component.less']
})
export class WarnModalComponent implements OnInit {
  data;
  type: ResourceOperationType;
  content;
  confirmMsg = this.i18n.get('explore_disable_log_label');
  confirmCheck = false;
  confirmTips = this.i18n.get('explore_disable_log_tips_label');

  constructor(
    private i18n: I18NService,
    private batchOperateService: BatchOperateService,
    private detectFilesystemService: IODETECTFILESYSTEMService
  ) {}

  ngOnInit(): void {}

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const pickKyes = [
        'deviceId',
        'fsId',
        'fsName',
        'fsUserId',
        'id',
        'vstoreId'
      ];
      this.batchOperateService.selfGetResults(
        item => {
          return this.type === ResourceOperationType.protection
            ? this.detectFilesystemService.deactivateProtectedObjects({
                protectionDeactivateReq: {
                  isCloseFsAuditSwitch: this.confirmCheck,
                  protectionFsInfo: <ProtectionFileSystemInfo>{
                    ...pick(item, pickKyes),
                    fsUserId: item.userId
                  }
                },
                akDoException: false,
                akOperationTips: false,
                akLoading: false
              })
            : this.detectFilesystemService.deleteProtectedObjects({
                protectionDelReq: {
                  isCloseFsAuditSwitch: this.confirmCheck,
                  protectionFsInfo: <ProtectionFileSystemInfo>{
                    ...pick(item, pickKyes),
                    fsUserId: item.userId
                  }
                },
                akDoException: false,
                akOperationTips: false,
                akLoading: false
              });
        },
        map(cloneDeep(this.data), item => {
          return assign(item, {
            name: item.fsName
          });
        }),
        () => {
          observer.next();
          observer.complete();
        },
        '',
        true,
        0,
        [],
        true
      );
    });
  }
}
