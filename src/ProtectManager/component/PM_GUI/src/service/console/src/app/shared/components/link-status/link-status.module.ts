import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { AlertModule } from '@iux/live';
import { BaseModule } from 'app/shared/base.module';
import { ProStatusModule } from '../pro-status';
import { ProTableModule } from '../pro-table';
import { LinkStatusComponent } from './link-status.component';

@NgModule({
  declarations: [LinkStatusComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProStatusModule,
    ProTableModule,
    AlertModule
  ],
  exports: [LinkStatusComponent]
})
export class LinkStatusModule {}
