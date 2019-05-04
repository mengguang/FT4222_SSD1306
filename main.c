#include <stdio.h>
#include "ftd2xx.h"
#include "LibFT4222.h"
#include <stdint.h>
#include "ssd1306.h"
#include "ssd1306_tests.h"

FT_DEVICE_LIST_INFO_NODE g_FT4222DevList[8];
uint32_t ndev = 0;

FT_HANDLE ftHandle = NULL;

void ListFtUsbDevices() {
    FT_STATUS ftStatus = 0;

    DWORD numOfDevices = 0;
    ftStatus = FT_CreateDeviceInfoList(&numOfDevices);
    if (ftStatus != FT_OK) {
        printf("FT_CreateDeviceInfoList error: %lu\n", ftStatus);
        return;
    }

    for (DWORD iDev = 0; iDev < numOfDevices; ++iDev) {
        FT_DEVICE_LIST_INFO_NODE devInfo;
        memset(&devInfo, 0, sizeof(devInfo));

        ftStatus = FT_GetDeviceInfoDetail(iDev, &devInfo.Flags, &devInfo.Type,
                                          &devInfo.ID, &devInfo.LocId,
                                          devInfo.SerialNumber,
                                          devInfo.Description,
                                          &devInfo.ftHandle);

        if (FT_OK == ftStatus) {
            const char *desc = devInfo.Description;
            if ((strcmp(desc, "FT4222") == 0) || (strcmp(desc, "FT4222 A") == 0)) {
                g_FT4222DevList[ndev] = devInfo;
                ndev++;
            }
        }
    }
}


char *DeviceFlagToString(DWORD flags) {
    static char desc[32];
    if (flags & 0x01) {
        snprintf(desc, 32, "%s, ", "DEVICE_OPEN");
    } else {
        snprintf(desc, 32, "%s, ", "DEVICE_CLOSED");
    }
    if (flags & 0x02) {
        snprintf(desc + strlen(desc), 32 - strlen(desc), "%s", "High-speed USB");
    } else {
        snprintf(desc + strlen(desc), 32 - strlen(desc), "%s", "Full-speed USB");
    }
    return desc;
}

int main() {
    ListFtUsbDevices();
    if (ndev == 0) {
        printf("No FT4222 device is found!\n");
        return 0;
    }
    const FT_DEVICE_LIST_INFO_NODE devInfo = g_FT4222DevList[0];

    printf("Open Device\n");
    printf("  Flags = 0x%lx, (%s)\n", devInfo.Flags, DeviceFlagToString(devInfo.Flags));
    printf("  Type = 0x%lx\n", devInfo.Type);
    printf("  ID = 0x%lx\n", devInfo.ID);
    printf("  LocId = 0x%lx\n", devInfo.LocId);
    printf("  SerialNumber = %s\n", devInfo.SerialNumber);
    printf("  Description = %s\n", devInfo.Description);
    printf("  ftHandle = 0x%p\n", devInfo.ftHandle);


    FT_STATUS ftStatus;
    ftStatus = FT_OpenEx((PVOID)((int64_t)(devInfo.LocId)), FT_OPEN_BY_LOCATION, &ftHandle);
    //ftStatus = FT_Open(0, &ftHandle);
    if (FT_OK != ftStatus) {
        printf("Open a FT4222 device failed!\n");
        return 0;
    }

    printf("\n\n");
    printf("Init FT4222 as I2C master\n");
    ftStatus = FT4222_I2CMaster_Init(ftHandle, 1000);
    if (FT_OK != ftStatus) {
        printf("Init FT4222 as I2C master device failed!\n");
        return 0;
    }
    FT4222_I2CMaster_Reset(ftHandle);

    ssd1306_Init();
    for (int i = 0; i < 1000; i++) {
        ssd1306_TestAll();
        printf("testing %d\n",i);
    }

    printf("UnInitialize FT4222\n");
    FT4222_UnInitialize(ftHandle);

    printf("Close FT device\n");
    FT_Close(ftHandle);
    return 0;
}