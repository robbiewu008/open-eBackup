import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';
import { TidbRoutingModule } from './tidb-routing.module';
import { TidbComponent } from './tidb.component';

@NgModule({
  declarations: [TidbComponent],
  imports: [CommonModule, TidbRoutingModule, BaseModule, CopyResourceListModule]
})
export class TidbModule {}
