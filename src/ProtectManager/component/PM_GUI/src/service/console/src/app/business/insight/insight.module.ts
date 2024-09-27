import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';

import { InsightRoutingModule } from './insight-routing.module';
import {
  TabsModule,
  InputModule,
  OperationmenuModule,
  CheckboxModule,
  SearchModule as LiveSearchModule,
  PopoverModule,
  TooltipModule,
  RadioModule,
  MenuModule,
  GroupModule,
  LayoutModule
} from '@iux/live';
import { InsightComponent } from './insight.component';

@NgModule({
  declarations: [InsightComponent],
  imports: [
    BaseModule,
    InsightRoutingModule,
    TabsModule,
    InputModule,
    OperationmenuModule,
    CheckboxModule,
    LiveSearchModule,
    PopoverModule,
    TooltipModule,
    RadioModule,
    MenuModule,
    GroupModule,
    LayoutModule
  ]
})
export class InsightModule {}
