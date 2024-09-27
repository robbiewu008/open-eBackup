import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import {
  AlertModule,
  DatatableModule,
  PaginatorModule,
  TransferModule
} from '@iux/live';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table/pro-table.module';
import { SelectProtectObjectsModule } from 'app/shared/components/select-protect-objects/select-protect-objects.module';
import { SetStoragePolicyModule } from '../set-storage-policy/set-storage-policy.module';
import { CreateDistributedNasComponent } from './create-distributed-nas.component';
@NgModule({
  declarations: [CreateDistributedNasComponent],
  imports: [
    CommonModule,
    BaseModule,
    SelectProtectObjectsModule,
    ProTableModule,
    DatatableModule,
    PaginatorModule,
    TransferModule,
    SetStoragePolicyModule,
    AlertModule
  ]
})
export class CreateDistributedNasModule {}
