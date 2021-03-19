import { TestBed, inject } from '@angular/core/testing';

import { SecureEtsDataService } from './secure-ets-data.service';

describe('SecureEtsDataServiceService', () => {
  beforeEach(() => {
    TestBed.configureTestingModule({
      providers: [SecureEtsDataService]
    });
  });

  it('should be created', inject([SecureEtsDataService], (service: SecureEtsDataService) => {
    expect(service).toBeTruthy();
  }));
});
