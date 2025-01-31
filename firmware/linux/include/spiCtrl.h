/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2025
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

#define SPI_BUFFER_SIZE TRANSFER_BUFFER_SIZE

/*!
 *
 * SPI0
 * ============================================
 * BBB P9_17 :: PULPLE   :: SPI0_CS0
 * BBB P9_18 :: BLUE     :: SPI0_D1
 * BBB P9_21 :: BROWN    :: SPI0_D0
 * BBB P9_22 :: BLACK    :: SPI0_SCLK
 *
 * SPI1
 * ============================================
 * BBB P9_28 :: ORANGE   :: SPI1_CS0
 * BBB P9_30 :: YELOW    :: SPI1_D1
 * BBB P9_29 :: BLUE     :: SPI1_D0
 * BBB P9_31 :: GREEN    :: SPI1_SCLK
 *
 */

typedef enum
{
    BUS_SPI0,
    BUS_SPI1,
    BUS_AMOUNT
}spiBusType;

typedef enum
{
    SPI_PRIMARY,
    SPI_SECONDARY,
    SPI_AMOUNT
}spiDeviceType;

typedef struct
{
    struct spi_message spiMessage;
    struct spi_transfer spiTransfer;
    dma_addr_t tx_dma;
    dma_addr_t rx_dma;
}spiDmaData;

typedef struct
{
    struct spi_device *spiDevice;
    volatile uint8_t spiTx[SPI_BUFFER_SIZE];
    volatile uint8_t spiRx[SPI_BUFFER_SIZE];
    uint8_t spiLength;
    spiDmaData Dma;
}spiDeviceData;

/* SPI */ int spiInit(void);
/* CONFIG */ void enableDMAServer(void);
/* FPGA */ void transferFpgaInput(struct work_struct *work);
/* FPGA */ void transferFpgaOutput(struct work_struct *work);
/* KILL */ void killApplication(struct work_struct *work);
/* SPI */ void spiDestroy(void);

#endif // SPI_FPGA_H
