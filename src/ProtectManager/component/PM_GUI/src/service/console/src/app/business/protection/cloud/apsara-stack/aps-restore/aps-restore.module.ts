import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ApsRestoreComponent } from './aps-restore.component';

@NgModule({
  declarations: [ApsRestoreComponent],
  imports: [CommonModule, BaseModule],
  exports: [ApsRestoreComponent]
})
export class ApsRestoreModule {}
