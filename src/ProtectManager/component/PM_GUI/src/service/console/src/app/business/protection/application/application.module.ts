import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { ApplicationRoutingModule } from './application-routing.module';
import { ApplicationComponent } from './application.component';
import { SubAppCardModule } from 'app/shared/components/sub-app-card/sub-app-card.module';

@NgModule({
  declarations: [ApplicationComponent],
  imports: [CommonModule, ApplicationRoutingModule, SubAppCardModule],
  exports: [ApplicationComponent]
})
export class ApplicationModule {}
