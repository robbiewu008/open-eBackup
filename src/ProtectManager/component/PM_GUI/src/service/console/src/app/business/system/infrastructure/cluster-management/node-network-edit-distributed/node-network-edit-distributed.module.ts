import { BaseModule } from 'app/shared';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { NodeNetworkEditDistributedComponent } from './node-network-edit-distributed.component';
import { AlertModule } from '@iux/live';
import {
  ProFilterSearchModule,
  ProTableModule
} from 'app/shared/components/pro-table';

@NgModule({
  declarations: [NodeNetworkEditDistributedComponent],
  imports: [
    CommonModule,
    BaseModule,
    AlertModule,
    ProFilterSearchModule,
    ProTableModule
  ],
  entryComponents: [NodeNetworkEditDistributedComponent]
})
export class NodeNetworkEditDistributedModule {}
