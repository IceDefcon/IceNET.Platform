
#include "common.h"
#include "lwip.h"
#include "gpio.h"


#include "fsl_dspi.h"
#include "fsl_dspi_freertos.h"

///////////////////////////////////////////////////////////////////////////////////
// SPI
///////////////////////////////////////////////////////////////////////////////////
//
// Module Configuration Register for SPI0 and SPI1
//
#define DSPI_MASTER_BASE    (0x4002C000u)   // (SPI0_MCR)
#define DSPI_SLAVE_BASE     (0x4002D000u)   // (SPI1_MCR)
#define DSPI_MASTER_IRQN    (26)            // Interrupt 
#define DSPI_SLAVE_IRQN     (27)            // Interrupt

#define DSPI_MASTER_CLK_SRC         (kCLOCK_BusClk)
#define DSPI_MASTER_CLK_FREQ        CLOCK_GetFreq((kCLOCK_BusClk))

#define DSPI_MASTER_BASEADDR ((SPI_Type *)DSPI_MASTER_BASE)
#define DSPI_SLAVE_BASEADDR  ((SPI_Type *)DSPI_SLAVE_BASE)

#define TRANSFER_SIZE     (256)     /*! Transfer size */
#define TRANSFER_BAUDRATE (500000U) /*! Transfer baudrate - 500k */

//
// Variables
//
uint8_t masterReceiveBuffer[TRANSFER_SIZE] = {0};
uint8_t masterSendBuffer[TRANSFER_SIZE]    = {0};
uint8_t slaveReceiveBuffer[TRANSFER_SIZE]  = {0};
uint8_t slaveSendBuffer[TRANSFER_SIZE]     = {0};

dspi_slave_handle_t g_s_handle;
SemaphoreHandle_t dspi_sem;

//
// Definitions
//
/* Task priorities. */
#define slave_task_PRIORITY  (configMAX_PRIORITIES - 2)
#define master_task_PRIORITY (configMAX_PRIORITIES - 1)
/* Interrupt priorities. */
#define DSPI_NVIC_PRIO 2

//
// Prototypes
//
static void slave_task(void *pvParameters);
static void master_task(void *pvParameters);

//
// SPI Subroutines :: RTOS
//
typedef struct _callback_message_t
{
    status_t async_status;
    SemaphoreHandle_t sem;
} callback_message_t;

void DSPI_SlaveUserCallback(SPI_Type *base, dspi_slave_handle_t *handle, status_t status, void *userData)
{
    callback_message_t *cb_msg = (callback_message_t *)userData;
    BaseType_t reschedule      = 0;

    cb_msg->async_status = status;
    xSemaphoreGiveFromISR(cb_msg->sem, &reschedule);
    portYIELD_FROM_ISR(reschedule);
}

//
// SPI Slave task :: RTOS
//
static void slave_task(void *pvParameters)
{
    dspi_slave_config_t slaveConfig;
    dspi_transfer_t slaveXfer;

    uint32_t errorCount;
    uint32_t i;
    callback_message_t cb_msg;

    cb_msg.sem = xSemaphoreCreateBinary();
    dspi_sem   = cb_msg.sem;
    if (cb_msg.sem == NULL)
    {
        PRINTF("DSPI slave: Error creating semaphore\r\n");
        vTaskSuspend(NULL);
    }
    /*Set up the transfer data*/
    for (i = 0; i < TRANSFER_SIZE; i++)
    {
        masterSendBuffer[i]    = i % 256;
        masterReceiveBuffer[i] = 0;

        slaveSendBuffer[i]    = ~masterSendBuffer[i];
        slaveReceiveBuffer[i] = 0;
    }
    /*Slave config*/
    slaveConfig.whichCtar                  = kDSPI_Ctar0;
    slaveConfig.ctarConfig.bitsPerFrame    = 8;
    slaveConfig.ctarConfig.cpol            = kDSPI_ClockPolarityActiveHigh;
    slaveConfig.ctarConfig.cpha            = kDSPI_ClockPhaseFirstEdge;
    slaveConfig.enableContinuousSCK        = false;
    slaveConfig.enableRxFifoOverWrite      = false;
    slaveConfig.enableModifiedTimingFormat = false;
    slaveConfig.samplePoint                = kDSPI_SckToSin0Clock;

    DSPI_SlaveInit(DSPI_SLAVE_BASEADDR, &slaveConfig);

    /*Set up slave first */
    DSPI_SlaveTransferCreateHandle(DSPI_SLAVE_BASEADDR, &g_s_handle, DSPI_SlaveUserCallback, &cb_msg);

    /*Set slave transfer ready to receive/send data*/
    slaveXfer.txData      = slaveSendBuffer;
    slaveXfer.rxData      = slaveReceiveBuffer;
    slaveXfer.dataSize    = TRANSFER_SIZE;
    slaveXfer.configFlags = kDSPI_SlaveCtar0;

    DSPI_SlaveTransferNonBlocking(DSPI_SLAVE_BASEADDR, &g_s_handle, &slaveXfer);

    if (xTaskCreate(master_task, "Master_task", configMINIMAL_STACK_SIZE + 100, NULL, master_task_PRIORITY, NULL) !=
        pdPASS)
    {
        PRINTF("Failed to create master task");
        vTaskSuspend(NULL);
    }
    PRINTF("master TASK created \r\n\r\n");

    /* Wait for transfer to finish */
    xSemaphoreTake(cb_msg.sem, portMAX_DELAY);

    if (cb_msg.async_status == kStatus_Success)
    {
        PRINTF("DSPI slave transfer completed successfully. \r\n\r\n");
    }
    else
    {
        PRINTF("DSPI slave transfer completed with error. \r\n\r\n");
    }

    errorCount = 0;
    for (i = 0; i < TRANSFER_SIZE; i++)
    {
        if (slaveSendBuffer[i] != masterReceiveBuffer[i])
        {
            errorCount++;
        }

        if (masterSendBuffer[i] != slaveReceiveBuffer[i])
        {
            errorCount++;
        }

        if (masterSendBuffer[i] != slaveReceiveBuffer[i])
        {
            errorCount++;
        }

        if (slaveSendBuffer[i] != masterReceiveBuffer[i])
        {
            errorCount++;
        }
    }
    if (errorCount == 0)
    {
        PRINTF("DSPI transfer all data matched! \r\n");
    }
    else
    {
        PRINTF("Error occurred in DSPI transfer ! \r\n");
    }

    vTaskSuspend(NULL);
}

