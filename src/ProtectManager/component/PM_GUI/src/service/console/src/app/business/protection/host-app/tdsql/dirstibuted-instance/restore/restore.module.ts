import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { RestoreComponent } from './restore.component';
import { BaseModule } from 'app/shared';
import { SelectTagModule } from 'app/shared/components/select-tag/select-tag.module';

@NgModule({
  declarations: [RestoreComponent],
  imports: [CommonModule, BaseModule, SelectTagModule],
  exports: [RestoreComponent]
})
export class RestoreModule {}
