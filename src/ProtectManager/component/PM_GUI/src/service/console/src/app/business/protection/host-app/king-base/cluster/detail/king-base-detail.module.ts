import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { PostgreInstanceDatabaseListModule } from './basic-info/instance-database-list/king-base-instance-database-list.module';
import { KingBaseBasicInfoModule } from './basic-info/king-base-basic-info.module';
import { KingBaseDetailComponent } from './king-base-detail.component';

@NgModule({
  declarations: [KingBaseDetailComponent],
  imports: [
    CommonModule,
    BaseModule,
    KingBaseBasicInfoModule,
    PostgreInstanceDatabaseListModule
  ]
})
export class KingBaseDetailModule {}
