import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { ButtonModule } from '@iux/live';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { BatchSelectSpecificDriveComponent } from './batch-select-specific-drive.component';

@NgModule({
  declarations: [BatchSelectSpecificDriveComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    ButtonModule
  ],
  exports: [BatchSelectSpecificDriveComponent]
})
export class BatchSelectSpecificDriveModule {}
