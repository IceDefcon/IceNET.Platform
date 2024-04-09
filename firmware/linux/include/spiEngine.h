/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#ifndef SPI_FPGA_H
#define SPI_FPGA_H

#include <linux/spi/spi.h>

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
/* KERNEL */ void interruptFromFpgaExecute(struct work_struct *work);
/* FPGA */ void signalFromCharDevice(struct work_struct *work);
/* SPI */ int spiDestroy(void);

#endif // SPI_FPGA_H