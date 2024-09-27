import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared/base.module';
import { ManualIndexComponent } from './manual-index.component';
import { ProTableModule } from '../pro-table';

@NgModule({
  declarations: [ManualIndexComponent],
  imports: [CommonModule, BaseModule, ProTableModule],
  exports: [ManualIndexComponent]
})
export class ManualIndexModule {}
