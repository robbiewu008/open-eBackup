import {
  CdkDrag,
  CdkDragHandle,
  CdkDragPlaceholder,
  CdkDropList
} from '@angular/cdk/drag-drop';
import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { CopyRestoreComponent } from './copy-restore.component';

@NgModule({
  declarations: [CopyRestoreComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    CdkDropList,
    CdkDrag,
    CdkDragPlaceholder,
    CdkDragHandle
  ],
  exports: [CopyRestoreComponent]
})
export class CopyRestoreModule {}
