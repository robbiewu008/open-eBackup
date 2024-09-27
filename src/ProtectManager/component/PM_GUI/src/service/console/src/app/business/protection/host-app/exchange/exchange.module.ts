import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ExchangeComponent } from './exchange.component';
import { ExchangeRoutingModule } from './exchange-routing.module';
import { BaseModule } from 'app/shared';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { AvailabiltyGroupModule } from './availabilty-group/availabilty-group.module';
import { EmailModule } from './email/email.module';
import { DatabaseModule } from './database/database.module';
import { RestoreModule } from './database-group-restore/restore.module';

@NgModule({
  declarations: [ExchangeComponent],
  imports: [
    CommonModule,
    ExchangeRoutingModule,
    BaseModule,
    MultiClusterSwitchModule,
    AvailabiltyGroupModule,
    DatabaseModule,
    EmailModule,
    RestoreModule
  ]
})
export class ExchangeModule {}
