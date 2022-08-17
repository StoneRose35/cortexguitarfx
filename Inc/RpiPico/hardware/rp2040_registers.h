#ifndef _RP2040_REGISTERS_
#define _RP2040_REGISTERS_

/**
 * @brief device-specific settings such as pin numbers and timings
 * 
 */

// ***************
// * pin numbers *
// ***************
#define RP2040_LED_PIN (25) // 13 on rp2040 feather, 25 on rpi pico boards

#define DS18B20_PIN 2

#define HEATER  6 // heating element controlled using pwm

#define I2C_SCL 2
#define I2C_SDA 1

#define I2S_WS_PIN 12
#define I2S_BCK_PIN 11
#define I2S_DATA_PIN 10
#define I2S_DATA_IN_PIN 9
#define I2S_MCLK_PIN 8
#define I2S_DEBUG_PIN 6

// GPIO number where the neopixel is attached
// Integrated Neopixel on RP2040 Feather: 16
// Integrated Neopixel on RP2040 Itsybitsy: 17
#ifdef ITSYBITSY
#define NEOPIXEL_PIN 17
#define NEOPIXEL_POWER_PIN 16
#else
#define NEOPIXEL_PIN 16
#endif

#define REMOTESWITCH_PIN 14 // 433 MHz radio-controller switch, reverse-engineered

// rotary encoder combined with a push switch
#define ENCODER_1 7
#define ENCODER_2 6
#define SWITCH 22

// backlight for lcd display
#define BACKLIGHT 8

// 1.8" Color TFT LCD display with MicroSD Card Breakout - ST7735R
// one spi sharing connection to a ST7735 display and  an sd card
#define CS_SDCARD 5
#define CS_DISPLAY 12
#define MISO 20
#define MOSI 19
#define SCK 18
#define DISPLAY_RESET 24
#define DISPLAY_CD 25
#define DISPLAY_BACKLIGHT 8 

// driver for ssd1306-based display (128*64 pixel oled display interfaced using spi)
#define SSD1306_CS_DISPLAY 21
#define SSD1306_SCK 18
#define SSD1306_MOSI 19
#define SSD1306_DISPLAY_CD 14
#define SSD1306_DISPLAY_RESET 15

// pin definitions for two uart ports
#define UART_USB_RX 1
#define UART_USB_TX 0
#define UART_BT_RX 9
#define UART_BT_TX 8


// ****************************************
// * other device-specific configurations *
// ****************************************
#define DS18B20_INSTR_MEM_OFFSET 10
#define DS18B20_CLKDIV 480 // 4 us clockdiv
#define DS18B20_CLKDIV_READ 120 // 1 us clock
#define DS18B20_RESET_CLKDIV 3600 // 30 us clockdiv

// 120MHz / 78.125 = 48kHz*2*16
#define I2S_CLKDIV_INT 40 //39
#define I2S_CLKDIV_FRAC 0 //16

#define I2S_CLKDIV_DBL_INT 10 //19
#define I2S_CLKDIV_DBL_FRAC 0//136

#define I2S_CLKDIV_MCLK_INT 5//9
#define I2S_CLKDIV_MCLK_FRAC 0 //196

#define REMOTESWITCH_CLKDIV 26640
#define REMOTESWITCH_INSTR_MEM_OFFSET 20

#define SCK_SDCARD_INIT 199 // SPI clock divider for SD-Card initialization
#define SCK_SDCARD_MEDIUM 5 // SPI clock divider for SD-Card operation
#define SCK_DISPLAY_SLOW 5 // resulting in 10 MHz spi clock for display

#define SSD1306_SCK_DISPLAY_SLOW 10 // resulting in 5 MHz spi clock for display

/**
 * @brief register definitions
 * 
 */
#define RESETS ((volatile uint32_t*)(RESETS_BASE + RESETS_RESET_OFFSET))
#define RESETS_DONE ((volatile uint32_t*)(RESETS_BASE + RESETS_RESET_DONE_OFFSET))

#define CLK_ADC_CTRL ((volatile uint32_t*)(CLOCKS_BASE + CLOCKS_CLK_ADC_CTRL_OFFSET))
#define CLK_ADC_DIV ((volatile uint32_t*)(CLOCKS_BASE + CLOCKS_CLK_ADC_DIV_OFFSET))

