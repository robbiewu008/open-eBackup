import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { DetectionModelRoutingModule } from './detection-model-routing.module';
import { BaseModule } from 'app/shared';
import { DetectionModelComponent } from './detection-model.component';
import { ProTableModule } from 'app/shared/components/pro-table';
import { AddModelModule } from './add-model/add-model.module';

@NgModule({
  declarations: [DetectionModelComponent],
  imports: [
    CommonModule,
    DetectionModelRoutingModule,
    BaseModule,
    ProTableModule,
    AddModelModule
  ]
})
export class DetectionModelModule {}
