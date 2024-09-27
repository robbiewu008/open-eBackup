import { BaseModule } from 'app/shared';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { JobResourceComponent } from './job-resource.component';
import { JobTableModule } from '../job-table/job-table.module';

@NgModule({
  declarations: [JobResourceComponent],
  imports: [CommonModule, BaseModule, JobTableModule],
  exports: [JobResourceComponent]
})
export class JobResourceModule {}
