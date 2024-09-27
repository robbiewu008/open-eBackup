import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ConfigureDataplaneIpComponent } from './configure-dataplane-ip.component';

@NgModule({
  declarations: [ConfigureDataplaneIpComponent],
  imports: [CommonModule, BaseModule],
  exports: [ConfigureDataplaneIpComponent]
})
export class ConfigureDataplaneIpModule {}
