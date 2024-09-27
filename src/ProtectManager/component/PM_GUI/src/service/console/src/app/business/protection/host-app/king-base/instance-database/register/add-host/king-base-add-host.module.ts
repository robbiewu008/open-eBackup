import { BaseModule } from 'app/shared';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { KingBaseAddHostComponent } from './king-base-add-host.component';

@NgModule({
  declarations: [KingBaseAddHostComponent],
  imports: [CommonModule, BaseModule]
})
export class KingBaseAddHostModule {}