#define ADC_CS ((volatile uint32_t*)(ADC_BASE + ADC_CS_OFFSET))
#define ADC_DIV ((volatile uint32_t*)(ADC_BASE + ADC_DIV_OFFSET))
#define ADC_RESULT ((volatile uint32_t*)(ADC_BASE + ADC_RESULT_OFFSET))
#define ADC_FCS ((volatile uint32_t*)(ADC_BASE + ADC_FCS_OFFSET))
#define ADC_FIFO ((volatile uint32_t*)(ADC_BASE + ADC_FIFO_OFFSET))
#define ADC_INTE ((volatile uint32_t*)(ADC_BASE + ADC_INTE_OFFSET))

#define PADS_ADC0 ((volatile uint32_t*)(PADS_BANK0_BASE + PADS_BANK0_GPIO26_OFFSET))

#define GPIO_OE ((volatile uint32_t*)(SIO_BASE + SIO_GPIO_OE_OFFSET))
#define GPIO_OUT ((volatile uint32_t*)(SIO_BASE + SIO_GPIO_OUT_OFFSET))
#define GPIO_IN ((volatile uint32_t*)(SIO_BASE + SIO_GPIO_IN_OFFSET))

#define DMA_CH0_WRITE_ADDR ((volatile uint32_t*)(DMA_BASE+DMA_CH0_WRITE_ADDR_OFFSET))
#define DMA_CH0_READ_ADDR ((volatile uint32_t*)(DMA_BASE+DMA_CH0_READ_ADDR_OFFSET))
#define DMA_CH0_CTRL_TRIG ((volatile uint32_t*)(DMA_BASE+DMA_CH0_CTRL_TRIG_OFFSET))
#define DMA_CH0_TRANS_COUNT ((volatile uint32_t*)(DMA_BASE+DMA_CH0_TRANS_COUNT_OFFSET))

#define DMA_CH1_WRITE_ADDR ((volatile uint32_t*)(DMA_BASE+DMA_CH1_WRITE_ADDR_OFFSET))
#define DMA_CH1_READ_ADDR ((volatile uint32_t*)(DMA_BASE+DMA_CH1_READ_ADDR_OFFSET))
#define DMA_CH1_CTRL_TRIG ((volatile uint32_t*)(DMA_BASE+DMA_CH1_CTRL_TRIG_OFFSET))
#define DMA_CH1_TRANS_COUNT ((volatile uint32_t*)(DMA_BASE+DMA_CH1_TRANS_COUNT_OFFSET))

#define DMA_CH2_WRITE_ADDR ((volatile uint32_t*)(DMA_BASE+DMA_CH2_WRITE_ADDR_OFFSET))
#define DMA_CH2_READ_ADDR ((volatile uint32_t*)(DMA_BASE+DMA_CH2_READ_ADDR_OFFSET))
#define DMA_CH2_CTRL_TRIG ((volatile uint32_t*)(DMA_BASE+DMA_CH2_CTRL_TRIG_OFFSET))
#define DMA_CH2_TRANS_COUNT ((volatile uint32_t*)(DMA_BASE+DMA_CH2_TRANS_COUNT_OFFSET))
#define DMA_CH2_TRANS_COUNT_TRIG ((volatile uint32_t*)(DMA_BASE+DMA_CH2_TRANS_COUNT_OFFSET+0x14))

#define DMA_CH3_WRITE_ADDR ((volatile uint32_t*)(DMA_BASE+DMA_CH3_WRITE_ADDR_OFFSET))
#define DMA_CH3_READ_ADDR ((volatile uint32_t*)(DMA_BASE+DMA_CH3_READ_ADDR_OFFSET))
#define DMA_CH3_CTRL_TRIG ((volatile uint32_t*)(DMA_BASE+DMA_CH3_CTRL_TRIG_OFFSET))
#define DMA_CH3_TRANS_COUNT ((volatile uint32_t*)(DMA_BASE+DMA_CH3_TRANS_COUNT_OFFSET))
#define DMA_CH3_TRANS_COUNT_TRIG ((volatile uint32_t*)(DMA_BASE+DMA_CH3_TRANS_COUNT_OFFSET+0x14))

#define DMA_INTE0 ((volatile uint32_t*)(DMA_BASE+DMA_INTE0_OFFSET))
#define DMA_INTS0 ((volatile uint32_t*)(DMA_BASE+DMA_INTS0_OFFSET))


