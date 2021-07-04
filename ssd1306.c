#include "ssd1306.h"
#include "stdbool.h"

extern FT_HANDLE ftHandleI2C;

void ssd1306_Reset(void)
{
    /* for I2C - do nothing */
}

static uint8_t i2c_buffer[256];

static inline bool wait_i2c_idle()
{
    uint8_t cStatus = 0;
    uint32_t max_wait_ms = 1000;
    while (max_wait_ms > 0)
    {
        FT_STATUS ftStatus = FT4222_I2CMaster_GetStatus(ftHandleI2C, &cStatus);
        if ((ftStatus != 0x20) && (ftStatus != 0x00))
        {
            HAL_Delay(2);
            max_wait_ms -= 2;
        }
        else
        {
            return true;
        }
    }
    printf("cStatus: %d\n", cStatus);
    return false;
}

// Send a byte to the command register
static inline void ssd1306_WriteCommand(uint8_t byte)
{
    FT_STATUS ftStatus;
    uint16_t sizeTransferred = 0;
    //printf("I2C master write data to the slave(%#x)... \n", SSD1306_I2C_ADDR);
    i2c_buffer[0] = 0x00;
    i2c_buffer[1] = byte;
    ftStatus = FT4222_I2CMaster_Write(ftHandleI2C, SSD1306_I2C_ADDR, i2c_buffer, 2, &sizeTransferred);
    if (FT_OK != ftStatus)
    {
        printf("I2C master write error\n");
    }
    //printf("sizeTransferred: %d\n",sizeTransferred);
    wait_i2c_idle();
}

// Send data
static inline void ssd1306_WriteData(uint8_t *buffer, uint8_t buff_size)
{
    FT_STATUS ftStatus;
    uint16_t sizeTransferred = 0;
    //printf("I2C master write data to the slave(%#x)... \n", SSD1306_I2C_ADDR);
    i2c_buffer[0] = 0x40;
    memcpy(i2c_buffer + 1, buffer, buff_size);
    ftStatus = FT4222_I2CMaster_Write(ftHandleI2C, SSD1306_I2C_ADDR, i2c_buffer, buff_size + 1, &sizeTransferred);
    if (FT_OK != ftStatus)
    {
        printf("I2C master write error\n");
    }
    //printf("sizeTransferred: %d\n",sizeTransferred);
    wait_i2c_idle();
}

// Screenbuffer
static uint8_t SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

// Screen object
static SSD1306_t SSD1306;

