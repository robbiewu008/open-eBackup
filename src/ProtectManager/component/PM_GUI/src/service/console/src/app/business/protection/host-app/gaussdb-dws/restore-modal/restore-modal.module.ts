import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { ModalRestoreComponent } from './restore-modal.component';

@NgModule({
  declarations: [ModalRestoreComponent],
  imports: [CommonModule, BaseModule],
  exports: [ModalRestoreComponent]
})
export class ModalRestoreModule {}