#define SIO_FIFO_ST ((volatile uint32_t*)(SIO_BASE + SIO_FIFO_ST_OFFSET))
#define SIO_FIFO_RD ((volatile uint32_t*)(SIO_BASE + SIO_FIFO_RD_OFFSET))
#define SIO_FIFO_WR ((volatile uint32_t*)(SIO_BASE + SIO_FIFO_WR_OFFSET))

#define M0PLUS_VTOR ((volatile uint32_t*)(PPB_BASE + M0PLUS_VTOR_OFFSET))
#define NVIC_ISER ((volatile uint32_t*)(PPB_BASE + M0PLUS_NVIC_ISER_OFFSET))
#define NVIC_ICER ((volatile uint32_t*)(PPB_BASE + M0PLUS_NVIC_ICER_OFFSET))

#define RESETS ((volatile uint32_t*)(RESETS_BASE + RESETS_RESET_OFFSET))
#define RESETS_DONE ((volatile uint32_t*)(RESETS_BASE + RESETS_RESET_DONE_OFFSET))

#define PIO_SM0_EXECCTRL ((volatile uint32_t*)(PIO0_BASE+PIO_SM0_EXECCTRL_OFFSET))
#define PIO_SM0_SHIFTCTRL ((volatile uint32_t*)(PIO0_BASE+PIO_SM0_SHIFTCTRL_OFFSET))
#define PIO_SM0_PINCTRL ((volatile uint32_t*)(PIO0_BASE+PIO_SM0_PINCTRL_OFFSET))
#define PIO_SM0_CLKDIV ((volatile uint32_t*)(PIO0_BASE+PIO_SM0_CLKDIV_OFFSET))
#define PIO_SM0_TXF ((volatile uint32_t*)(PIO0_BASE+PIO_TXF0_OFFSET))
#define PIO_SM0_INSTR  ((volatile uint32_t*)(PIO0_BASE+PIO_SM0_INSTR_OFFSET))

#define PIO_SM1_EXECCTRL ((volatile uint32_t*)(PIO0_BASE+PIO_SM1_EXECCTRL_OFFSET))
#define PIO_SM1_SHIFTCTRL ((volatile uint32_t*)(PIO0_BASE+PIO_SM1_SHIFTCTRL_OFFSET))
#define PIO_SM1_PINCTRL ((volatile uint32_t*)(PIO0_BASE+PIO_SM1_PINCTRL_OFFSET))
#define PIO_SM1_CLKDIV ((volatile uint32_t*)(PIO0_BASE+PIO_SM1_CLKDIV_OFFSET))
#define PIO_SM1_TXF ((volatile uint32_t*)(PIO0_BASE+PIO_TXF1_OFFSET))
#define PIO_SM1_INSTR  ((volatile uint32_t*)(PIO0_BASE+PIO_SM1_INSTR_OFFSET))

#define PIO_SM2_EXECCTRL ((volatile uint32_t*)(PIO0_BASE+PIO_SM2_EXECCTRL_OFFSET))
#define PIO_SM2_SHIFTCTRL ((volatile uint32_t*)(PIO0_BASE+PIO_SM2_SHIFTCTRL_OFFSET))
#define PIO_SM2_PINCTRL ((volatile uint32_t*)(PIO0_BASE+PIO_SM2_PINCTRL_OFFSET))
#define PIO_SM2_CLKDIV ((volatile uint32_t*)(PIO0_BASE+PIO_SM2_CLKDIV_OFFSET))
#define PIO_SM2_TXF ((volatile uint32_t*)(PIO0_BASE+PIO_TXF2_OFFSET))
#define PIO_SM2_INSTR  ((volatile uint32_t*)(PIO0_BASE+PIO_SM2_INSTR_OFFSET))
#define PIO_SM2_RXF ((volatile uint32_t*)(PIO0_BASE+PIO_RXF2_OFFSET))

#define PIO_SM3_EXECCTRL ((volatile uint32_t*)(PIO0_BASE+PIO_SM3_EXECCTRL_OFFSET))
#define PIO_SM3_SHIFTCTRL ((volatile uint32_t*)(PIO0_BASE+PIO_SM3_SHIFTCTRL_OFFSET))
#define PIO_SM3_PINCTRL ((volatile uint32_t*)(PIO0_BASE+PIO_SM3_PINCTRL_OFFSET))
#define PIO_SM3_CLKDIV ((volatile uint32_t*)(PIO0_BASE+PIO_SM3_CLKDIV_OFFSET))
#define PIO_SM3_TXF ((volatile uint32_t*)(PIO0_BASE+PIO_TXF3_OFFSET))
#define PIO_SM3_INSTR  ((volatile uint32_t*)(PIO0_BASE+PIO_SM3_INSTR_OFFSET))
#define PIO_SM3_RXF ((volatile uint32_t*)(PIO0_BASE+PIO_RXF3_OFFSET))

