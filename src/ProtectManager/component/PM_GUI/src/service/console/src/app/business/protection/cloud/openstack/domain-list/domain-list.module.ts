import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { DomainListComponent } from './domain-list.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { AddTelnetComputeModule } from '../../huawei-stack/stack-list/add-telnet/add-telnet.module';

@NgModule({
  declarations: [DomainListComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    AddTelnetComputeModule
  ],
  exports: [DomainListComponent]
})
export class DomainListModule {}
