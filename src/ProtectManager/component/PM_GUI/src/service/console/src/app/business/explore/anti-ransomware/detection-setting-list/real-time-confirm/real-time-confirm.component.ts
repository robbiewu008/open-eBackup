import { Component, OnInit } from '@angular/core';
import { I18NService } from 'app/shared/services/i18n.service';

@Component({
  selector: 'aui-real-time-confirm',
  templateUrl: './real-time-confirm.component.html',
  styleUrls: ['./real-time-confirm.component.css']
})
export class RealTimeConfirmComponent implements OnInit {
  isChecked: boolean = true;
  type;

  constructor(public i18n: I18NService) {}

  ngOnInit() {}
}
