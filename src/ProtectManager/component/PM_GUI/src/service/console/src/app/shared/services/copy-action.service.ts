import { CommonModule, DatePipe } from '@angular/common';
import { Component, Injectable, NgModule } from '@angular/core';
import { MessageboxService } from '@iux/live';
import { DetectionReportModule } from 'app/business/explore/anti-ransomware/resource-statistic/detection-repicas-list/detection-report/detection-report.module';
import { SnapshotDetailComponent } from 'app/business/explore/snapshot-data/snapshot-list/snapshot-detail/snapshot-detail.component';
import { SnapshotDetailModule } from 'app/business/explore/snapshot-data/snapshot-list/snapshot-detail/snapshot-detail.module';
import { SnapshotRestoreComponent } from 'app/business/explore/snapshot-data/snapshot-list/snapshot-restore/snapshot-restore.component';
import { SnapshotRestoreModule } from 'app/business/explore/snapshot-data/snapshot-list/snapshot-restore/snapshot-restore.module';
import { SnapshotReportComponent } from 'app/business/explore/snapshot-data/snapshot-report/snapshot-report.component';
import { SnapshotReportModule } from 'app/business/explore/snapshot-data/snapshot-report/snapshot-report.module';
import { assign, isFunction } from 'lodash';
import { Subject } from 'rxjs';
import {
  BaseModule,
  DataMap,
  LANGUAGE,
  MODAL_COMMON,
  SnapshotRstore,
  WarningMessageService
} from '..';
import { CopiesDetectReportService } from '../api/services';
import { CopyDuplicateComponent } from '../components/copy-duplicate/copy-duplicate.component';
import { FileExportComponent } from '../components/file-export/file-export.component';
import { FileExportModule } from '../components/file-export/file-export.module';
import { ManualDetectionComponent } from '../components/manual-detection/manual-detection.component';
import { ManualDetectionModule } from '../components/manual-detection/manual-detection.module';
import { ModifyRetentionPolicyComponent } from '../components/modify-retention-policy/modify-retention-policy.component';
import { ModifyRetentionPolicyModule } from '../components/modify-retention-policy/modify-retention-policy.module';
import { DrawModalService } from './draw-modal.service';
import { I18NService } from './i18n.service';
import { IoSnapshotReportModule } from 'app/business/explore/snapshot-data/io-snapshot-report/io-snapshot-report.module';
import { IoSnapshotReportComponent } from 'app/business/explore/snapshot-data/io-snapshot-report/io-snapshot-report.component';

@Injectable({
  providedIn: 'root'
})
export class CopyActionService {
  private browserActionComponent = BrowserActionComponent;
  constructor(
    private i18n: I18NService,
    private messageBox: MessageboxService,
    private drawModalService: DrawModalService,
    private warningMessageService: WarningMessageService,
    private copiesDetectReportService: CopiesDetectReportService
  ) {}

