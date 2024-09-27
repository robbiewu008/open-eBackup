import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { DataReductionComponent } from './data-reduction.component';
import { BaseModule } from 'app/shared/base.module';

@NgModule({
  declarations: [DataReductionComponent],
  imports: [BaseModule, CommonModule],
  exports: [DataReductionComponent]
})
export class DataReductionModule {}