#define PIO_CTRL ((volatile uint32_t*)(PIO0_BASE+PIO_CTRL_OFFSET))
#define PIO_INSTR_MEM ((volatile uint32_t*)(PIO0_BASE+PIO_INSTR_MEM0_OFFSET))
#define PIO_INTE ((volatile uint32_t*)(PIO0_BASE + PIO_IRQ0_INTE_OFFSET))
#define PIO_INTF ((volatile uint32_t*)(PIO0_BASE + PIO_IRQ0_INTF_OFFSET))
#define PIO_IRQ ((volatile uint32_t*)(PIO0_BASE + PIO_IRQ_OFFSET))
#define PIO_IRQ_FORCE ((volatile uint32_t*)(PIO0_BASE + PIO_IRQ_FORCE_OFFSET))
#define PIO_FSTAT ((volatile uint32_t*)(PIO0_BASE + PIO_FSTAT_OFFSET))
#define PIO_FDEBUG ((volatile uint32_t*)(PIO0_BASE + PIO_FDEBUG_OFFSET))

#define PIO1_CTRL ((volatile uint32_t*)(PIO1_BASE+PIO_CTRL_OFFSET))
#define PIO1_INSTR_MEM ((volatile uint32_t*)(PIO1_BASE+PIO_INSTR_MEM0_OFFSET))
#define PIO1_INTE ((volatile uint32_t*)(PIO1_BASE + PIO_IRQ0_INTE_OFFSET))
#define PIO1_INTF ((volatile uint32_t*)(PIO1_BASE + PIO_IRQ0_INTF_OFFSET))
#define PIO1_IRQ ((volatile uint32_t*)(PIO1_BASE + PIO_IRQ_OFFSET))
#define PIO1_IRQ_FORCE ((volatile uint32_t*)(PIO1_BASE + PIO_IRQ_FORCE_OFFSET))
#define PIO1_FSTAT ((volatile uint32_t*)(PIO1_BASE + PIO_FSTAT_OFFSET))

#define PIO1_INSTR_MEM ((volatile uint32_t*)(PIO1_BASE+PIO_INSTR_MEM0_OFFSET))
#define PIO1_CTRL ((volatile uint32_t*)(PIO1_BASE+PIO_CTRL_OFFSET))
#define PIO1_IRQ ((volatile uint32_t*)(PIO1_BASE + PIO_IRQ_OFFSET))
#define PIO1_FDEBUG ((volatile uint32_t*)(PIO1_BASE + PIO_FDEBUG_OFFSET))
#define PIO1_FSTAT ((volatile uint32_t*)(PIO1_BASE + PIO_FSTAT_OFFSET))

#define PIO1_SM0_EXECCTRL ((volatile uint32_t*)(PIO1_BASE+PIO_SM0_EXECCTRL_OFFSET))
#define PIO1_SM0_SHIFTCTRL ((volatile uint32_t*)(PIO1_BASE+PIO_SM0_SHIFTCTRL_OFFSET))
#define PIO1_SM0_PINCTRL ((volatile uint32_t*)(PIO1_BASE+PIO_SM0_PINCTRL_OFFSET))
#define PIO1_SM0_CLKDIV ((volatile uint32_t*)(PIO1_BASE+PIO_SM0_CLKDIV_OFFSET))
#define PIO1_SM0_TXF ((volatile uint32_t*)(PIO1_BASE+PIO_TXF0_OFFSET))
#define PIO1_SM0_INSTR  ((volatile uint32_t*)(PIO1_BASE+PIO_SM0_INSTR_OFFSET))
#define PIO1_SM0_RXF ((volatile uint32_t*)(PIO1_BASE+PIO_RXF0_OFFSET))

