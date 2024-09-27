import { Component, Injectable, OnInit } from '@angular/core';
import {
  I18NService,
  MODAL_COMMON,
  ProjectedObjectApiService,
  ResourceOperationType
} from 'app/shared';
import { DataMap } from 'app/shared/consts';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { first, isFunction, size } from 'lodash';
import { combineLatest, Observable, Observer, Subject } from 'rxjs';

export interface Options {
  data: Array<any>;
  message: string;
  selection: Array<any>;
  operationType: ResourceOperationType;
  onCancel?: (modal: any) => void;
  onOK?: (modal: any) => void;
}

@Component({
  selector: 'aui-warning-batch-confirm',
  templateUrl: './warning-batch-confirm.component.html',
  styleUrls: ['./warning-batch-confirm.component.less']
})
export class WarningBatchConfirmComponent implements OnInit {
  message;
  status;
  operationType: ResourceOperationType;
  data = [];
  selection = [];
  columns = [
    {
      key: 'name',
      label: this.i18n.get('common_name_label')
    },
    {
      key: 'environment_endpoint',
      label: this.i18n.get('common_ip_address_label')
    }
  ];

  isChecked$ = new Subject<boolean>();
  selection$ = new Subject<boolean>();

  constructor(
    private i18n: I18NService,
    private batchOperateService: BatchOperateService,
    private projectedObjectApiService: ProjectedObjectApiService
  ) {}

  ngOnInit() {
    this.initColumns();
  }

  initColumns() {
    if (first(this.data)) {
      switch (first(this.data).sub_type) {
        case DataMap.Resource_Type.msVirtualMachine.value:
        case DataMap.Resource_Type.virtualMachine.value:
        case DataMap.Resource_Type.FusionCompute.value:
          this.columns = [
            {
              key: 'name',
              label: this.i18n.get('common_name_label')
            },
            {
              key: 'path',
              label: this.i18n.get('common_location_label')
            }
          ];
          break;
        default:
          break;
      }
    }
  }

  selectionChange(source) {
    this.selection$.next(!!size(source));
  }

  filterChange = e => {};

  checkboxModelChange(source) {
    this.isChecked$.next(source);
  }

  onOK(): Observable<void> {
    return new Observable<any>((observer: Observer<void>) => {
      if (!this.status) {
        return;
      }

      this.batchOperateService.selfGetResults(
        item => {
          if (this.operationType === ResourceOperationType.protection) {
            return this.projectedObjectApiService.deleteV1ProtectedObjectsDelete(
              {
                body: {
                  resource_ids: [item.uuid]
                },
                akDoException: false,
                akOperationTips: false,
                akLoading: false
              }
            );
          }
        },
        this.selection,
        () => {
          observer.next();
          observer.complete();
        },
        '',
        true
      );
    });
  }
}

@Injectable({
  providedIn: 'root'
})
export class WarningBatchConfirmsService {
  private warningBatchConfirmComponent = WarningBatchConfirmComponent;

  constructor(
    private i18n: I18NService,
    private drawModalService: DrawModalService
  ) {}

  create(options: Options) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvModalKey: 'warningBatchConfirmsModal',
      ...{
        lvType: 'dialog',
        lvDialogIcon: 'lv-icon-popup-danger-48',
        lvHeader: this.i18n.get('common_danger_label'),
        lvContent: this.warningBatchConfirmComponent,
        lvComponentParams: {
          data: options.data,
          message: options.message,
          selection: options.selection,
          operationType: options.operationType
        },
        lvWidth: MODAL_COMMON.normalWidth,
        lvOkType: 'primary',
        lvCancelType: 'default',
        lvOkDisabled: true,
        lvFocusButtonId: 'cancel',
        lvCloseButtonDisplay: true,
        lvAfterOpen: modal => {
          const component = modal.getContentComponent() as WarningBatchConfirmComponent;
          combineLatest([component.isChecked$, component.selection$]).subscribe(
            res => {
              modal.lvOkDisabled = !(res[0] && res[1]);
            }
          );
          component.selection$.next(true);
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const component = modal.getContentComponent() as WarningBatchConfirmComponent;
            component.onOK().subscribe(
              () => {
                if (isFunction(options.onOK)) {
                  options.onOK(modal);
                }
                resolve(true);
              },
              () => {
                resolve(false);
              }
            );
          });
        },
        lvCancel: modal => {
          if (isFunction(options.onCancel)) {
            options.onCancel(modal);
          }
        }
      }
    });
  }
}
