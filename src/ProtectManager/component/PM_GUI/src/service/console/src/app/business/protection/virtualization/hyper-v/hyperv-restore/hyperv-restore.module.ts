import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { HypervRestoreComponent } from './hyperv-restore.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [HypervRestoreComponent],
  imports: [CommonModule, BaseModule],
  exports: [HypervRestoreComponent]
})
export class HypervRestoreModule {}