#define PIO1_SM1_EXECCTRL ((volatile uint32_t*)(PIO1_BASE+PIO_SM1_EXECCTRL_OFFSET))
#define PIO1_SM1_SHIFTCTRL ((volatile uint32_t*)(PIO1_BASE+PIO_SM1_SHIFTCTRL_OFFSET))
#define PIO1_SM1_PINCTRL ((volatile uint32_t*)(PIO1_BASE+PIO_SM1_PINCTRL_OFFSET))
#define PIO1_SM1_CLKDIV ((volatile uint32_t*)(PIO1_BASE+PIO_SM1_CLKDIV_OFFSET))
#define PIO1_SM1_TXF ((volatile uint32_t*)(PIO1_BASE+PIO_TXF1_OFFSET))
#define PIO1_SM1_INSTR  ((volatile uint32_t*)(PIO1_BASE+PIO_SM1_INSTR_OFFSET))
#define PIO1_SM1_RXF ((volatile uint32_t*)(PIO1_BASE+PIO_RXF1_OFFSET))

#define PIO1_SM2_EXECCTRL ((volatile uint32_t*)(PIO1_BASE+PIO_SM2_EXECCTRL_OFFSET))
#define PIO1_SM2_SHIFTCTRL ((volatile uint32_t*)(PIO1_BASE+PIO_SM2_SHIFTCTRL_OFFSET))
#define PIO1_SM2_PINCTRL ((volatile uint32_t*)(PIO1_BASE+PIO_SM2_PINCTRL_OFFSET))
#define PIO1_SM2_CLKDIV ((volatile uint32_t*)(PIO1_BASE+PIO_SM2_CLKDIV_OFFSET))
#define PIO1_SM2_TXF ((volatile uint32_t*)(PIO1_BASE+PIO_TXF2_OFFSET))
#define PIO1_SM2_INSTR  ((volatile uint32_t*)(PIO1_BASE+PIO_SM2_INSTR_OFFSET))
#define PIO1_SM2_RXF ((volatile uint32_t*)(PIO1_BASE+PIO_RXF2_OFFSET))


#define I2C_ENABLE_IC ((volatile uint32_t*)(I2C0_BASE +I2C_IC_ENABLE_OFFSET))
#define I2C_IC_CON ((volatile uint32_t*)(I2C0_BASE +I2C_IC_CON_OFFSET))
#define I2C_IC_TAR ((volatile uint32_t*)(I2C0_BASE +I2C_IC_TAR_OFFSET))
#define I2C_IC_DATA_CMD ((volatile uint32_t*)(I2C0_BASE +I2C_IC_DATA_CMD_OFFSET))

#define I2C_IC_SS_SCL_HCNT ((volatile uint32_t*)(I2C0_BASE +I2C_IC_SS_SCL_HCNT_OFFSET))
#define I2C_IC_SS_SCL_LCNT ((volatile uint32_t*)(I2C0_BASE +I2C_IC_SS_SCL_LCNT_OFFSET))
#define I2C_IC_STATUS ((volatile uint32_t*)(I2C0_BASE +I2C_IC_STATUS_OFFSET))

#define I2C_IC_CLR_TX_ABRT ((volatile uint32_t*)(I2C0_BASE +I2C_IC_CLR_TX_ABRT_OFFSET))
#define I2C_IC_RAW_INTR_STAT ((volatile uint32_t*)(I2C0_BASE +I2C_IC_RAW_INTR_STAT_OFFSET))
#define I2C_IC_TX_ABRT_SOURCE ((volatile uint32_t*)(I2C0_BASE +I2C_IC_TX_ABRT_SOURCE_OFFSET))

#define SSPCR0 ((volatile uint32_t*)(SPI0_BASE +  SPI_SSPCR0_OFFSET))
#define SSPCR1 ((volatile uint32_t*)(SPI0_BASE +  SPI_SSPCR1_OFFSET))
#define SSPCPSR ((volatile uint32_t*)(SPI0_BASE +  SPI_SSPCPSR_OFFSET))
#define SSPDR ((volatile uint32_t*)(SPI0_BASE +  SPI_SSPDR_OFFSET))
#define SSPSR ((volatile uint32_t*)(SPI0_BASE +  SPI_SSPSR_OFFSET))

#define XOSC_STARTUP ((volatile uint32_t*)(XOSC_BASE+XOSC_STARTUP_OFFSET))
#define XOSC_CTRL_ENABLE ((volatile uint32_t*)(XOSC_BASE+XOSC_CTRL_OFFSET))
#define XOSC_STATUS ((volatile uint32_t*)(XOSC_BASE+XOSC_STATUS_OFFSET))

