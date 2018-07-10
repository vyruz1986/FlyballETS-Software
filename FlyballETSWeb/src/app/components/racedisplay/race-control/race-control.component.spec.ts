import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { RaceControlComponent } from './race-control.component';

describe('RaceControlComponent', () => {
  let component: RaceControlComponent;
  let fixture: ComponentFixture<RaceControlComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ RaceControlComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(RaceControlComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
