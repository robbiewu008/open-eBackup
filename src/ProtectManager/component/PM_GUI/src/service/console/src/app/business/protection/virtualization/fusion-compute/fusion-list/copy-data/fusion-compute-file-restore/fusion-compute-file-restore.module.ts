import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ComputerLocationModule } from 'app/shared/components/computer-location/computer-location.module';
import { FusionComputeFileRestoreComponent } from './fusion-compute-file-restore.component';

@NgModule({
  declarations: [FusionComputeFileRestoreComponent],
  imports: [CommonModule, BaseModule, ComputerLocationModule]
})
export class FusionComputeFileRestoreModule {}
