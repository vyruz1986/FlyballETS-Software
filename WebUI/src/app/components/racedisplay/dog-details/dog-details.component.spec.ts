import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { DogDetailsComponent } from './dog-details.component';

describe('DogDetailsComponent', () => {
  let component: DogDetailsComponent;
  let fixture: ComponentFixture<DogDetailsComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ DogDetailsComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(DogDetailsComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
