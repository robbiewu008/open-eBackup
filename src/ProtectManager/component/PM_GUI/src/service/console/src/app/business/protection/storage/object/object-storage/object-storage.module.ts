import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ObjectStorageComponent } from './object-storage.component';
import { RegisterObjectStorageModule } from './register-object-storage/register-object-storage.module';
import { SummaryModule } from './summary/summary.module';
import { ProButtonModule } from 'app/shared/components/pro-button';

@NgModule({
  declarations: [ObjectStorageComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProStatusModule,
    ProTableModule,
    ProButtonModule,
    SummaryModule,
    RegisterObjectStorageModule
  ],
  exports: [ObjectStorageComponent]
})
export class ObjectStorageModule {}
