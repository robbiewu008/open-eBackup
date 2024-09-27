import { BaseModule } from 'app/shared';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ModifyNetworkProcessComponent } from './modify-network-process.component';

@NgModule({
  declarations: [ModifyNetworkProcessComponent],
  imports: [CommonModule, BaseModule],
  exports: [ModifyNetworkProcessComponent]
})
export class ModifyNetworkProcessModule {}
