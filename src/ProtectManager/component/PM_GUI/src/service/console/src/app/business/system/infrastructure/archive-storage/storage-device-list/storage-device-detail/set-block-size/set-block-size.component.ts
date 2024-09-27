import { Component, OnInit } from '@angular/core';
import {
  I18NService,
  BaseUtilService,
  TapeLibraryApiService,
  WarningMessageService
} from 'app/shared';
import { FormBuilder, FormGroup, FormControl } from '@angular/forms';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-set-block-size',
  templateUrl: './set-block-size.component.html',
  styleUrls: ['./set-block-size.component.less']
})
export class SetBlockSizeComponent implements OnInit {
  rowItem;
  tapeLibrarySn;
  formGroup: FormGroup;
  blockSizeLabel = this.i18n.get('system_block_size_label');
  node;

  rangeErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [0, 256])
  };

  constructor(
    private i18n: I18NService,
    private fb: FormBuilder,
    public baseUtilService: BaseUtilService,
    private warningMessageService: WarningMessageService,
    private tapeLibraryApiService: TapeLibraryApiService
  ) {}

  ngOnInit() {
    this.initForm();
  }

  initForm() {
    this.formGroup = this.fb.group({
      blockSize: new FormControl(
        this.rowItem.blockSize ? this.rowItem.blockSize + '' : '',
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(0, 256)
          ]
        }
      )
    });
  }

  onOk(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.warningMessageService.create({
        content: this.i18n.get('system_set_block_tip_label', [
          this.rowItem.name
        ]),
        onOK: () => {
          this.tapeLibraryApiService
            .modifyTapeDriveUsingPUT({
              tapeLibrarySn: this.tapeLibrarySn,
              tapeDriveModifyRequest: {
                blockSize: parseInt(this.formGroup.value.blockSize, 0),
                compressionStatus: this.rowItem.compressionStatus,
                status: this.rowItem.status
              },
              driverSn: this.rowItem.driverSn,
              memberEsn: this.node?.remoteEsn
            })
            .subscribe(
              res => {
                observer.next();
                observer.complete();
              },
              err => {
                observer.error(err);
                observer.complete();
              }
            );
        },
        onCancel: () => {
          observer.error('');
          observer.complete();
        },
        lvAfterClose: result => {
          if (result && result.trigger === 'close') {
            observer.error(null);
            observer.complete();
          }
        }
      });
    });
  }
}
