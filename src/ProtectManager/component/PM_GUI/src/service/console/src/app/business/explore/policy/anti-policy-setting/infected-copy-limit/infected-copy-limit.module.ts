import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { AddLimitModule } from './add-limit/add-limit.module';
import { InfectedCopyLimitComponent } from './infected-copy-limit.component';

@NgModule({
  declarations: [InfectedCopyLimitComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    AddLimitModule
  ],
  exports: [InfectedCopyLimitComponent]
})
export class InfectedCopyLimitModule {}
