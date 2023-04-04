

#include "lwip/opt.h"

#if LWIP_IPV4 && LWIP_DHCP && LWIP_NETCONN

#include "lwip/dhcp.h"
#include "lwip/ip_addr.h"
#include "lwip/netifapi.h"
#include "lwip/prot/dhcp.h"
#include "lwip/tcpip.h"
#include "lwip/sys.h"
#include "enet_ethernetif.h"

#include "board.h"

#include "fsl_device_registers.h"
#include "pin_mux.h"
#include "clock_config.h"

#include "fsl_dspi.h"
#include "fsl_dspi_freertos.h"
#include "fsl_uart_freertos.h"





///////////////////////////////////////////////////////////////////////////////////
// UART
///////////////////////////////////////////////////////////////////////////////////
// UART instance and clock
#define DEMO_UART            UART0
#define DEMO_UART_CLKSRC     UART0_CLK_SRC
#define DEMO_UART_CLK_FREQ   CLOCK_GetFreq(UART0_CLK_SRC)
#define DEMO_UART_RX_TX_IRQn UART0_RX_TX_IRQn
// Task priorities.
#define uart_task_PRIORITY (configMAX_PRIORITIES - 1)
//
// Prototypes
//
static void uart_task(void *pvParameters);
//
// Variables
//
char *to_send               = "FreeRTOS UART driver example!\r\n";
char *send_ring_overrun     = "\r\nRing buffer overrun!\r\n";
char *send_hardware_overrun = "\r\nHardware buffer overrun!\r\n";
uint8_t background_buffer[32];
uint8_t recv_buffer[4];

uart_rtos_handle_t handle;
struct _uart_handle t_handle;

uart_rtos_config_t uart_config = {
    .baudrate    = 115200,
    .parity      = kUART_ParityDisabled,
    .stopbits    = kUART_OneStopBit,
    .buffer      = background_buffer,
    .buffer_size = sizeof(background_buffer),
};

//
// RTOS :: UART Task
//
static void uart_task(void *pvParameters)
{
    int error;
    size_t n = 0;

    uart_config.srcclk = DEMO_UART_CLK_FREQ;
    uart_config.base   = DEMO_UART;

    if (kStatus_Success != UART_RTOS_Init(&handle, &t_handle, &uart_config))
    {
        vTaskSuspend(NULL);
    }

    /* Send introduction message. */
    if (kStatus_Success != UART_RTOS_Send(&handle, (uint8_t *)to_send, strlen(to_send)))
    {
        vTaskSuspend(NULL);
    }

    /* Receive user input and send it back to terminal. */
    do
    {
        error = UART_RTOS_Receive(&handle, recv_buffer, sizeof(recv_buffer), &n);
        if (error == kStatus_UART_RxHardwareOverrun)
        {
            /* Notify about hardware buffer overrun */
            if (kStatus_Success !=
                UART_RTOS_Send(&handle, (uint8_t *)send_hardware_overrun, strlen(send_hardware_overrun)))
            {
                vTaskSuspend(NULL);
            }
        }
        if (error == kStatus_UART_RxRingBufferOverrun)
        {
            /* Notify about ring buffer overrun */
            if (kStatus_Success != UART_RTOS_Send(&handle, (uint8_t *)send_ring_overrun, strlen(send_ring_overrun)))
            {
                vTaskSuspend(NULL);
            }
        }
        if (n > 0)
        {
            /* send back the received data */
            UART_RTOS_Send(&handle, recv_buffer, n);
        }
    } while (kStatus_Success == error);

    UART_RTOS_Deinit(&handle);
    vTaskSuspend(NULL);
}

///////////////////////////////////////////////////////////////////////////////////
// SPI
///////////////////////////////////////////////////////////////////////////////////
//
// Module Configuration Register for SPI0 and SPI1
//
#define EXAMPLE_DSPI_MASTER_BASE    (0x4002C000u)   // (SPI0_MCR)
#define EXAMPLE_DSPI_SLAVE_BASE     (0x4002D000u)   // (SPI1_MCR)
#define EXAMPLE_DSPI_MASTER_IRQN    (26)            // Interrupt 
#define EXAMPLE_DSPI_SLAVE_IRQN     (27)            // Interrupt