  exportFile(params) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('protection_download_file_label'),
      lvContent: FileExportComponent,
      lvOkDisabled: true,
      lvWidth: MODAL_COMMON.normalWidth,
      lvComponentParams: {
        rowItem: assign({}, params)
      },
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as FileExportComponent;
        const modalIns = modal.getInstance();
        content.fileValid$.subscribe(res => {
          modalIns.lvOkDisabled = !res;
        });
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as FileExportComponent;
          content.downloadFile();
          resolve(false);
        });
      }
    });
  }

  copyReplicate(params, callback?) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('common_replicate_label'),
      lvContent: CopyDuplicateComponent,
      lvOkDisabled: true,
      lvWidth:
        this.i18n.language === LANGUAGE.CN
          ? MODAL_COMMON.normalWidth
          : MODAL_COMMON.normalWidth + 50,
      lvComponentParams: {
        rowItem: assign({}, params)
      },
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as CopyDuplicateComponent;
        const modalIns = modal.getInstance();
        content.formGroup.statusChanges.subscribe(res => {
          modalIns.lvOkDisabled = res !== 'VALID';
        });
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as CopyDuplicateComponent;
          content.onOK().subscribe(
            res => {
              resolve(true);
              isFunction(callback) && callback();
            },
            () => resolve(false)
          );
        });
      }
    });
  }

  dealMisreport(rowData, content?: string, callback?) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvModalKey: 'warningMessage',
      ...{
        lvType: 'dialog',
        lvDialogIcon: 'lv-icon-popup-danger-48',
        lvHeader: this.i18n.get('common_danger_label'),
        lvContent: DealMisReportTipComponent,
        lvComponentParams: {
          rowData,
          content
        },
        lvWidth: 500,
        lvOkType: 'primary',
        lvCancelType: 'default',
        lvOkDisabled: true,
        lvFocusButtonId: 'cancel',
        lvCloseButtonDisplay: true,
        lvAfterOpen: modal => {
          const component = modal.getContentComponent() as DealMisReportTipComponent;
          const modalIns = modal.getInstance();
          component.isChecked$.subscribe(e => {
            modalIns.lvOkDisabled = !e;
          });
        },
        lvOk: modal => {
          const component = modal.getContentComponent() as DealMisReportTipComponent;
          this.copiesDetectReportService
            .UpdateCopyDetectionStatusCyber({
              copyId: rowData.copyId,
              extParameters: {
                is_security_snap: component.is_security_snap
              }
            })
            .subscribe(() => {
              modal.close();
              if (isFunction(callback)) {
                callback();
              }
            });
        }
      }
    });
  }

  manualDetecte(rowItem, callback?) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('explore_detect_immediately_label'),
      lvContent: ManualDetectionComponent,
      lvWidth: MODAL_COMMON.normalWidth,
      lvComponentParams: {
        rowItem
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as ManualDetectionComponent;
          content.onOK().subscribe(
            () => {
              resolve(true);
              if (isFunction(callback)) {
                callback();
              }
            },
            () => resolve(false)
          );
        });
      }
    });
  }

  modifyRetention(data, callback?) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvHeader: this.i18n.get('common_modify_retention_policy_label'),
        lvModalKey: 'modify_retention_policy',
        lvOkLoadingText: this.i18n.get('common_loading_label'),
        lvWidth: MODAL_COMMON.smallModal,
        lvContent: ModifyRetentionPolicyComponent,
        lvOkDisabled: true,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as ModifyRetentionPolicyComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res !== 'VALID';
          });
        },
        lvComponentParams: {
          data
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as ModifyRetentionPolicyComponent;
            content.onOK().subscribe({
              next: () => {
                resolve(true);
                if (isFunction(callback)) {
                  callback();
                }
              },
              error: () => resolve(false)
            });
          });
        }
      })
    );
  }

  detectionReport(data) {
    this.drawModalService.openDetailModal(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'detection-copy-report',
        lvHeader: this.i18n.get('explore_detection_report_label'),
        lvWidth: MODAL_COMMON.xLargeWidth,
        lvContent:
          data.generate_type === DataMap.snapshotGeneratetype.ioDetect.value
            ? IoSnapshotReportComponent
            : SnapshotReportComponent,
        lvComponentParams: { copyId: data.uuid, rowData: data },
        lvFooter: [
          {
            label: this.i18n.get('common_close_label'),
            onClick: modal => modal.close()
          }
        ]
      })
    );
  }

  getSnapshotDetail(title: string, rowData) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: title,
      lvModalKey: 'snapshot-detail',
      lvWidth: MODAL_COMMON.normalWidth + 100,
      lvContent: SnapshotDetailComponent,
      lvOkDisabled: true,
      lvComponentParams: {
        rowData
      },
      lvFooter: [
        {
          label: this.i18n.get('common_close_label'),
          onClick: modal => modal.close()
        }
      ]
    });
  }

  snapshotRestore(row, isResource?) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'show-fusion-compute-environment-info',
        lvWidth: this.i18n.isEn
          ? MODAL_COMMON.normalWidth + 300
          : MODAL_COMMON.normalWidth + 150,
        lvHeader: this.i18n.get('common_restore_label'),
        lvContent: SnapshotRestoreComponent,
        lvComponentParams: {
          rowData: row,
          isResource
        },
        lvOkDisabled: false,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as SnapshotRestoreComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled =
              content.formGroup.value.restoreTo === SnapshotRstore.SHAREDPATH
                ? res === 'INVALID' ||
                  (!content.formGroup.value.nfsEnable &&
                    !content.formGroup.value.cifsEnable)
                : res === 'INVALID';
          });
        },
        lvOk: modal => {
          let targetPath;
          const content = modal.getContentComponent() as SnapshotRestoreComponent;
          const name = content.getName();
          targetPath = content.getTargetPath();
          this.messageBox.danger({
            lvHeader: this.i18n.get('common_restore_tips_label'),
            lvContent: this.browserActionComponent,
            lvWidth: MODAL_COMMON.smallWidth + 100,
            lvComponentParams: {
              targetPath: targetPath,
              name
            },
            lvFooter: [
              {
                label: this.i18n.get('common_ok_label'),
                type: 'primary',
                onClick: modal => {
                  modal.close();
                  if (targetPath !== '') {
                    this.warningMessageService.create({
                      content: this.i18n.get(
                        'protection_cyber_restore_warn_label'
                      ),
                      onOK: () => {
                        return new Promise(resolve => {
                          content.restore().subscribe({
                            next: () => {
                              resolve(true);
                            },
                            error: () => {
                              resolve(false);
                            }
                          });
                        });
                      }
                    });
                  } else {
                    return new Promise(resolve => {
                      content.restore().subscribe({
                        next: () => {
                          resolve(true);
                        },
                        error: () => {
                          resolve(false);
                        }
                      });
                    });
                  }
                }
              },
              {
                label: this.i18n.get('common_cancel_label'),
                onClick: modal => {
                  modal.close();
                }
              }
            ]
          });
        }
      })
    );
  }
}

