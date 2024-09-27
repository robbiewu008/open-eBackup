import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ApsDiskRestoreComponent } from './aps-disk-restore.component';

@NgModule({
  declarations: [ApsDiskRestoreComponent],
  imports: [CommonModule, BaseModule, ProTableModule],
  exports: [ApsDiskRestoreComponent]
})
export class ApsDiskRestoreModule {}
