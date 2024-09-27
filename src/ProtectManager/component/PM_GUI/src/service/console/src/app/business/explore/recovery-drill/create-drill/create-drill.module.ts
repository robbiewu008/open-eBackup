import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';

import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { AddDrillResourceModule } from './add-drill-resource/add-drill-resource.module';
import { AddScriptModule } from './add-script/add-script.module';
import { CreateDrillRoutingModule } from './create-drill-routing.module';
import { CreateDrillComponent } from './create-drill.component';
import { AlertModule } from '@iux/live';

@NgModule({
  declarations: [CreateDrillComponent],
  imports: [
    CommonModule,
    CreateDrillRoutingModule,
    BaseModule,
    ProTableModule,
    AddDrillResourceModule,
    AddScriptModule,
    AlertModule
  ]
})
export class CreateDrillModule {}
