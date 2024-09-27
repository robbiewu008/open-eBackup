import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { AlertModule } from '@iux/live';
import { BaseModule } from 'app/shared/base.module';
import { BatchResultsComponent } from './batch-results.component';

@NgModule({
  declarations: [BatchResultsComponent],
  imports: [CommonModule, BaseModule, AlertModule],

  exports: [BatchResultsComponent]
})
export class BatchResultsModule {}
