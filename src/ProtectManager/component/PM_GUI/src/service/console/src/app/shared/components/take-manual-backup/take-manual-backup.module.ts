import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared/base.module';
import { TakeManualBackupComponent } from './take-manual-backup.component';

@NgModule({
  declarations: [TakeManualBackupComponent],
  imports: [CommonModule, BaseModule],
  exports: [TakeManualBackupComponent]
})
export class TakeManualBackupModule {}
