import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { SelectTagComponent } from './select-tag.component';
import { BaseModule } from 'app/shared/base.module';

@NgModule({
  declarations: [SelectTagComponent],
  imports: [CommonModule, BaseModule],
  exports: [SelectTagComponent]
})
export class SelectTagModule {}
