import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { MongodbRoutingModule } from './mongodb-routing.module';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';
import { MongodbComponent } from './mongodb.component';

@NgModule({
  declarations: [MongodbComponent],
  imports: [CommonModule, MongodbRoutingModule, CopyResourceListModule],
  exports: [MongodbComponent]
})
export class MongodbModule {}
