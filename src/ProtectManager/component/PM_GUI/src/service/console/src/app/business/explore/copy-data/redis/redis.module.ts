import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';
import { RedisRoutingModule } from './redis-routing.module';
import { RedisComponent } from './redis.component';

@NgModule({
  declarations: [RedisComponent],
  imports: [
    CommonModule,
    RedisRoutingModule,
    BaseModule,
    CopyResourceListModule
  ]
})
export class RedisModule {}