#define DSPI_MASTER_CLK_SRC         (kCLOCK_BusClk)
#define DSPI_MASTER_CLK_FREQ        CLOCK_GetFreq((kCLOCK_BusClk))

#define EXAMPLE_DSPI_MASTER_BASEADDR ((SPI_Type *)EXAMPLE_DSPI_MASTER_BASE)
#define EXAMPLE_DSPI_SLAVE_BASEADDR  ((SPI_Type *)EXAMPLE_DSPI_SLAVE_BASE)

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

    DSPI_SlaveInit(EXAMPLE_DSPI_SLAVE_BASEADDR, &slaveConfig);

    /*Set up slave first */
    DSPI_SlaveTransferCreateHandle(EXAMPLE_DSPI_SLAVE_BASEADDR, &g_s_handle, DSPI_SlaveUserCallback, &cb_msg);

    /*Set slave transfer ready to receive/send data*/
    slaveXfer.txData      = slaveSendBuffer;
    slaveXfer.rxData      = slaveReceiveBuffer;
    slaveXfer.dataSize    = TRANSFER_SIZE;
    slaveXfer.configFlags = kDSPI_SlaveCtar0;

    DSPI_SlaveTransferNonBlocking(EXAMPLE_DSPI_SLAVE_BASEADDR, &g_s_handle, &slaveXfer);

    if (xTaskCreate(master_task, "Master_task", configMINIMAL_STACK_SIZE + 100, NULL, master_task_PRIORITY, NULL) !=
        pdPASS)
    {
        PRINTF("Failed to create master task");
        vTaskSuspend(NULL);
    }

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
    status      = DSPI_RTOS_Init(&master_rtos_handle, EXAMPLE_DSPI_MASTER_BASEADDR, &masterConfig, sourceClock);

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

///////////////////////////////////////////////////////////////////////////////////
// GPIO ---> For LED status control
///////////////////////////////////////////////////////////////////////////////////
//                                          // K64 Sub-Family Reference Manual
//                                          // ==================================
//                                          //                      (page number)
#define SIM_SCGC5 (*(int *)0x40048038u)     // Clock gate 5                 (314)
#define SIM_SCGC5_PORTB 10                  // Open gate PORTB              (314)

#define PORTB_PCR21 (*(int *)0x4004A054u)   // Pin Control Register         (277)
#define PORTB_PCR21_MUX 8                   // Mux "001"                    (282)
#define PORTB_PCR22 (*(int *)0x4004A058u)   // Pin Control Register         (277)
#define PORTB_PCR22_MUX 8                   // Mux "001"                    (282)

#define GPIOB_PDDR (*(int *)0x400FF054u)    // Port Data Direction Register (1760)
#define GPIOB_PDOR (*(int *)0x400FF040u)    // Port Data Output Register    (1759)
#define PIN_21_N 21                         // PTB21 --> Blue LED           (1761)
#define PIN_22_N 22                         // PTB22 --> Red  LED           (1761)

#define SIM_SCGC5 (*(int *)0x40048038u)     // Clock gate 5                 (314)
#define SIM_SCGC5_PORTD 12                  // Open gate PORTD              (314)

#define PORTD_PCR1 (*(int *)0x4004C004u)    // Pin Control Register         (277)
#define PORTD_PCR1_MUX 8                    // Mux "001"                    (282)

#define GPIOD_PDDR (*(int *)0x400FF0D4u)    // Port Data Direction Register (1760)
#define GPIOD_PDOR (*(int *)0x400FF0C0u)    // Port Data Output Register    (1759)
#define PIN_SPI_N 1                         // PTB21 --> Blue LED           (1761)

void init_gpio()
{
    // Enable clock for PORTB
    SIM_SCGC5 |= 1 << SIM_SCGC5_PORTB;  // For LED

    // Configure pins PCR21 and PCR22 as GPIO
    PORTB_PCR21 |= 1 << PORTB_PCR21_MUX;
    PORTB_PCR22 |= 1 << PORTB_PCR22_MUX;

    // Configure GPIO pins 21 and 22 as output.
    // It will have a default output value set
    // to 0, so LED will light (negative logic).
    GPIOB_PDDR |= 1 << PIN_21_N;
    GPIOB_PDDR |= 1 << PIN_22_N;

    // Set Blue pin 21 ---> OFF
    // Leave Red pin 22 in default ---> ON
    GPIOB_PDOR ^= 1 << PIN_21_N;
}

