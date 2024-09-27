import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { DetectionSettingListComponent } from './detection-setting-list.component';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { SetFileBlockingModule } from './set-file-blocking/set-file-blocking.module';
import { RealTimeConfirmModule } from './real-time-confirm/real-time-confirm.module';

@NgModule({
  declarations: [DetectionSettingListComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    SetFileBlockingModule,
    RealTimeConfirmModule
  ],
  exports: [DetectionSettingListComponent]
})
export class DetectionSettingListModule {}