#define CLK_SYS_CTRL ((volatile uint32_t*)(CLOCKS_BASE+CLOCKS_CLK_SYS_CTRL_OFFSET))
#define CLK_REF_CTRL ((volatile uint32_t*)(CLOCKS_BASE+CLOCKS_CLK_REF_CTRL_OFFSET))
#define CLK_PERI_CTRL ((volatile uint32_t*)(CLOCKS_BASE+CLOCKS_CLK_PERI_CTRL_OFFSET))

typedef struct {
	volatile uint32_t cs;
	volatile uint32_t pwr;
	volatile uint32_t fbdiv;
	volatile uint32_t prim;
} PllType;

#define PLL_SYS ((PllType*)PLL_SYS_BASE)

#define PLL_USB ((PllType*)PLL_USB_BASE)

#define M0PLUS_SYST_CSR ((volatile uint32_t*)(PPB_BASE + M0PLUS_SYST_CSR_OFFSET))
#define M0PLUS_SYST_RVR ((volatile uint32_t*)(PPB_BASE + M0PLUS_SYST_RVR_OFFSET))
#define M0PLUS_SYST_CVR ((volatile uint32_t*)(PPB_BASE + M0PLUS_SYST_CVR_OFFSET))

// device-specific pins/pads
#define DS18B20_PIN_CNTR ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8*DS18B20_PIN))
#define DS18B20_PAD_CNTR ((volatile uint32_t*)(PADS_BANK0_BASE + PADS_BANK0_GPIO0_OFFSET + 4*DS18B20_PIN))
#define HEATER_PIN_CNTR ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8*HEATER))
#define DEBUG_LED_PIN_CNTR ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8*RP2040_LED_PIN))

#define I2S_WS_PIN_CNTR ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8*I2S_WS_PIN))
#define I2S_WS_PAD_CNTR ((volatile uint32_t*)(PADS_BANK0_BASE + PADS_BANK0_GPIO0_OFFSET + 4*I2S_WS_PIN))
#define I2S_BCK_PIN_CNTR ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8*I2S_BCK_PIN))
#define I2S_BCK_PAD_CNTR ((volatile uint32_t*)(PADS_BANK0_BASE + PADS_BANK0_GPIO0_OFFSET + 4*I2S_BCK_PIN))
#define I2S_DATA_PIN_CNTR ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8*I2S_DATA_PIN))
#define I2S_DATA_PAD_CNTR ((volatile uint32_t*)(PADS_BANK0_BASE + PADS_BANK0_GPIO0_OFFSET + 4*I2S_DATA_PIN))
#define I2S_DATA_IN_PIN_CNTR ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8*I2S_DATA_IN_PIN))
#define I2S_DATA_IN_PAD_CNTR ((volatile uint32_t*)(PADS_BANK0_BASE + PADS_BANK0_GPIO0_OFFSET + 4*I2S_DATA_IN_PIN))
#define I2S_MCLK_PIN_CNTR ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8*I2S_MCLK_PIN))
#define I2S_MCLK_PAD_CNTR ((volatile uint32_t*)(PADS_BANK0_BASE + PADS_BANK0_GPIO0_OFFSET + 4*I2S_MCLK_PIN))
#define I2S_DEBUG_PIN_CNTR ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8*I2S_DEBUG_PIN))
#define I2S_DEBUG_PAD_CNTR ((volatile uint32_t*)(PADS_BANK0_BASE + PADS_BANK0_GPIO0_OFFSET + 4*I2S_DEBUG_PIN))

#define NEOPIXEL_PIN_CNTR ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8*NEOPIXEL_PIN))
#ifdef ITSYBITSY
#define NEOPIXEL_POWER_PIN_CNTR ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8*NEOPIXEL_POWER_PIN))
#endif

#define REMOTESWITCH_PIN_CNTR ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8*REMOTESWITCH_PIN))
#define REMOTESWITCH_PAD_CNTR ((volatile uint32_t*)(PADS_BANK0_BASE + PADS_BANK0_GPIO0_OFFSET + 4*REMOTESWITCH_PIN))


#define ENCODER_1_PIN_CNTR  ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8*ENCODER_1))
#define ENCODER_1_PAD_CNTR ((volatile uint32_t*)(PADS_BANK0_BASE + PADS_BANK0_GPIO0_OFFSET + 4*ENCODER_1))
#define ENCODER_1_INTE ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_PROC0_INTE0_OFFSET + (((4*ENCODER_1) & 0xFFE0) >> 3)))
#define ENCODER_1_INTR ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_INTR0_OFFSET + (((4*ENCODER_1) & 0xFFE0) >> 3)))
#define ENCODER_1_EDGE_LOW (((4*ENCODER_1) & 0x1F)+2)
#define ENCODER_1_EDGE_HIGH (((4*ENCODER_1) & 0x1F)+3)

