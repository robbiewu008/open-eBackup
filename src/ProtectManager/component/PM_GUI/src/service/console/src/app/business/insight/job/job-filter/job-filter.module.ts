import { BaseModule } from 'app/shared';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { JobFilterComponent } from './job-filter.component';

@NgModule({
  declarations: [JobFilterComponent],
  imports: [CommonModule, BaseModule],
  exports: [JobFilterComponent]
})
export class JobFilterModule {}
