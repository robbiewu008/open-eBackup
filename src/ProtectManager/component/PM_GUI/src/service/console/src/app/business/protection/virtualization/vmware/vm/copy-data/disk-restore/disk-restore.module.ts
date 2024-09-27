import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ComputerLocationModule } from 'app/shared/components/computer-location/computer-location.module';
import { DiskRestoreComponent } from './disk-restore.component';

@NgModule({
  declarations: [DiskRestoreComponent],
  imports: [CommonModule, BaseModule, ComputerLocationModule]
})
export class DiskRestoreModule {}
