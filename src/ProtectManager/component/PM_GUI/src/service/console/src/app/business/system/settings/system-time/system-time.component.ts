import { AfterViewInit, Component, ElementRef, OnInit } from '@angular/core';
import { FormGroup } from '@angular/forms';
import { DomSanitizer, SafeHtml } from '@angular/platform-browser';
import {
  DataMap,
  I18NService,
  LANGUAGE,
  LocalStorageApiService
} from 'app/shared';
import { toString } from 'lodash';

@Component({
  selector: 'aui-system-time',
  templateUrl: './system-time.component.html',
  styleUrls: ['./system-time.component.less']
})
export class SystemTimeComponent implements OnInit, AfterViewInit {
  desc: SafeHtml;
  isModify = false;
  formGroup: FormGroup;
  isCyberengine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;
  tips = this.isCyberengine
    ? this.i18n.get('system_device_time_desc_cyber_label')
    : this.i18n.get('system_device_time_desc_label');
  constructor(
    private i18n: I18NService,
    private elementRef: ElementRef,
    private sanitizer: DomSanitizer,
    private localStorageApiService: LocalStorageApiService
  ) {}

  ngOnInit() {
    this.desc = this.sanitizer.bypassSecurityTrustHtml(this.tips);
  }

  ngAfterViewInit() {
    const openDeviceDom = this.elementRef.nativeElement.querySelector(
      '#open-device'
    );
    if (!openDeviceDom) {
      return;
    }
    openDeviceDom.addEventListener('click', this.getStorageToken.bind(this));
  }

  onChange() {
    this.ngAfterViewInit();
  }

  getStorageToken() {
    this.localStorageApiService.getStorageTokenUsingGET({}).subscribe(res => {
      const language =
        this.i18n.language.toLowerCase() === LANGUAGE.CN ? 'zh' : 'en';
      const url = `https://${encodeURI(res.ip)}:${encodeURI(
        toString(res.port)
      )}/deviceManager/devicemanager/feature/login/crossDomainLogin.html?passphrase=${encodeURIComponent(
        res.token
      )}&language=${encodeURIComponent(language)}`;
      window.open(url, '_blank');
    });
  }

  saveTime() {}

  cancelTime() {}
}
