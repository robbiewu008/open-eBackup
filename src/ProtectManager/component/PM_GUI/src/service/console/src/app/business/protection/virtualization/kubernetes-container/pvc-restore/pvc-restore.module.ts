import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { PvcRestoreComponent } from './pvc-restore.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [PvcRestoreComponent],
  imports: [CommonModule, BaseModule, ProTableModule],
  exports: [PvcRestoreComponent]
})
export class PvcRestoreModule {}
