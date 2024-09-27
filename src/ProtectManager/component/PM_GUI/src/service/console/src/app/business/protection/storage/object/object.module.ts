import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { DatabaseTemplateModule } from '../../host-app/database-template/database-template.module';
import { ObjectRoutingModule } from './object-routing.module';
import { ObjectRestoreModule } from './object-service/object-restore/object-restore.module';
import { RegisterObjectModule } from './object-service/register-object/register-object.module';
import { SummaryModule } from './object-service/summary/summary.module';
import { ObjectStorageModule } from './object-storage/object-storage.module';
import { ObjectComponent } from './object.component';

@NgModule({
  declarations: [ObjectComponent],
  imports: [
    CommonModule,
    ObjectRoutingModule,
    BaseModule,
    ObjectStorageModule,
    DatabaseTemplateModule,
    RegisterObjectModule,
    SummaryModule,
    ObjectRestoreModule
  ],
  exports: [ObjectComponent]
})
export class ObjectModule {}