#define ENCODER_2_PIN_CNTR  ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8*ENCODER_2))
#define ENCODER_2_PAD_CNTR ((volatile uint32_t*)(PADS_BANK0_BASE + PADS_BANK0_GPIO0_OFFSET + 4*ENCODER_2))
#define ENCODER_2_INTE ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_PROC0_INTE0_OFFSET + (((4*ENCODER_2) & 0xFFE0) >> 3)))
#define ENCODER_2_INTR ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_INTR0_OFFSET + (((4*ENCODER_2) & 0xFFE0) >> 3)))
#define ENCODER_2_EDGE_LOW (((4*ENCODER_2) & 0x1F)+2)
#define ENCODER_2_EDGE_HIGH (((4*ENCODER_2) & 0x1F)+3)

#define SWITCH_PIN_CNTR  ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8*SWITCH))
#define SWITCH_PAD_CNTR ((volatile uint32_t*)(PADS_BANK0_BASE + PADS_BANK0_GPIO0_OFFSET + 4*SWITCH))
#define SWITCH_INTE ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_PROC0_INTE0_OFFSET + (((4*SWITCH) & 0xFFE0) >> 3)))
#define SWITCH_INTR ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_INTR0_OFFSET + (((4*SWITCH) & 0xFFE0) >> 3)))
#define SWITCH_EDGE_LOW (((4*SWITCH) & 0x1F)+2)
#define SWITCH_EDGE_HIGH (((4*SWITCH) & 0x1F)+3)

#define MISO_PIN_CNTR ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8*MISO))
#define MOSI_PIN_CNTR ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8*MOSI))
#define SCK_PIN_CNTR  ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8*SCK))
#define MISO_PAD_CNTR ((volatile uint32_t*)(PADS_BANK0_BASE + PADS_BANK0_GPIO0_OFFSET + 4*MISO))
#define CS_SDCARD_PIN_CNTR ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8*CS_SDCARD))
#define CS_DISPLAY_PIN_CNTR ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8*CS_DISPLAY))

#define DISPLAY_RESET_PIN_CNTR ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8*DISPLAY_RESET))
#define DISPLAY_CD_PIN_CNTR ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8*DISPLAY_CD))

#define I2C_SCL_PIN_CNTR ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8*I2C_SCL))
#define I2C_SDA_PIN_CNTR ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8*I2C_SDA))

#define I2C_SCL_PAD_CNTR ((volatile uint32_t*)(PADS_BANK0_BASE + 4*I2C_SCL + 4))
#define I2C_SDA_PAD_CNTR ((volatile uint32_t*)(PADS_BANK0_BASE + 4*I2C_SDA + 4))

#define DISPLAY_BACKLIGHT_PIN_CNTR ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8*DISPLAY_BACKLIGHT))


#define SSD1306_MOSI_PIN_CNTR ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8*SSD1306_MOSI))
#define SSD1306_CS_DISPLAY_PIN_CNTR ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8*SSD1306_CS_DISPLAY))
#define SSD1306_DISPLAY_CD_PIN_CNTR ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8*SSD1306_DISPLAY_CD))
#define SSD1306_SCK_PIN_CNTR ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8*SSD1306_SCK))
#define SSD1306_DISPLAY_RESET_PIN_CNTR ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8*SSD1306_DISPLAY_RESET))

#define TIMER_TIMEHW ((volatile uint32_t*)(TIMER_BASE + TIMER_TIMEHW_OFFSET))
#define TIMER_TIMELW ((volatile uint32_t*)(TIMER_BASE + TIMER_TIMELW_OFFSET))
#define TIMER_TIMEHR ((volatile uint32_t*)(TIMER_BASE + TIMER_TIMEHR_OFFSET))
#define TIMER_TIMELR ((volatile uint32_t*)(TIMER_BASE + TIMER_TIMELR_OFFSET))

#define WATCHDOG_TICK ((volatile uint32_t*)(WATCHDOG_BASE + WATCHDOG_TICK_OFFSET))

