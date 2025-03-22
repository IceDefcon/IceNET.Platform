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

typedef enum
{
    DMA_IN,
    DMA_OUT,
    DMA_AMOUNT
}dmaControlType;

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
    uint8_t spiLength;
    spiDmaData Dma;
}spiDeviceData;

typedef struct
{
    uint8_t configCount;
    bool configDone;
}dmaCtrlType;

/* SPI */ int spiInit(void);
/* CONFIG */ void enableDMAConfig(void);
/* CONFIG */ void enableDMASingle(void);
/* FPGA */ void masterTransferPrimary(struct work_struct *work);
/* FPGA */ void masterTransferSecondary(struct work_struct *work);
/* SPI */ void spiDestroy(void);

/* RESET */ void resetLongDma(void);
/* GET */ bool isConfigDone(void);

#endif // SPI_FPGA_H

/**
 *
 * BMI160 Connected
 *
 * SPI0_MOSI :: H40-19 :: SDA :: OUT
 * SPI0_MISO :: H40-21 :: SAO :: IN
 * SPI0_SCLK :: H40-23 :: SCL :: OUT
 * SPI0_CS0  :: H40-24 :: CS  :: OUT
 *
 * SPI1_MOSI :: H40-37
 * SPI1_MISO :: H40-22
 * SPI1_SCLK :: H40-13
 * SPI1_CS0  :: H40-18
 *
 */

/**
 *          SPI 0
 *  _______________________
 * |     |     |     |     |
 * | SCL | SDA | CS  | SAO |
 * |     |     |     |     |
 * | H23 | H19 | H24 | H21 |
 * |_____|_____|_____|_____|
 *
 *
 *          SPI 1
 *  _______________________
 * |     |     |     |     |
 * | SCL | SDA | CS  | SAO |
 * |     |     |     |     |
 * | H13 | H37 | H18 | H22 |
 * |_____|_____|_____|_____|
 *
 *
 */


