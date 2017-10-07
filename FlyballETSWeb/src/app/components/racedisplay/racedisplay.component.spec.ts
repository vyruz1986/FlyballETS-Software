import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { RacedisplayComponent } from './racedisplay.component';

describe('RacedisplayComponent', () => {
  let component: RacedisplayComponent;
  let fixture: ComponentFixture<RacedisplayComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ RacedisplayComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(RacedisplayComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should be created', () => {
    expect(component).toBeTruthy();
  });
});