//
// SPI Master task :: RTOS
//
static void master_task(void *pvParameters)
{
    dspi_transfer_t masterXfer;
    dspi_rtos_handle_t master_rtos_handle;
    dspi_master_config_t masterConfig;
    uint32_t sourceClock;
    status_t status;

    // /*Master config*/
    masterConfig.whichCtar                                = kDSPI_Ctar0;
    masterConfig.ctarConfig.baudRate                      = TRANSFER_BAUDRATE;
    masterConfig.ctarConfig.bitsPerFrame                  = 8;
    masterConfig.ctarConfig.cpol                          = kDSPI_ClockPolarityActiveHigh;
    masterConfig.ctarConfig.cpha                          = kDSPI_ClockPhaseFirstEdge;
    masterConfig.ctarConfig.direction                     = kDSPI_MsbFirst;
    masterConfig.ctarConfig.pcsToSckDelayInNanoSec        = 2000;
    masterConfig.ctarConfig.lastSckToPcsDelayInNanoSec    = 2000;
    masterConfig.ctarConfig.betweenTransferDelayInNanoSec = 1000;

    masterConfig.whichPcs           = kDSPI_Pcs0;
    masterConfig.pcsActiveHighOrLow = kDSPI_PcsActiveLow;

    masterConfig.enableContinuousSCK        = false;
    masterConfig.enableRxFifoOverWrite      = false;
    masterConfig.enableModifiedTimingFormat = false;
    masterConfig.samplePoint                = kDSPI_SckToSin0Clock;

    sourceClock = DSPI_MASTER_CLK_FREQ;
    status      = DSPI_RTOS_Init(&master_rtos_handle, DSPI_MASTER_BASEADDR, &masterConfig, sourceClock);

    if (status != kStatus_Success)
    {
        PRINTF("DSPI master: error during initialization. \r\n");
        vTaskSuspend(NULL);
    }
    /*Start master transfer*/
    masterXfer.txData      = masterSendBuffer;
    masterXfer.rxData      = masterReceiveBuffer;
    masterXfer.dataSize    = TRANSFER_SIZE;
    masterXfer.configFlags = kDSPI_MasterCtar0 | kDSPI_MasterPcs0 | kDSPI_MasterPcsContinuous;

    status = DSPI_RTOS_Transfer(&master_rtos_handle, &masterXfer);

    if (status == kStatus_Success)
    {
        xSemaphoreGive(dspi_sem);
        PRINTF("DSPI master transfer completed successfully. \r\n\r\n");
    }
    else
    {
        PRINTF("DSPI master transfer completed with error. \r\n\r\n");
    }

    vTaskSuspend(NULL);
}


//////////////////////////////////////
// Main
//////////////////////////////////////
int main(void)
{

    //
    // Board init :: SPI , LWIP and UART0 Debug Console
    //
    SYSMPU_Type *base = SYSMPU;
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    //
    // GPIO :: LED STATUS
    //
    gpio_init();
    xTaskCreate(Blinky_task, "Blinky", 100, NULL, 1, NULL); 
    
    //
    // Disable SYSMPU :: Full access to memory :: Turn off MPU :: Memory Protection Unit 
    //
    base->CESR &= ~SYSMPU_CESR_VLD_MASK;

    //
    // LWIP :: RTOS
    //
    dhcp_init();

    //
    // DSPI :: Set interrupt priorities
    //
    NVIC_SetPriority(DSPI_SLAVE_IRQN, 2);
    NVIC_SetPriority(DSPI_MASTER_IRQN, 3);

    //
    // DSPI :: TASK
    //
    xTaskCreate(slave_task, "Slave_task", configMINIMAL_STACK_SIZE + 100, NULL, slave_task_PRIORITY, NULL);
    PRINTF("slave TASK created \r\n\r\n");

    vTaskStartScheduler();

    /* Will not get here unless a task calls vTaskEndScheduler ()*/
    return 0;
}


