import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { SanDetectionSettingListComponent } from './san-detection-setting-list.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { AlertModule } from '@iux/live';

@NgModule({
  declarations: [SanDetectionSettingListComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    AlertModule
  ],
  exports: [SanDetectionSettingListComponent]
})
export class SanDetectionSettingListModule {}
