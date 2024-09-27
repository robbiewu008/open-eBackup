import { DatePipe } from '@angular/common';
import { Component, OnInit, TemplateRef, ViewChild } from '@angular/core';
import { MessageboxService, ModalRef } from '@iux/live';
import {
  DataMap,
  GlobalService,
  I18NService,
  LiveMountAction,
  LiveMountApiService,
  MODAL_COMMON,
  WarningMessageService
} from 'app/shared';
import { assign, filter, first, intersection, size } from 'lodash';
import { LiveMountOptionsComponent as FilesetLiveMountOptionsComponent } from '../fileset/live-mount-options/live-mount-options.component';
import { LiveMountSummaryComponent as FilesetLiveMountSummaryComponent } from '../fileset/live-mount-summary/live-mount-summary.component';
import { LiveMountOptionsComponent as NasSharedLiveMountOptionsComponent } from '../nas-shared/live-mount-options/live-mount-options.component';
import { LiveMountSummaryComponent as NasSharedLiveMountSummaryComponent } from '../nas-shared/live-mount-summary/live-mount-summary.component';
import { LiveMountOptionsComponent as OracleLiveMountOptionsComponent } from '../oracle/live-mount-options/live-mount-options.component';
import { LiveMountSummaryComponent as OracleLiveMountSummaryComponent } from '../oracle/live-mount-summary/live-mount-summary.component';
import { LiveMountOptionsComponent as VMwareLiveMountOptionsComponent } from '../vmware/live-mount-options/live-mount-options.component';
import { LiveMountSummaryComponent as VMwareLiveMountSummaryComponent } from '../vmware/live-mount-summary/live-mount-summary.component';
import { LiveMountOptionsComponent as CnwareLiveMountOptionsComponent } from '../cnware/live-mount-options/live-mount-options.component';
import { LiveMountSummaryComponent as CnwareLiveMountSummaryComponent } from '../cnware/live-mount-summary/live-mount-summary.component';
import { SelectCopyDataComponent } from './select-copy-data/select-copy-data.component';
import { SelectResourceComponent } from './select-resource/select-resource.component';

@Component({
  selector: 'aui-live-mount-create',
  templateUrl: './live-mount-create.component.html',
  styleUrls: ['./live-mount-create.component.less'],
  providers: [DatePipe]
})
export class LiveMountCreateComponent implements OnInit {
  isLoading = false;
  componentData;
  activeIndex = 0;
  nextBtnDisabled = true;
  dataMap = DataMap;
  _intersection = intersection;
  confirmCopy = '';
  confirmLocation = '';
  isFileSetWinMount = false;

  @ViewChild('footerTpl', { static: true })
  footerTpl: TemplateRef<any>;
  @ViewChild('confirmTpl', { static: true })
  confirmTpl: TemplateRef<any>;
  @ViewChild(SelectResourceComponent, { static: false })
  selectResourceComponent: SelectResourceComponent;
  @ViewChild(SelectCopyDataComponent, { static: false })
  selectCopyDataComponent: SelectCopyDataComponent;
  @ViewChild(OracleLiveMountOptionsComponent, { static: false })
  oracleLiveMountOptionsComponent: OracleLiveMountOptionsComponent;
  @ViewChild(OracleLiveMountSummaryComponent, { static: false })
  oracleLiveMountSummaryComponent: OracleLiveMountSummaryComponent;
  @ViewChild(VMwareLiveMountOptionsComponent, { static: false })
  vmwareLiveMountOptionsComponent: VMwareLiveMountOptionsComponent;
  @ViewChild(VMwareLiveMountSummaryComponent, { static: false })
  vmwareLiveMountSummaryComponent: VMwareLiveMountSummaryComponent;
  @ViewChild(NasSharedLiveMountOptionsComponent, { static: false })
  nasSharedLiveMountOptionsComponent: NasSharedLiveMountOptionsComponent;
  @ViewChild(NasSharedLiveMountSummaryComponent, { static: false })
  nasSharedLiveMountSummaryComponent: NasSharedLiveMountSummaryComponent;
  @ViewChild(FilesetLiveMountOptionsComponent, { static: false })
  filesetLiveMountOptionsComponent: FilesetLiveMountOptionsComponent;
  @ViewChild(FilesetLiveMountSummaryComponent, { static: false })
  filesetLiveMountSummaryComponent: FilesetLiveMountSummaryComponent;
  @ViewChild(CnwareLiveMountOptionsComponent, { static: false })
  cnwareLiveMountOptionsComponent: CnwareLiveMountOptionsComponent;
  @ViewChild(CnwareLiveMountSummaryComponent, { static: false })
  cnwareLiveMountSummaryComponent: CnwareLiveMountSummaryComponent;