//
// This is for SPI to behave as toogling clock pin
//
void init_sclk_pin()
{
    /* Enable clocks. */
    SIM_SCGC5  |= 1 << SIM_SCGC5_PORTD;
    /* Configure pin 1 as GPIO. */
    PORTD_PCR1 |= 1 << PORTD_PCR1_MUX;
    /* Configure GPIO pin 21 as output. */
    GPIOD_PDDR |= 1 << PIN_SPI_N;
}

void blinky_task()
{
    // Toogle pin 22 ---> Red LED OFF
    GPIOB_PDOR ^= 1 << PIN_22_N;

    while(1) {
        GPIOB_PDOR ^= 1 << PIN_21_N;            // LED Pin   ---> Toggle with XOR
        GPIOD_PDOR ^= 1 << PIN_SPI_N;           // SPI CLock ---> Toggle with XOR
        vTaskDelay(500 / portTICK_PERIOD_MS);   // 500/10
    }
}

///////////////////////////////////////////////////////////////////////////////////
// Network Stack ---> LWIP
///////////////////////////////////////////////////////////////////////////////////
//
// Definitions
//
// MAC address configuration
//
#define configMAC_ADDR                  \
{                                       \
    0x02, 0x12, 0x13, 0x10, 0x15, 0x11  \
}

// Address of PHY interface
#define EXAMPLE_PHY_ADDRESS BOARD_ENET0_PHY_ADDRESS

// System clock name
#define EXAMPLE_CLOCK_NAME kCLOCK_CoreSysClk

// GPIO pin configuration
#define BOARD_LED_GPIO BOARD_LED_RED_GPIO
#define BOARD_LED_GPIO_PIN BOARD_LED_RED_GPIO_PIN
#define BOARD_SW_GPIO BOARD_SW3_GPIO
#define BOARD_SW_GPIO_PIN BOARD_SW3_GPIO_PIN
#define BOARD_SW_PORT BOARD_SW3_PORT
#define BOARD_SW_IRQ BOARD_SW3_IRQ
#define BOARD_SW_IRQ_HANDLER BOARD_SW3_IRQ_HANDLER

// brief Stack size of the thread which prints DHCP info
#define PRINT_THREAD_STACKSIZE 512

// brief Priority of the thread which prints DHCP info
#define PRINT_THREAD_PRIO DEFAULT_THREAD_PRIO

//
// brief Prints DHCP status of the interface when it has changed from last status.
// 
// param arg pointer to network interface structure
//
static void print_dhcp_state(void *arg)
{
    struct netif *netif = (struct netif *)arg;
    struct dhcp *dhcp;
    u8_t dhcp_last_state = DHCP_STATE_OFF;

    while (netif_is_up(netif))
    {
        dhcp = netif_dhcp_data(netif);

        if (dhcp == NULL)
        {
            dhcp_last_state = DHCP_STATE_OFF;
        }
        else if (dhcp_last_state != dhcp->state)
        {
            dhcp_last_state = dhcp->state;

            PRINTF(" DHCP state       : ");
            switch (dhcp_last_state)
            {
                case DHCP_STATE_OFF:
                    PRINTF("OFF");
                    break;
                case DHCP_STATE_REQUESTING:
                    PRINTF("REQUESTING");
                    break;
                case DHCP_STATE_INIT:
                    PRINTF("INIT");
                    break;
                case DHCP_STATE_REBOOTING:
                    PRINTF("REBOOTING");
                    break;
                case DHCP_STATE_REBINDING:
                    PRINTF("REBINDING");
                    break;
                case DHCP_STATE_RENEWING:
                    PRINTF("RENEWING");
                    break;
                case DHCP_STATE_SELECTING:
                    PRINTF("SELECTING");
                    break;
                case DHCP_STATE_INFORMING:
                    PRINTF("INFORMING");
                    break;
                case DHCP_STATE_CHECKING:
                    PRINTF("CHECKING");
                    break;
                case DHCP_STATE_BOUND:
                    PRINTF("BOUND");
                    break;
                case DHCP_STATE_BACKING_OFF:
                    PRINTF("BACKING_OFF");
                    break;
                default:
                    PRINTF("%u", dhcp_last_state);
                    assert(0);
                    break;
            }
            PRINTF("\r\n");

            if (dhcp_last_state == DHCP_STATE_BOUND)
            {
                PRINTF("\r\n IPv4 Address     : %s\r\n", ipaddr_ntoa(&netif->ip_addr));
                PRINTF(" IPv4 Subnet mask : %s\r\n", ipaddr_ntoa(&netif->netmask));
                PRINTF(" IPv4 Gateway     : %s\r\n\r\n", ipaddr_ntoa(&netif->gw));
            }
        }

        sys_msleep(20U);
    }

    vTaskDelete(NULL);
}

