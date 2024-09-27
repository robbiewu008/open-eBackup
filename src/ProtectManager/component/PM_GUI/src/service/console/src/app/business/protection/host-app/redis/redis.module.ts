import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { RedisRoutingModule } from './redis-routing.module';
import { RedisComponent } from './redis.component';
import { RegisterRedisModule } from './register-redis/register-redis.module';
import { RedisClusterNodeModule } from './register-redis/redis-cluster-node/redis-cluster-node-redis.module';
import { RedisShowModule } from './redis-show/redis-show.module';
import { RedisSummaryModule } from './summary/summary.module';

@NgModule({
  declarations: [RedisComponent],
  imports: [
    CommonModule,
    RedisRoutingModule,
    BaseModule,
    MultiClusterSwitchModule,
    RedisShowModule,
    RedisSummaryModule,
    RegisterRedisModule,
    RedisClusterNodeModule
  ]
})
export class RedisModule {}