  constructor(
    public modal: ModalRef,
    private i18n: I18NService,
    private datePipe: DatePipe,
    private globalService: GlobalService,
    private messageBox: MessageboxService,
    private liveMountApiService: LiveMountApiService,
    private warningMessageService: WarningMessageService
  ) {}

  ngOnInit() {
    this.initData();
    this.getFooter();
  }

  initData() {
    assign(this.componentData, {
      selectionResource: {},
      selectionPolicy: {},
      selectionCopy: {},
      selectionMount: {}
    });
  }

  getFooter() {
    this.modal.setProperty({ lvFooter: this.footerTpl });
  }

  selectResourceChange(selection) {
    this.nextBtnDisabled = !size(selection);
  }

  selectCopyDataChange(selection) {
    this.nextBtnDisabled = !size(selection);
  }

  selectMountOptionChange(isValid) {
    this.nextBtnDisabled = !isValid;
  }

  previous() {
    this.activeIndex--;
    if (this.activeIndex === 0) {
      this.nextBtnDisabled = !size(
        this.selectResourceComponent.lvTable.getSelection()
      );
    }

    if (this.activeIndex === 1) {
      this.nextBtnDisabled =
        this.componentData.copyDataSelectionType ===
        !size(this.selectCopyDataComponent.copyTable.getSelection());
    }

    if (this.activeIndex === 2) {
      if (
        !!size(
          intersection(this.componentData.childResourceType, [
            DataMap.Resource_Type.oracle.value,
            DataMap.Resource_Type.MySQLInstance.value,
            DataMap.Resource_Type.tdsqlInstance.value
          ])
        )
      ) {
        this.nextBtnDisabled = this.oracleLiveMountOptionsComponent.formGroup.invalid;
      } else if (
        !!size(
          intersection(this.componentData.childResourceType, [
            DataMap.Resource_Type.virtualMachine.value
          ])
        )
      ) {
        this.nextBtnDisabled = this.vmwareLiveMountOptionsComponent.formGroup.invalid;
      } else if (
        !!size(
          intersection(this.componentData.childResourceType, [
            DataMap.Resource_Type.NASFileSystem.value,
            DataMap.Resource_Type.NASShare.value
          ])
        )
      ) {
        this.nextBtnDisabled = this.nasSharedLiveMountOptionsComponent.formGroup.invalid;
      } else if (
        !!size(
          intersection(this.componentData.childResourceType, [
            DataMap.Resource_Type.fileset.value,
            DataMap.Resource_Type.volume.value
          ])
        )
      ) {
        this.nextBtnDisabled = this.filesetLiveMountOptionsComponent.formGroup.invalid;
      } else if (
        !!size(
          intersection(this.componentData.childResourceType, [
            DataMap.Resource_Type.cNwareVm.value
          ])
        )
      ) {
        this.nextBtnDisabled = this.cnwareLiveMountOptionsComponent.formGroup.invalid;
      }
    }
  }

