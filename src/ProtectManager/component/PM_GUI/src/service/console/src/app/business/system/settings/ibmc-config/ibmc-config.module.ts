import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { IbmcConfigRoutingModule } from './ibmc-config-routing.module';
import { IbmcConfigComponent } from './ibmc-config.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [IbmcConfigComponent],
  imports: [CommonModule, IbmcConfigRoutingModule, BaseModule],
  exports: [IbmcConfigComponent]
})
export class IbmcConfigModule {}
