import { Component, OnInit, ViewChild } from '@angular/core';
import { AvailabiltyGroupComponent } from './availabilty-group/availabilty-group.component';
import { DatabaseComponent } from './database/database.component';
import { EmailComponent } from './email/email.component';

@Component({
  selector: 'aui-exchange',
  templateUrl: './exchange.component.html',
  styleUrls: ['./exchange.component.less']
})
export class ExchangeComponent implements OnInit {
  activeIndex = 'group';
  constructor() {}

  @ViewChild(AvailabiltyGroupComponent, { static: false })
  AvailabiltyGroupComponent: AvailabiltyGroupComponent;
  @ViewChild(DatabaseComponent, { static: false })
  DatabaseComponent: DatabaseComponent;
  @ViewChild(EmailComponent, { static: false })
  EmailComponent: EmailComponent;

  onChange() {
    if (this.activeIndex === 'group') {
      this.AvailabiltyGroupComponent.ngOnInit();
    } else if (this.activeIndex === 'database') {
      this.DatabaseComponent.ngOnInit();
    } else {
      this.EmailComponent.ngOnInit();
    }
  }

  ngOnInit(): void {}
}
