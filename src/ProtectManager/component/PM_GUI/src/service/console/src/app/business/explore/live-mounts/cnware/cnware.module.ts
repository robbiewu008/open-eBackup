import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { CnwareRoutingModule } from './cnware-routing.module';
import { CnwareComponent } from './cnware.component';
import { LiveMountsListModule } from '../live-mounts-list/live-mounts-list.module';

@NgModule({
  declarations: [CnwareComponent],
  imports: [CommonModule, CnwareRoutingModule, LiveMountsListModule]
})
export class CnwareModule {}
