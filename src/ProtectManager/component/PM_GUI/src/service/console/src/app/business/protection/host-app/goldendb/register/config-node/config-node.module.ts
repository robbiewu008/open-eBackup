import { BaseModule } from 'app/shared';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ConfigNodeComponent } from './config-node.component';

@NgModule({
  declarations: [ConfigNodeComponent],
  imports: [CommonModule, BaseModule]
})
export class ConfigNodeModule {}
