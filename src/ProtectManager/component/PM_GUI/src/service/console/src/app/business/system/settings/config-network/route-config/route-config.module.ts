import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { RouteConfigComponent } from './route-config.component';

@NgModule({
  declarations: [RouteConfigComponent],
  imports: [CommonModule, BaseModule],
  exports: [RouteConfigComponent]
})
export class RouteConfigModule {}
