#ifndef _CALOTRANS_HH
#define _CALOTRANS_HH
#include <stdint.h>
struct CaloTransHeader {
  uint32_t detector_id;
  uint32_t source_id;
  uint32_t event_id;
  uint64_t bx_id;
};
struct CaloTransHit
{
  uint64_t asic_bcid;
  double time;
  double energy;
  float x,y,z;
  uint32_t spill;
  uint16_t adc_energy;
  uint8_t dif_id;
  uint8_t asic_id;
  uint8_t chan_id;
  uint8_t sca;
  uint8_t map_x,map_y,map_z;
};
#endif
