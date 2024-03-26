/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#include "spiFpga.h"

//////////////////////
//                  //
//                  //
//                  //
//   [SPI] Comms    //
//                  //
//                  //
//                  //
//////////////////////////////////////////
//                                      //
//                                      //
// BBB P9_17 :: PULPLE   :: SPI0_CS0    //
// BBB P9_18 :: BLUE     :: SPI0_D1     //
// BBB P9_21 :: BROWN    :: SPI0_D0     //
// BBB P9_22 :: BLACK    :: SPI0_SCLK   //
//                                      //
// BBB P9_28 :: YELOW    :: SPI1_CS0    //
// BBB P9_30 :: GREEN    :: SPI1_D1     //
// BBB P9_29 :: RED      :: SPI1_D0     //
// BBB P9_31 :: ORANGE   :: SPI1_SCLK   //
//                                      //
//                                      //
//////////////////////////////////////////


// static struct spi_device *spi_dev;

// /* SPI DEVICE */
// /* BASE */ struct spi_device *spi_dev;
// /* GET */ struct spi_device* get_spi_dev(void)
// {
//     return spi_dev;
// }
// /* SET */ void set_spi_dev(struct spi_device *spi)
// {
//     spi_dev = spi;
// }