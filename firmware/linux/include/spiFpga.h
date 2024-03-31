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

void spi_kernel_execute(struct work_struct *work);
void spi_fpga_command(struct work_struct *work);
int spiInit(void);
int spiDestroy(void);

#endif // SPI_FPGA_H