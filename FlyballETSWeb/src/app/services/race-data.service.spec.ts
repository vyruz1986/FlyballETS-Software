import { TestBed, inject } from '@angular/core/testing';

import { RaceDataService } from './race-data.service';

describe('RaceDataService', () => {
  beforeEach(() => {
    TestBed.configureTestingModule({
      providers: [RaceDataService]
    });
  });

  it('should be created', inject([RaceDataService], (service: RaceDataService) => {
    expect(service).toBeTruthy();
  }));
});
