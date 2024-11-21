/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
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
