import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ComputerLocationModule } from 'app/shared/components/computer-location/computer-location.module';
import { FileRestoreComponent } from './file-restore.component';

@NgModule({
  declarations: [FileRestoreComponent],
  imports: [CommonModule, BaseModule, ComputerLocationModule],

  exports: [FileRestoreComponent]
})
export class FileRestoreModule {}
