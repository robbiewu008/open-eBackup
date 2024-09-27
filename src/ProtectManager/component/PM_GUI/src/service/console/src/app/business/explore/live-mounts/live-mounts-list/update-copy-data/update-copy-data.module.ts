import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { UpdateCopyDataComponent } from './update-copy-data.component';

@NgModule({
  declarations: [UpdateCopyDataComponent],
  imports: [CommonModule, BaseModule],
  exports: [UpdateCopyDataComponent]
})
export class UpdateCopyDataModule {}
