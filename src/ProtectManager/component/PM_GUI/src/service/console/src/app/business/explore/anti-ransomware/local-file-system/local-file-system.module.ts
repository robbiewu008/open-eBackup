import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { LocalFileSystemRoutingModule } from './local-file-system-routing.module';
import { LocalFileSystemComponent } from './local-file-system.component';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { BaseDetectionTableComponent } from './base-detection-table/base-detection-table.component';

@NgModule({
  declarations: [LocalFileSystemComponent, BaseDetectionTableComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    LocalFileSystemRoutingModule
  ]
})
export class LocalFileSystemModule {}
