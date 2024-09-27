import { BaseModule } from 'app/shared';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { JobEventComponent } from './job-event.component';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';

@NgModule({
  declarations: [JobEventComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    ProStatusModule
  ],
  exports: [JobEventComponent]
})
export class JobEventModule {}
