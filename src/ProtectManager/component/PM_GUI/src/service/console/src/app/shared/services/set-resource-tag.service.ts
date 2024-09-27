import { Injectable } from '@angular/core';
import { DrawModalService } from './draw-modal.service';
import { MODAL_COMMON } from '../consts';
import { AddResourceTagComponent } from '../components/add-resource-tag/add-resource-tag.component';
import { Router } from '@angular/router';
import { combineLatest } from 'rxjs';
import { isFunction } from 'lodash';

export interface SetParams {
  isAdd: boolean; // 判断是添加还是删除标签
  rowDatas: any; // 资源数据
  onOk?: () => void; // 回调
}
@Injectable({
  providedIn: 'root'
})
export class SetResourceTagService {
  constructor(
    private drawModalService: DrawModalService,
    public router: Router
  ) {}

  setTag(params: SetParams) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      ...{
        lvWidth: MODAL_COMMON.largeWidth,
        lvOkDisabled: true,
        lvContent: AddResourceTagComponent,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as AddResourceTagComponent;
          const modalIns = modal.getInstance();
          const combined: any = combineLatest([
            content.formGroup.statusChanges,
            content.selectValid$
          ]);
          combined.subscribe(latestValues => {
            const [formGroupStatus, valid] = latestValues;
            modalIns.lvOkDisabled = !valid || formGroupStatus !== 'VALID';
          });
          content.formGroup.updateValueAndValidity();
        },
        lvComponentParams: params,
        lvOk: modal => {
          this.dealConfirm(modal, params);
        },
        lvCancel: modal => {
          modal.close();
        }
      }
    });
  }
  dealConfirm(modal, params) {
    return new Promise(resolve => {
      const content = modal.getContentComponent() as AddResourceTagComponent;
      content.onOK().subscribe(
        res => {
          resolve(true);
          if (isFunction(params.onOk)) {
            params.onOk();
          }
        },
        err => {
          resolve(false);
        }
      );
    });
  }
}
