import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared/base.module';
import { BatchResultsComponent } from './batch-results.component';
import { ProTableModule } from 'app/shared/components/pro-table';
@NgModule({
  declarations: [BatchResultsComponent],
  imports: [CommonModule, BaseModule, ProTableModule],
  exports: [BatchResultsComponent]
})
export class BatchResultsModule {}
