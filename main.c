#include <stdio.h>
#include "ftd2xx.h"
#include "LibFT4222.h"
#include <stdint.h>
#include "ssd1306.h"
#include "ssd1306_tests.h"

FT_DEVICE_LIST_INFO_NODE g_FT4222DevList[8];
uint32_t ndev = 0;

FT_HANDLE ftHandleI2C = NULL;
FT_HANDLE ftHandleGPIO = NULL;

void ListFtUsbDevices()
{
    FT_STATUS ftStatus = 0;

    DWORD numOfDevices = 0;
    ftStatus = FT_CreateDeviceInfoList(&numOfDevices);
    if (ftStatus != FT_OK)
    {
        printf("FT_CreateDeviceInfoList error: %lu\n", ftStatus);
        return;
    }

    for (DWORD iDev = 0; iDev < numOfDevices; ++iDev)
    {
        FT_DEVICE_LIST_INFO_NODE devInfo;
        memset(&devInfo, 0, sizeof(devInfo));

        ftStatus = FT_GetDeviceInfoDetail(iDev, &devInfo.Flags, &devInfo.Type,
                                          &devInfo.ID, &devInfo.LocId,
                                          devInfo.SerialNumber,
                                          devInfo.Description,
                                          &devInfo.ftHandle);

        if (FT_OK == ftStatus)
        {
            g_FT4222DevList[ndev] = devInfo;
            ndev++;
        }
    }
}

char *DeviceFlagToString(DWORD flags)
{
    static char desc[32];
    if (flags & 0x01)
    {
        snprintf(desc, 32, "%s, ", "DEVICE_OPEN");
    }
    else
    {
        snprintf(desc, 32, "%s, ", "DEVICE_CLOSED");
    }
    if (flags & 0x02)
    {
        snprintf(desc + strlen(desc), 32 - strlen(desc), "%s", "High-speed USB");
    }
    else
    {
        snprintf(desc + strlen(desc), 32 - strlen(desc), "%s", "Full-speed USB");
    }
    return desc;
}

void print_version(FT_HANDLE ftHandle)
{
    struct FT4222_Version version;
    FT4222_STATUS s = FT4222_GetVersion(ftHandle, &version);
    printf("chipVersion: %08X\n", version.chipVersion);
    printf("libraryVersion: %08X\n", version.dllVersion);
}

int main()
{
    ListFtUsbDevices();
    if (ndev == 0)
    {
        printf("No FT4222 device is found!\n");
        return 0;
    }

    for (uint32_t i = 0; i < ndev; i++)
    {
        const FT_DEVICE_LIST_INFO_NODE devInfo = g_FT4222DevList[i];

        printf("Open Device\n");
        printf("  Flags = 0x%lx, (%s)\n", devInfo.Flags, DeviceFlagToString(devInfo.Flags));
        printf("  Type = 0x%lx\n", devInfo.Type);
        printf("  ID = 0x%lx\n", devInfo.ID);
        printf("  LocId = 0x%lx\n", devInfo.LocId);
        printf("  SerialNumber = %s\n", devInfo.SerialNumber);
        printf("  Description = %s\n", devInfo.Description);
        printf("  ftHandle = 0x%p\n", devInfo.ftHandle);
    }

    FT_STATUS ftStatus;
    ftStatus = FT_OpenEx("FT4222 A", FT_OPEN_BY_DESCRIPTION, &ftHandleI2C);
    if (FT_OK != ftStatus)
    {
        printf("Open a FT4222 device for I2C failed!\n");
        return 0;
    }
    print_version(ftHandleI2C);
    ftStatus = FT_OpenEx("FT4222 B", FT_OPEN_BY_DESCRIPTION, &ftHandleGPIO);
    if (FT_OK != ftStatus)
    {
        printf("Open a FT4222 device for GPIO failed!\n");
        return 0;
    }
    // print_version(ftHandleGPIO);
    printf("\n\n");
    printf("Init FT4222 as I2C master\n");
    ftStatus = FT4222_I2CMaster_Init(ftHandleI2C, 1000);
    if (FT_OK != ftStatus)
    {
        printf("Init FT4222 as I2C master device failed!\n");
        return 0;
    }
    // FT4222_I2CMaster_Reset(ftHandleI2C);

    //disable suspend out , enable gpio 2
    FT4222_SetSuspendOut(ftHandleGPIO, FALSE);
    //disable interrupt , enable gpio 3
    FT4222_SetWakeUpInterrupt(ftHandleGPIO, FALSE);
    FT4222_STATUS ft4222Status;
    GPIO_Dir gpio_dir[4] = {GPIO_INPUT, GPIO_INPUT, GPIO_OUTPUT, GPIO_OUTPUT};
    ft4222Status = FT4222_GPIO_Init(ftHandleGPIO, gpio_dir);
    if (FT4222_OK != ft4222Status)
    {
        printf("FT4222_GPIO_Init failed!\n");
        return 0;
    }

    uint8_t led_value = 1;

    ssd1306_Init();
    for (int i = 0; i < 1000; i++)
    {
        printf("testing %d\n", i);
        FT4222_GPIO_Write(ftHandleGPIO, GPIO_PORT2, led_value);
        FT4222_GPIO_Write(ftHandleGPIO, GPIO_PORT3, 1 - led_value);
        led_value = 1 - led_value;
        HAL_Delay(200);
        ssd1306_TestAll();
    }

    printf("UnInitialize FT4222\n");
    FT4222_UnInitialize(ftHandleGPIO);
    FT4222_UnInitialize(ftHandleI2C);

    printf("Close FT device\n");
    FT_Close(ftHandleGPIO);
    FT_Close(ftHandleI2C);
    return 0;
}