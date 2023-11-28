#ifndef SSI_H
#define SSI_H

#include "lwip/apps/httpd.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"

// SSI tags - tag length limited to 8 bytes by default
const char * ssi_tags[] = {"bar"};

/** \fn u16_t ssi_handler(int iIndex, char *pcInsert, int iInsertLen)
    \brief Server Side Include (SSI) handler for populating dynamic content into HTML pages.
    \param iIndex Index specifying the type of dynamic content to insert.
    \param pcInsert A character buffer where the SSI handler should insert the content.
    \param iInsertLen The length of the character buffer.
    \return The number of characters inserted into pcInsert.
*/

u16_t ssi_handler(int iIndex, char *pcInsert, int iInsertLen) {
  size_t printed;
  switch (iIndex) {
  case 0: // bar
    {
    printed = snprintf(pcInsert, iInsertLen, "Barcode here");
    }
    break;

  default:
    printed = 0;
    break;
  }

  return (u16_t)printed;
}

// Initialise the SSI handler
void ssi_init() {
  http_set_ssi_handler(ssi_handler, ssi_tags, LWIP_ARRAYSIZE(ssi_tags));
}

#endif