//
// DHCP ---> Dynamic Host Configuration Protocol
//
void dhcp_init()
{
    static struct netif fsl_netif0;
#if defined(FSL_FEATURE_SOC_LPC_ENET_COUNT) && (FSL_FEATURE_SOC_LPC_ENET_COUNT > 0)
    static mem_range_t non_dma_memory[] = NON_DMA_MEMORY_ARRAY;
#endif /* FSL_FEATURE_SOC_LPC_ENET_COUNT */
    ip4_addr_t fsl_netif0_ipaddr, fsl_netif0_netmask, fsl_netif0_gw;
    ethernetif_config_t fsl_enet_config0 = {
        .phyAddress = EXAMPLE_PHY_ADDRESS,
        .clockName  = EXAMPLE_CLOCK_NAME,
        .macAddress = configMAC_ADDR,
#if defined(FSL_FEATURE_SOC_LPC_ENET_COUNT) && (FSL_FEATURE_SOC_LPC_ENET_COUNT > 0)
        .non_dma_memory = non_dma_memory,
#endif /* FSL_FEATURE_SOC_LPC_ENET_COUNT */
    };

    IP4_ADDR(&fsl_netif0_ipaddr, 10U, 0U, 0U, 2U);
    IP4_ADDR(&fsl_netif0_netmask, 255U, 255U, 255U, 0U);
    IP4_ADDR(&fsl_netif0_gw, 10U, 0U, 0U, 1U);

    tcpip_init(NULL, NULL);

    netifapi_netif_add(&fsl_netif0, &fsl_netif0_ipaddr, &fsl_netif0_netmask, &fsl_netif0_gw, &fsl_enet_config0,
                       ethernetif0_init, tcpip_input);
    netifapi_netif_set_default(&fsl_netif0);
    netifapi_netif_set_up(&fsl_netif0);

    netifapi_dhcp_start(&fsl_netif0);

    PRINTF("\r\n************************************************\r\n");
    PRINTF(" DHCP example\r\n");
    PRINTF("************************************************\r\n");

    if (sys_thread_new("print_dhcp", print_dhcp_state, &fsl_netif0, PRINT_THREAD_STACKSIZE, PRINT_THREAD_PRIO) == NULL)
    {
        LWIP_ASSERT("stack_init(): Task creation failed.", 0);
    }
}

//
// Mains
//
int main(void)
{
    //
    // GPIO :: LED STATUS
    //
    init_gpio();
    init_sclk_pin();
    xTaskCreate(blinky_task, "Blinky", 100, NULL, 1, NULL); 

    //
    // Board init :: For SPI and LWIP
    //
    SYSMPU_Type *base = SYSMPU;
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    
    //
    // Disable SYSMPU :: Full access to memory :: Turn off MPU :: Memory Protection Unit 
    //
    base->CESR &= ~SYSMPU_CESR_VLD_MASK;

    /* Set interrupt priorities */
    NVIC_SetPriority(EXAMPLE_DSPI_SLAVE_IRQN, 2);
    NVIC_SetPriority(EXAMPLE_DSPI_MASTER_IRQN, 3);
    NVIC_SetPriority(DEMO_UART_RX_TX_IRQn, 5);

    //
    // UART
    //
    xTaskCreate(uart_task, "Uart_task", configMINIMAL_STACK_SIZE + 100, NULL, uart_task_PRIORITY, NULL);

    //
    // DSPI :: RTOS
    //
    xTaskCreate(slave_task, "Slave_task", configMINIMAL_STACK_SIZE + 100, NULL, slave_task_PRIORITY, NULL);

    //
    // LWIP :: RTOS
    //
    dhcp_init();

    vTaskStartScheduler();

    /* Will not get here unless a task calls vTaskEndScheduler ()*/
    return 0;
}
#endif