  next() {
    this.nextBtnDisabled = true;

    if (this.activeIndex === 0) {
      assign(
        this.componentData,
        this.selectResourceComponent.getComponentData()
      );
      this.selectCopyDataComponent.getTableData();
    }

    if (this.activeIndex === 1) {
      assign(
        this.componentData,
        this.selectCopyDataComponent.getComponentData()
      );

      if (
        !!size(
          intersection(this.componentData.childResourceType, [
            DataMap.Resource_Type.oracle.value,
            DataMap.Resource_Type.MySQLInstance.value,
            DataMap.Resource_Type.tdsqlInstance.value
          ])
        )
      ) {
        this.oracleLiveMountOptionsComponent.getTargetHostOptions();
        this.globalService.emitStore({
          action: LiveMountAction.SelectResource,
          state: first(this.selectResourceComponent.lvTable.getSelection())
        });
      } else if (
        !!size(
          intersection(this.componentData.childResourceType, [
            DataMap.Resource_Type.virtualMachine.value
          ])
        )
      ) {
        this.vmwareLiveMountOptionsComponent
          .getVirtualResource()
          .subscribe(() => {
            this.vmwareLiveMountOptionsComponent.initData();
          });
      } else if (
        !!size(
          intersection(this.componentData.childResourceType, [
            DataMap.Resource_Type.NASFileSystem.value,
            DataMap.Resource_Type.NASShare.value
          ])
        )
      ) {
        this.nasSharedLiveMountOptionsComponent.initData();
      } else if (
        !!size(
          intersection(this.componentData.childResourceType, [
            DataMap.Resource_Type.fileset.value
          ])
        )
      ) {
        this.filesetLiveMountOptionsComponent.isWindows =
          JSON.parse(
            this.componentData?.selectionCopy?.resource_properties || '{}'
          )['environment_os_type'] === DataMap.Os_Type.windows.value;
        this.filesetLiveMountOptionsComponent.setValidForm();
        this.filesetLiveMountOptionsComponent.hostOptions = filter(
          this.filesetLiveMountOptionsComponent.hostOptionsCache,
          item =>
            item.os_type ===
            JSON.parse(
              this.componentData?.selectionCopy?.resource_properties || '{}'
            )['environment_os_type']
        );
        this.filesetLiveMountOptionsComponent.formGroup.updateValueAndValidity();
      } else if (
        !!size(
          intersection(this.componentData.childResourceType, [
            DataMap.Resource_Type.cNwareVm.value
          ])
        )
      ) {
        this.cnwareLiveMountOptionsComponent.initData();
      }
    }

    if (this.activeIndex === 2) {
      if (
        !!size(
          intersection(this.componentData.childResourceType, [
            DataMap.Resource_Type.MySQLInstance.value,
            DataMap.Resource_Type.tdsqlInstance.value
          ])
        )
      ) {
        if (this.oracleLiveMountOptionsComponent.offlineWarnTip) {
          this.warningMessageService.create({
            content: this.oracleLiveMountOptionsComponent.offlineWarnTip,
            onOK: () => {
              assign(
                this.componentData,
                this.oracleLiveMountOptionsComponent.getComponentData()
              );
              this.oracleLiveMountSummaryComponent.getSummaryData();
              this.activeIndex++;
            },
            onCancel: () => {
              this.nextBtnDisabled = false;
            }
          });
          return;
        } else {
          assign(
            this.componentData,
            this.oracleLiveMountOptionsComponent.getComponentData()
          );
          this.oracleLiveMountSummaryComponent.getSummaryData();
        }
      } else if (
        !!size(
          intersection(this.componentData.childResourceType, [
            DataMap.Resource_Type.oracle.value
          ])
        )
      ) {
        this.warningMessageService.create({
          content: this.oracleLiveMountOptionsComponent.oracleOfflineWarnTip,
          onOK: () => {
            assign(
              this.componentData,
              this.oracleLiveMountOptionsComponent.getComponentData()
            );
            this.oracleLiveMountSummaryComponent.getSummaryData();
            this.activeIndex++;
          },
          onCancel: () => {
            this.nextBtnDisabled = false;
          }
        });
        return;
      } else if (
        !!size(
          intersection(this.componentData.childResourceType, [
            DataMap.Resource_Type.virtualMachine.value
          ])
        )
      ) {
        assign(
          this.componentData,
          this.vmwareLiveMountOptionsComponent.getComponentData()
        );
        this.vmwareLiveMountSummaryComponent.getSummaryData();
      } else if (
        !!size(
          intersection(this.componentData.childResourceType, [
            DataMap.Resource_Type.NASFileSystem.value,
            DataMap.Resource_Type.NASShare.value
          ])
        )
      ) {
        assign(
          this.componentData,
          this.nasSharedLiveMountOptionsComponent.getComponentData()
        );
        this.nasSharedLiveMountSummaryComponent.getSummaryData();
      } else if (
        !!size(
          intersection(this.componentData.childResourceType, [
            DataMap.Resource_Type.fileset.value,
            DataMap.Resource_Type.volume.value
          ])
        )
      ) {
        this.filesetLiveMountSummaryComponent.isWindows =
          JSON.parse(
            this.componentData?.selectionCopy?.resource_properties || '{}'
          )['environment_os_type'] === DataMap.Os_Type.windows.value;
        this.isFileSetWinMount =
          JSON.parse(
            this.componentData?.selectionCopy?.resource_properties || '{}'
          )['environment_os_type'] === DataMap.Os_Type.windows.value;
        assign(
          this.componentData,
          this.filesetLiveMountOptionsComponent.getComponentData()
        );
        this.filesetLiveMountSummaryComponent.getSummaryData();
      } else if (
        !!size(
          intersection(this.componentData.childResourceType, [
            DataMap.Resource_Type.cNwareVm.value
          ])
        )
      ) {
        assign(
          this.componentData,
          this.cnwareLiveMountOptionsComponent.getComponentData()
        );
        this.cnwareLiveMountSummaryComponent.getSummaryData();
      }
    }
    this.activeIndex++;
  }

