import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { AddStorageModule } from './add-storage/add-storage.module';
import { ExternalStorageRoutingModule } from './external-storage-routing.module';
import { ExternalStorageComponent } from './external-storage.component';

@NgModule({
  declarations: [ExternalStorageComponent],
  imports: [
    CommonModule,
    ExternalStorageRoutingModule,
    BaseModule,
    AddStorageModule,
    MultiClusterSwitchModule
  ]
})
export class ExternalStorageModule {}
