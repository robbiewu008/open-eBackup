import { CommonModule } from '@angular/common';
import {
  Component,
  Input,
  NgModule,
  OnChanges,
  SimpleChanges,
  OnInit
} from '@angular/core';
import { IconModule, TooltipModule } from '@iux/live';
import { each, isFunction } from 'lodash';
import { DataMap, RestoreType } from '../consts';
import { I18NService } from '../services';

@Component({
  selector: 'copy-features',
  template: `
    <ng-container *ngIf="!featuresParams.length">
      --
    </ng-container>
    <ng-container *ngIf="!!featuresParams.length">
      <ng-container *ngFor="let item of featuresParams">
        <i
          lv-tooltip="{{ item.content }}"
          lv-icon="{{ item.icon }}"
          class="aui-icon-copy"
          (click)="item.click()"
        ></i>
      </ng-container>
    </ng-container>
  `,
  styles: [
    `
      .aui-icon-copy {
        width: 16px;
        height: 16px;
        display: inline-block;
        margin-left: 5px;
      }
      i {
        cursor: pointer;
        color: #5b7ede;
      }
    `
  ]
})
export class FeatureComponent implements OnInit, OnChanges {
  featuresParams = [];
  @Input() features: number;
  @Input() copyData;
  @Input() restoreService;
  @Input() manualMountService;
  @Input() resType;
  @Input() onOk: () => void = () => {};

  constructor(public i18n: I18NService) {}

  ngOnInit() {
    this.initFeaturesParams();
  }

  ngOnChanges(changes: SimpleChanges) {
    this.initFeaturesParams();
  }

  initFeaturesParams() {
    const binaryStr = this.features.toString(2);
    const featuresArr = binaryStr.split('').reverse();
    this.featuresParams = [];
    each(featuresArr, (item, index) => {
      switch (index) {
        case 1:
          if (item === '1') {
            this.featuresParams.push({
              icon:
                this.copyData.status !==
                DataMap.copydata_validStatus.normal.value
                  ? 'aui-icon-restore-disabled'
                  : 'aui-icon-restore',
              content: this.i18n.get('common_restore_label'),
              click: () => {
                if (
                  this.copyData.status ===
                    DataMap.copydata_validStatus.normal.value &&
                  this.restoreService &&
                  isFunction(this.restoreService.restore)
                ) {
                  this.restoreService.restore({
                    type: this.resType,
                    childResType: this.copyData.resource_sub_type,
                    copyData: this.copyData,
                    restoreType: RestoreType.CommonRestore,
                    onOk: this.onOk
                  });
                }
              }
            });
          }
          break;
        case 2:
          if (item === '1') {
            this.featuresParams.push({
              icon:
                this.copyData.status !==
                DataMap.copydata_validStatus.normal.value
                  ? 'aui-icon-instant-restore-disabled'
                  : 'aui-icon-instant-restore',
              content: this.i18n.get('common_live_restore_job_label'),
              click: () => {
                if (
                  this.copyData.status ===
                    DataMap.copydata_validStatus.normal.value &&
                  this.restoreService &&
                  isFunction(this.restoreService.restore)
                ) {
                  this.restoreService.restore({
                    type: this.resType,
                    childResType: this.copyData.resource_sub_type,
                    copyData: this.copyData,
                    restoreType: RestoreType.InstanceRestore,
                    onOk: this.onOk
                  });
                }
              }
            });
          }
          break;
        case 3:
          if (item === '1') {
            this.featuresParams.push({
              icon:
                this.copyData.status !==
                  DataMap.copydata_validStatus.normal.value ||
                (this.copyData.generated_by ===
                DataMap.CopyData_generatedType.liveMount.value
                  ? this.copyData.generation >
                    DataMap.CopyData_Generation.two.value
                  : this.copyData.generation >=
                    DataMap.CopyData_Generation.two.value)
                  ? 'aui-icon-mount-disabled'
                  : 'aui-icon-mount',
              content: this.i18n.get('common_live_mount_label'),
              click: () => {
                if (
                  this.copyData.status ===
                    DataMap.copydata_validStatus.normal.value &&
                  (this.copyData.generated_by ===
                  DataMap.CopyData_generatedType.liveMount.value
                    ? this.copyData.generation <=
                      DataMap.CopyData_Generation.two.value
                    : this.copyData.generation <
                      DataMap.CopyData_Generation.two.value) &&
                  this.manualMountService &&
                  isFunction(this.manualMountService.create)
                ) {
                  this.manualMountService.create({
                    item: this.copyData,
                    resType: this.resType,
                    onOk: this.onOk
                  });
                }
              }
            });
          }
          break;
        default:
          break;
      }
    });
  }
}

@NgModule({
  declarations: [FeatureComponent],
  imports: [CommonModule, IconModule, TooltipModule],
  exports: [FeatureComponent]
})
export class FeatureModule {}