  finish() {
    this.isLoading = true;

    if (
      !!size(
        intersection(this.componentData.childResourceType, [
          DataMap.Resource_Type.fileset.value,
          DataMap.Resource_Type.volume.value
        ])
      )
    ) {
      this.confirmCopy = this.datePipe.transform(
        this.componentData.selectionCopy.display_timestamp,
        'yyyy-MM-dd HH:mm:ss'
      );
      this.confirmLocation = `${this.componentData.requestParams?.parameters?.name} : ${this.componentData.requestParams?.parameters?.dstPath}`;
      if (this.isFileSetWinMount) {
        this.confirmLocation = `${this.componentData.requestParams?.parameters?.name}`;
      }
      this.messageBox.confirm({
        lvDialogIcon: 'lv-icon-status-info',
        lvHeader: this.i18n.get('protection_live_mount_confirm_header_label'),
        lvWidth: MODAL_COMMON.normalWidth,
        lvContent: this.confirmTpl,
        lvOk: () => {
          this.liveMountApiService
            .createLiveMountUsingPOST({
              liveMountObject: this.componentData.requestParams
            })
            .subscribe(
              res => {
                this.modal.close();
                this.isLoading = false;
                this.globalService.emitStore({
                  action: LiveMountAction.Create,
                  state: ''
                });
              },
              err => {
                this.isLoading = false;
              }
            );
        },
        lvCancel: () => {
          this.isLoading = false;
        },
        lvAfterClose: () => {
          this.isLoading = false;
        }
      });

      return;
    }

    this.liveMountApiService
      .createLiveMountUsingPOST({
        liveMountObject: this.componentData.requestParams
      })
      .subscribe(
        res => {
          this.modal.close();
          this.isLoading = false;
          this.globalService.emitStore({
            action: LiveMountAction.Create,
            state: ''
          });
        },
        err => {
          this.isLoading = false;
        }
      );
  }
}
