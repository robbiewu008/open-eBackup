import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { LiveMountOptionsComponent } from './live-mount-options.component';
import { BaseModule } from 'app/shared';
import { LiveMountAdvancedPerformanceModule } from 'app/shared/components/live-mount-advanced-performance/live-mount-advanced-performance.module';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [LiveMountOptionsComponent],
  imports: [
    CommonModule,
    BaseModule,
    LiveMountAdvancedPerformanceModule,
    ProTableModule
  ],
  exports: [LiveMountOptionsComponent]
})
export class LiveMountOptionsModule {}
