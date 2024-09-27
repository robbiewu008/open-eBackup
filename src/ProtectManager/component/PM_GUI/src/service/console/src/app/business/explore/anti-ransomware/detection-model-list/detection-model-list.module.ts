import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { DetectionModelListComponent } from './detection-model-list.component';
import { BaseModule } from 'app/shared';
import { AddDetectionModelModule } from './add-detection-model/add-detection-model.module';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { DetectionModelListRoutingModule } from './detection-model-list-routing.module';

@NgModule({
  declarations: [DetectionModelListComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    AddDetectionModelModule,
    DetectionModelListRoutingModule
  ],
  exports: [DetectionModelListComponent]
})
export class DetectionModelListModule {}