// Initialize the oled screen
void ssd1306_Init(void)
{
    // Reset OLED
    ssd1306_Reset();

    // Wait for the screen to boot
    HAL_Delay(100);

    // Init OLED
    ssd1306_WriteCommand(0xAE); //display off

    ssd1306_WriteCommand(0x20); //Set Memory Addressing Mode
    // 00,Horizontal Addressing Mode; 01,Vertical Addressing Mode;
    // 10,Page Addressing Mode (RESET); 11,Invalid
    ssd1306_WriteCommand(0x10);

    ssd1306_WriteCommand(0xB0); //Set Page Start Address for Page Addressing Mode,0-7

#ifdef SSD1306_MIRROR_VERT
    ssd1306_WriteCommand(0xC0); // Mirror vertically
#else
    ssd1306_WriteCommand(0xC8); //Set COM Output Scan Direction
#endif

    ssd1306_WriteCommand(0x00); //---set low column address
    ssd1306_WriteCommand(0x10); //---set high column address

    ssd1306_WriteCommand(0x40); //--set start line address - CHECK

    ssd1306_WriteCommand(0x81); //--set contrast control register - CHECK
    ssd1306_WriteCommand(0xFF);

#ifdef SSD1306_MIRROR_HORIZ
    ssd1306_WriteCommand(0xA0); // Mirror horizontally
#else
    ssd1306_WriteCommand(0xA1); //--set segment re-map 0 to 127 - CHECK
#endif

#ifdef SSD1306_INVERSE_COLOR
    ssd1306_WriteCommand(0xA7); //--set inverse color
#else
    ssd1306_WriteCommand(0xA6); //--set normal color
#endif

    ssd1306_WriteCommand(0xA8); //--set multiplex ratio(1 to 64) - CHECK
#if SSD1306_HEIGHT == 32
    ssd1306_WriteCommand(0x1F);
#else
    ssd1306_WriteCommand(0x3F);
#endif

    ssd1306_WriteCommand(0xA4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content

    ssd1306_WriteCommand(0xD3); //-set display offset - CHECK
    ssd1306_WriteCommand(0x00); //-not offset

    ssd1306_WriteCommand(0xD5); //--set display clock divide ratio/oscillator frequency
    ssd1306_WriteCommand(0xF0); //--set divide ratio

    ssd1306_WriteCommand(0xD9); //--set pre-charge period
    ssd1306_WriteCommand(0x22); //

    ssd1306_WriteCommand(0xDA); //--set com pins hardware configuration - CHECK
#if SSD1306_HEIGHT == 32
    ssd1306_WriteCommand(0x02);
#else
    ssd1306_WriteCommand(0x12);
#endif

    ssd1306_WriteCommand(0xDB); //--set vcomh
    ssd1306_WriteCommand(0x20); //0x20,0.77xVcc

    ssd1306_WriteCommand(0x8D); //--set DC-DC enable
    ssd1306_WriteCommand(0x14); //
    ssd1306_WriteCommand(0xAF); //--turn on SSD1306 panel

    // Clear screen
    ssd1306_Fill(Black);

    // Flush buffer to screen
    ssd1306_UpdateScreen();

    // Set default values for screen object
    SSD1306.CurrentX = 0;
    SSD1306.CurrentY = 0;

    SSD1306.Initialized = 1;
}

// Fill the whole screen with the given color
void ssd1306_Fill(SSD1306_COLOR color)
{
    /* Set memory */
    uint32_t i;

    for (i = 0; i < sizeof(SSD1306_Buffer); i++)
    {
        SSD1306_Buffer[i] = (color == Black) ? 0x00 : 0xFF;
    }
}

// Write the screenbuffer with changed to the screen
void ssd1306_UpdateScreen(void)
{
    uint8_t i;
    for (i = 0; i < (SSD1306_HEIGHT / 8); i++)
    {
        ssd1306_WriteCommand(0xB0 + i);
        ssd1306_WriteCommand(0x00);
        ssd1306_WriteCommand(0x10);
        ssd1306_WriteData(&SSD1306_Buffer[SSD1306_WIDTH * i], SSD1306_WIDTH);
    }
}

//    Draw one pixel in the screenbuffer
//    X => X Coordinate
//    Y => Y Coordinate
//    color => Pixel color
void ssd1306_DrawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color)
{
#if SSD1306_WIDTH == 132
    x += 2;
#endif

    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT)
    {
        // Don't write outside the buffer
        return;
    }

    // Check if pixel should be inverted
    if (SSD1306.Inverted)
    {
        color = (SSD1306_COLOR)!color;
    }

    // Draw in the right color
    if (color == White)
    {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
    }
    else
    {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
    }
}

// Draw 1 char to the screen buffer
// ch         => char om weg te schrijven
// Font     => Font waarmee we gaan schrijven
// color     => Black or White
char ssd1306_WriteChar(char ch, FontDef Font, SSD1306_COLOR color)
{
    uint32_t i, b, j;

    // Check remaining space on current line
    if (SSD1306_WIDTH <= (SSD1306.CurrentX + Font.FontWidth) ||
        SSD1306_HEIGHT <= (SSD1306.CurrentY + Font.FontHeight))
    {
        // Not enough space on current line
        return 0;
    }

    // Use the font to write
    for (i = 0; i < Font.FontHeight; i++)
    {
        b = Font.data[(ch - 32) * Font.FontHeight + i];
        for (j = 0; j < Font.FontWidth; j++)
        {
            if ((b << j) & 0x8000)
            {
                ssd1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i),
                                  (SSD1306_COLOR)color);
            }
            else
            {
                ssd1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i),
                                  (SSD1306_COLOR)!color);
            }
        }
    }

    // The current space is now taken
    SSD1306.CurrentX += Font.FontWidth;

    // Return written char for validation
    return ch;
}

// Write full string to screenbuffer
char ssd1306_WriteString(const char *str, FontDef Font, SSD1306_COLOR color)
{
    // Write until null-byte
    while (*str)
    {
        if (ssd1306_WriteChar(*str, Font, color) != *str)
        {
            // Char could not be written
            return *str;
        }

        // Next char
        str++;
    }

    // Everything ok
    return *str;
}

// Write full string to screenbuffer
char ssd1306_WriteString_with_length(const char *str, uint32_t length,
                                     FontDef Font, SSD1306_COLOR color)
{
    // Write until null-byte
    while (length > 0)
    {
        if (ssd1306_WriteChar(*str, Font, color) != *str)
        {
            // Char could not be written
            return *str;
        }

        // Next char
        str++;
        length--;
    }

    // Everything ok
    return *str;
}

// Position the cursor
void ssd1306_SetCursor(uint8_t x, uint8_t y)
{
    SSD1306.CurrentX = x;
    SSD1306.CurrentY = y;
}
