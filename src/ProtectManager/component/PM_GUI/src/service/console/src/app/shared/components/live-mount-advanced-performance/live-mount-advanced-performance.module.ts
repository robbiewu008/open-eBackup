import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { LiveMountAdvancedPerformanceComponent } from './live-mount-advanced-performance.component';
import { BaseModule } from 'app/shared/base.module';

@NgModule({
  declarations: [LiveMountAdvancedPerformanceComponent],
  imports: [CommonModule, BaseModule],
  exports: [LiveMountAdvancedPerformanceComponent]
})
export class LiveMountAdvancedPerformanceModule {}
