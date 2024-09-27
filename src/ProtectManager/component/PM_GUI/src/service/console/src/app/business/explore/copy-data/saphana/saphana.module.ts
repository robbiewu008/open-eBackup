import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';
import { SaphanaRoutingModule } from './saphana-routing.module';
import { SaphanaComponent } from './saphana.component';

@NgModule({
  declarations: [SaphanaComponent],
  imports: [
    CommonModule,
    SaphanaRoutingModule,
    BaseModule,
    CopyResourceListModule
  ]
})
export class SaphanaModule {}
