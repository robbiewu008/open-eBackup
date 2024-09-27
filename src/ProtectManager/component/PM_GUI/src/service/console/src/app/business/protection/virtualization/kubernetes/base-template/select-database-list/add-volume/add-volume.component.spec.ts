import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { AddVolumeComponent } from './add-volume.component';

describe('AddVolumeComponent', () => {
  let component: AddVolumeComponent;
  let fixture: ComponentFixture<AddVolumeComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [AddVolumeComponent]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(AddVolumeComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
