import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { LiveMountOptionsComponent } from './live-mount-options.component';
import { BaseModule } from 'app/shared';
import { ComputerLocationModule } from 'app/shared/components/computer-location/computer-location.module';

@NgModule({
  declarations: [LiveMountOptionsComponent],
  imports: [CommonModule, BaseModule, ComputerLocationModule],
  exports: [LiveMountOptionsComponent]
})
export class LiveMountOptionsModule {}
