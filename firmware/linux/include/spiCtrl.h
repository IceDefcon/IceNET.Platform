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
/* FPGA */ void interruptFromFpga(struct work_struct *work);
/* KERNEL */ void transferFromCharDevice(struct work_struct *work);
/* KERNEL */ void feedbackTransferFromFPGA(struct work_struct *work);
/* SPI */ void spiDestroy(void);

/* GET TRANSFER RX DATA */ DataTransfer* get_fpgaFeedbackTransfer(void);

#endif // SPI_FPGA_H