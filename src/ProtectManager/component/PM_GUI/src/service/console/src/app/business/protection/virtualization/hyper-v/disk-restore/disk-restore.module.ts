import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { DiskRestoreComponent } from './disk-restore.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [DiskRestoreComponent],
  imports: [CommonModule, BaseModule, ProTableModule],
  exports: [DiskRestoreComponent]
})
export class DiskRestoreModule {}
