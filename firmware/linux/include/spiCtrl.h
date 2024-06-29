/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#ifndef SPI_FPGA_H
#define SPI_FPGA_H

#include <linux/spi/spi.h>
#include "types.h"

//////////////////////
//                  //
//                  //
//                  //
//   [SPI] Comms    //
//                  //
//                  //
//                  //
//////////////////////

/* SPI */ int spiInit(void);
/* FPGA */ void transferFpgaInput(struct work_struct *work);
/* FPGA */ void transferFpgaOutput(struct work_struct *work);
/* KILL */ void killApplication(struct work_struct *work);
/* SPI */ void spiDestroy(void);

#endif // SPI_FPGA_H