@NgModule({
  declarations: [CopyDuplicateComponent],
  imports: [
    CommonModule,
    BaseModule,
    FileExportModule,
    ManualDetectionModule,
    DetectionReportModule,
    SnapshotRestoreModule,
    SnapshotDetailModule,
    ModifyRetentionPolicyModule,
    SnapshotReportModule,
    IoSnapshotReportModule
  ],
  providers: [CopyActionService]
})
export class CopyActionModule {}

@Component({
  selector: 'aui-deal-mis-detecttion-tips',
  template: `
    <div class="warning-content">
      <span [innerHTML]="content"></span>
    </div>
    <lv-group lvGutter="14px" *ngIf="showSecurity">
      <span>
        {{ 'explore_to_secure_snapshot_label' | i18n }}
        <i
          lv-icon="aui-icon-help"
          lv-tooltip="{{ 'protection_anti_lock_enable_tip_label' | i18n }}"
          lvTooltipPosition="rightTop"
          lvTooltipTheme="light"
          class="configform-constraint"
          lvColorState="true"
        ></i>
      </span>
      <lv-switch [(ngModel)]="is_security_snap"></lv-switch>
    </lv-group>
    <div class="warning-checkbox">
      <label
        lv-checkbox
        [(ngModel)]="status"
        (ngModelChange)="warningConfirmChange($event)"
        >{{ i18n.get('common_warning_confirm_label') }}</label
      >
    </div>
  `,
  styles: [
    `
      .warning-content {
        max-height: 240px;
        overflow: auto;
        margin-bottom: 20px;
      }
    `
  ]
})
export class DealMisReportTipComponent {
  rowData;
  content;
  showSecurity = false;
  is_security_snap = false;
  status = false;
  isChecked$ = new Subject<boolean>();
  constructor(public i18n: I18NService) {}

  ngOnInit() {
    this.showSecurity =
      this.rowData?.generate_type !==
      DataMap.snapshotGeneratetype.ioDetect.value;
    this.initContent();
  }

  initContent() {
    if (!this.content) {
      this.content = this.i18n.get('explore_deal_mis_report_warn_label', [
        this.rowData?.snapshotTime
      ]);
    }
  }
  warningConfirmChange(e) {
    this.isChecked$.next(e);
  }
}
@NgModule({
  imports: [CommonModule, BaseModule],
  declarations: [DealMisReportTipComponent]
})
export class DealMisReportTipModule {}

@Component({
  selector: 'aui-browser-action',
  template: `
    <div style="margin-bottom: 15px;" [innerHTML]="tips"></div>

    <ng-container *ngIf="!isPath">
      <span style="color: #aaafbc; margin-right:15px;"
        >{{ locationLable }}
      </span>
      <span>{{ targetPath }}</span>
    </ng-container>
  `,
  styles: [],
  providers: [DatePipe]
})
export class BrowserActionComponent {
  name;
  targetPath;
  isPath = false;
  tips;
  locationLable = this.i18n.get('protection_restore_target_label');
  constructor(private i18n: I18NService, private datePipe: DatePipe) {}
  ngOnInit() {
    if (this.targetPath === '') {
      this.isPath = true;
      this.tips = this.i18n.get(
        'common_restore_snapshot_share_path_tips_label',
        [this.name]
      );
    } else {
      this.tips = this.i18n.get('common_snapshot_restore_tips_label', [
        this.name
      ]);
    }
  }
}
@NgModule({
  imports: [CommonModule],
  declarations: [BrowserActionComponent]
})
export class BrowserActionModule {}
