import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ObjectRestoreComponent } from './object-restore.component';

@NgModule({
  declarations: [ObjectRestoreComponent],
  imports: [CommonModule, BaseModule],
  exports: [ObjectRestoreComponent]
})
export class ObjectRestoreModule {}
