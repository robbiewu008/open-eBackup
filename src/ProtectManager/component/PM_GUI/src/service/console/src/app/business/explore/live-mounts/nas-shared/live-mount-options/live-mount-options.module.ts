import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { LiveMountOptionsComponent } from './live-mount-options.component';
import { BaseModule } from 'app/shared';
import { LiveMountAdvancedPerformanceModule } from 'app/shared/components/live-mount-advanced-performance/live-mount-advanced-performance.module';
import { CreateFileSystemModule } from 'app/shared/components/dorado-nas-restore/create-file-system/create-file-system.module';

@NgModule({
  declarations: [LiveMountOptionsComponent],
  imports: [
    CommonModule,
    BaseModule,
    LiveMountAdvancedPerformanceModule,
    CreateFileSystemModule
  ],
  exports: [LiveMountOptionsComponent]
})
export class LiveMountOptionsModule {}