#define UART_UARTIBRD ((volatile uint32_t*)(UART0_BASE+UART_UARTIBRD_OFFSET))
#define UART_UARTFBRD ((volatile uint32_t*)(UART0_BASE+UART_UARTFBRD_OFFSET))
#define UART_UARTCR ((volatile uint32_t*)(UART0_BASE+UART_UARTCR_OFFSET))
#define UART_UARTLCR_H ((volatile uint32_t*)(UART0_BASE+UART_UARTLCR_H_OFFSET))
#define UART_RX_PIN_CNTR ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8*UART_USB_RX))
#define UART_TX_PIN_CNTR ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8*UART_USB_TX))
#define UART_UARTIMSC ((volatile uint32_t*)(UART0_BASE+UART_UARTIMSC_OFFSET))
#define UART_UARTMIS ((volatile uint32_t*)(UART0_BASE+UART_UARTMIS_OFFSET))
#define UART_UARTCR ((volatile uint32_t*)(UART0_BASE+UART_UARTCR_OFFSET))
#define UART_UARTDR ((volatile uint32_t*)(UART0_BASE+UART_UARTDR_OFFSET))
#define UART_UARTRIS ((volatile uint32_t*)(UART0_BASE+UART_UARTRIS_OFFSET))
#define UART_UARTFR ((volatile uint32_t*)(UART0_BASE+UART_UARTFR_OFFSET))
#define UART_UARTDMACR ((volatile uint32_t*)(UART0_BASE+UART_UARTDMACR_OFFSET))

#define UARTBT_UARTIBRD ((volatile uint32_t*)(UART1_BASE+UART_UARTIBRD_OFFSET))
#define UARTBT_UARTFBRD ((volatile uint32_t*)(UART1_BASE+UART_UARTFBRD_OFFSET))
#define UARTBT_UARTCR ((volatile uint32_t*)(UART1_BASE+UART_UARTCR_OFFSET))
#define UARTBT_UARTLCR_H ((volatile uint32_t*)(UART1_BASE+UART_UARTLCR_H_OFFSET))
#define UARTBT_RX_PIN_CNTR ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8*UART_BT_RX))
#define UARTBT_TX_PIN_CNTR ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8*UART_BT_TX))
#define UARTBT_UARTIMSC ((volatile uint32_t*)(UART1_BASE+UART_UARTIMSC_OFFSET))
#define UARTBT_UARTMIS ((volatile uint32_t*)(UART1_BASE+UART_UARTMIS_OFFSET))
#define UARTBT_UARTCR ((volatile uint32_t*)(UART1_BASE+UART_UARTCR_OFFSET))
#define UARTBT_UARTDR ((volatile uint32_t*)(UART1_BASE+UART_UARTDR_OFFSET))
#define UARTBT_UARTRIS ((volatile uint32_t*)(UART1_BASE+UART_UARTRIS_OFFSET))
#define UARTBT_UARTFR ((volatile uint32_t*)(UART1_BASE+UART_UARTFR_OFFSET))
#define UARTBT_UARTDMACR ((volatile uint32_t*)(UART1_BASE+UART_UARTDMACR_OFFSET))

// PWM Channels
#define PWM_CH1_CSR ((volatile uint32_t*)(PWM_BASE + PWM_CH0_CSR_OFFSET + 10*HEATER))
#define PWM_CH1_DIV ((volatile uint32_t*)(PWM_BASE + PWM_CH0_DIV_OFFSET + 10*HEATER))
#define PWM_CH1_CC ((volatile uint16_t*)(PWM_BASE + PWM_CH0_CC_OFFSET + 10*HEATER + (HEATER & 1)*2))
#define PWM_CH1_TOP ((volatile uint32_t*)(PWM_BASE + PWM_CH0_TOP_OFFSET + 10*HEATER))

#define PWM_CH0_CSR ((volatile uint32_t*)(PWM_BASE + PWM_CH0_CSR_OFFSET + 10*DISPLAY_BACKLIGHT))
#define PWM_CH0_DIV ((volatile uint32_t*)(PWM_BASE + PWM_CH0_DIV_OFFSET + 10*DISPLAY_BACKLIGHT))
#define PWM_CH0_CC ((volatile uint16_t*)(PWM_BASE + PWM_CH0_CC_OFFSET + 10*DISPLAY_BACKLIGHT + (DISPLAY_BACKLIGHT & 1)*2))
#define PWM_CH0_TOP ((volatile uint32_t*)(PWM_BASE + PWM_CH0_TOP_OFFSET + 10*DISPLAY_BACKLIGHT))

#endif