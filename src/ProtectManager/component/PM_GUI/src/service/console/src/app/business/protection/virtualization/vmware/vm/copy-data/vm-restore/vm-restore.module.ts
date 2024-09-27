import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ComputerLocationModule } from 'app/shared/components/computer-location/computer-location.module';
import { VmRestoreComponent } from './vm-restore.component';

@NgModule({
  declarations: [VmRestoreComponent],
  imports: [CommonModule, BaseModule, ComputerLocationModule]
})
export class VmRestoreModule {}
