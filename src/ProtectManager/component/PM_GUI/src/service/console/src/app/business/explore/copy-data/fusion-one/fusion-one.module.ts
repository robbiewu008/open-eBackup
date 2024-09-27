import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { FusionOneRoutingModule } from './fusion-one-routing.module';
import { FusionOneComponent } from './fusion-one.component';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';

@NgModule({
  declarations: [FusionOneComponent],
  imports: [CommonModule, FusionOneRoutingModule, CopyResourceListModule]
})
export class FusionOneModule {}
