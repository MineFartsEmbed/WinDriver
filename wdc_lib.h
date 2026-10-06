/* Jungo Connectivity Confidential. Copyright (c) 2026 Jungo Connectivity Ltd.  https://www.jungo.com */

#ifndef _WDC_LIB_H_
#define _WDC_LIB_H_

/*********************************************************************
*  File: wdc_lib.h - Shared WD card (WDC) library header.            *
*                    This file defines the WDC library's high-level  *
*                    interface                                       *
**********************************************************************/

#if defined(__KERNEL__)
    #include "kpstdlib.h"
#endif
#include "windrvr.h"
#include "windrvr_int_thread.h"
#include "windrvr_events.h"
#include "bits.h"
#include "pci_regs.h"

#ifdef __cplusplus
    extern "C" {
#endif
/**************************************************************
  General definitions
 **************************************************************/

#define MAX_NAME 128
#define MAX_DESC 128
#define MAX_NAME_DISPLAY 22

/** Handle to device information struct */
typedef void *WDC_DEVICE_HANDLE;

#ifndef __KERNEL__
/** PCI scan results */
typedef struct {
    DWORD       dwNumDevices;             /**< Number of matching devices */
    WD_PCI_ID   deviceId[WD_PCI_CARDS];   /**< Array of matching device IDs */
    WD_PCI_SLOT deviceSlot[WD_PCI_CARDS]; /**< Array of matching device
                                          locations */
} WDC_PCI_SCAN_RESULT;

#if defined(PLATFORM_API)
/** Platform scan results 
 * @note This structure is available only if PLATFORM_API is
 * defined.
*/
typedef struct {
    DWORD dwNumDevices; /**< Number of matching devices */
    CHAR pcDeviceNames[WD_PLATFORM_CARDS][WD_DEVICE_NAME_LENGTH]; /**< Array of 
                                        matching device names */
    PHYS_ADDR pDeviceAddresses[WD_PLATFORM_CARDS]; /**< Array of matching 
                                                device physical addresses */
} WDC_PLATFORM_SCAN_RESULT;
#endif /* defined(PLATFORM_API) */

#endif /* __KERNEL__ */

/** PCI capabilities scan results */
typedef struct {
    DWORD      dwNumCaps; /**< Number of matching PCI capabilities */
    WD_PCI_CAP pciCaps[WD_PCI_MAX_CAPS]; /**< Array of matching PCI
                                          * capabilities */
} WDC_PCI_SCAN_CAPS_RESULT;

/* Driver open options */
/* Basic driver open flags */
#define WDC_DRV_OPEN_CHECK_VER 0x1 /**< Compare source files WinDriver version
                                     with that of the running WinDriver kernel
                                    */
#define WDC_DRV_OPEN_REG_LIC   0x2 /**< Register WinDriver license */
#define WDC_DRV_OPEN_AGENT_DEFAULT_WAIT 0x10 /**< Wait for a short period until agent timeout */
#define WDC_DRV_OPEN_AGENT_LONG_WAIT 0x20    /**< Wait a longer time until agent timeout
                                            in license check (may slow down initialization) */
#define WDC_DRV_OPEN_AGENT_INFINITE_WAIT 0x40 /**< Wait infinite time until agent timeout
                                            in license check (may cause user app to hang) */
/* Convenient driver open options */
#define WDC_DRV_OPEN_BASIC     0x0 /**< No option -> perform only the basic
                                      open driver tasks, which are always
                                      performed by WDC_DriverOpen
                                      (mainly - open a handle to WinDriver) */
#define WDC_DRV_OPEN_KP WDC_DRV_OPEN_BASIC /**< Kernel PlugIn driver open
                                            * options <=> basic */
#define WDC_DRV_OPEN_ALL (WDC_DRV_OPEN_CHECK_VER | WDC_DRV_OPEN_REG_LIC | WDC_DRV_OPEN_AGENT_DEFAULT_WAIT)
#if defined(__KERNEL__)
    #define WDC_DRV_OPEN_DEFAULT WDC_DRV_OPEN_KP
#else
    #define WDC_DRV_OPEN_DEFAULT WDC_DRV_OPEN_CHECK_VER | WDC_DRV_OPEN_REG_LIC
#endif
typedef DWORD WDC_DRV_OPEN_OPTIONS;

/* Debug information display options */
#define WDC_DBG_OUT_DBM     0x1  /**< Send WDC debug messages to the
                                  * Debug Monitor */
#define WDC_DBG_OUT_FILE    0x2  /**< Send WDC debug messages to a debug file
                                  * (default: stderr) [User-mode only] */

#define WDC_DBG_LEVEL_ERR   0x10 /**< Display only error WDC debug messages */
#define WDC_DBG_LEVEL_TRACE 0x20 /**< Display error and trace
                                  * WDC debug messages */

#define WDC_DBG_NONE        0x100 /**< Do not print debug messages */

/** Convenient debug options combinations/defintions */
#define WDC_DBG_DEFAULT     (WDC_DBG_OUT_DBM | WDC_DBG_LEVEL_TRACE)

#define WDC_DBG_DBM_ERR   (WDC_DBG_OUT_DBM | WDC_DBG_LEVEL_ERR)
#define WDC_DBG_DBM_TRACE (WDC_DBG_OUT_DBM | WDC_DBG_LEVEL_TRACE)

#if !defined(__KERNEL__)
    #define WDC_DBG_FILE_ERR   (WDC_DBG_OUT_FILE | WDC_DBG_LEVEL_ERR)
    #define WDC_DBG_FILE_TRACE (WDC_DBG_OUT_FILE | WDC_DBG_LEVEL_TRACE)

    #define WDC_DBG_DBM_FILE_ERR   \
        (WDC_DBG_OUT_DBM | WDC_DBG_OUT_FILE | WDC_DBG_LEVEL_ERR)
    #define WDC_DBG_DBM_FILE_TRACE \
        (WDC_DBG_OUT_DBM | WDC_DBG_OUT_FILE | WDC_DBG_LEVEL_TRACE)

    #define WDC_DBG_FULL \
        (WDC_DBG_OUT_DBM | WDC_DBG_OUT_FILE | WDC_DBG_LEVEL_TRACE)
#else
    #define WDC_DBG_FULL (WDC_DBG_OUT_DBM | WDC_DBG_LEVEL_TRACE)
#endif

typedef DWORD WDC_DBG_OPTIONS;

/** Sleep options */
#define WDC_SLEEP_BUSY 0
#define WDC_SLEEP_NON_BUSY SLEEP_NON_BUSY
typedef DWORD WDC_SLEEP_OPTIONS;

/* -----------------------------------------------
    Memory / I/O / Registers
   ----------------------------------------------- */
   /** Address space information struct */
struct WDC_ADDR_DESC;

typedef enum {
    WDC_WRITE = 0x1,
    WDC_READ = 0x2,
    WDC_READ_WRITE = 0x4,
} WDC_DIRECTION;

/** Read/write address options */
typedef enum {
    WDC_ADDR_RW_DEFAULT = 0x0, /**< Default: memory resource - direct access;
                                * autoincrement on block transfers */
    WDC_ADDR_RW_NO_AUTOINC = 0x4 /**< Hold device address constant while
                                  * reading/writing a block */
} WDC_ADDR_RW_OPTIONS;

/* Memory/I/O address size and access mode definitions (size - in bytes) */
#define WDC_SIZE_8 ((DWORD)sizeof(BYTE))
#define WDC_SIZE_16 ((DWORD)sizeof(WORD))
#define WDC_SIZE_32 ((DWORD)sizeof(UINT32))
#define WDC_SIZE_64 ((DWORD)sizeof(UINT64))
typedef DWORD WDC_ADDR_SIZE;

typedef enum {
    WDC_MODE_8 = WDC_SIZE_8,
    WDC_MODE_16 = WDC_SIZE_16,
    WDC_MODE_32 = WDC_SIZE_32,
    WDC_MODE_64 = WDC_SIZE_64
} WDC_ADDR_MODE;

#define WDC_ADDR_MODE_TO_SIZE(mode) (UINT64)mode
#define WDC_ADDR_SIZE_TO_MODE(size) (((size) > WDC_SIZE_32) ? WDC_MODE_64 : \
    ((size) > WDC_SIZE_16) ? WDC_MODE_32 : \
    ((size) > WDC_SIZE_8) ? WDC_MODE_16 : WDC_MODE_8)

/** Device configuration space identifier (PCI configuration space) */
#define WDC_AD_CFG_SPACE 0xFF

/**************************************************************
  Function Prototypes
 **************************************************************/
/* -----------------------------------------------
    General
   ----------------------------------------------- */
/**
* Get a handle to WinDriver.
* When using only the WDC API, you do not need to get a handle to WinDriver,
* since the WDC library encapsulates this for you.
* This function enables you to get the WinDriver handles used by the
* WDC library so you can pass it to low-level WD_xxx API,
* if such APIs are used from your code.
* @return  Returns a handle to WinDriver's kernel module
*   (or INVALID_HANDLE_VALUE in case of a failure), which is required by
*   the basic WD_xxx WinDriver PCI/ISA API
*/
HANDLE DLLCALLCONV WDC_GetWDHandle(void);

/**
* Returns the device's user context information.
*   @param [in] hDev: Handle to a WDC device, returned by WDC_xxxDeviceOpen().
*
* @return  Pointer to the device's user context,
*          or NULL if no context has been set.
*/
PVOID DLLCALLCONV WDC_GetDevContext(_In_ WDC_DEVICE_HANDLE hDev);

/**
* Returns the device's bus type: WD_BUS_PCI, WD_BUS_ISA or WD_BUS_UNKNOWN.
*   @param [in] hDev: Handle to a WDC device, returned by WDC_xxxDeviceOpen().
*
* @return Returns the device's bus type.
*/
WD_BUS_TYPE DLLCALLCONV WDC_GetBusType(_In_ WDC_DEVICE_HANDLE hDev);

/**
* Delays execution for the specified duration of time (in microseconds).
* By default the function performs a busy sleep (consumes CPU cycles).
*   @param [in] dwMicroSecs: The number of microseconds to sleep.
*   @param [in] options:     Sleep options.
*
* @return Returns WD_STATUS_SUCCESS (0) on success,
*  or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_Sleep(_In_ DWORD dwMicroSecs,
    _In_ WDC_SLEEP_OPTIONS options);

/**
 *   Returns the version number of the WinDriver kernel module
 * used by the WDC library.
 *
 *   @param [out] pcVersion:   Pointer to a pre-allocated buffer to be
 *                     filled by the function with the driver's
 *                     version information string.
 *                     The size of the version string buffer must be at least
 *                     128 bytes (characters).
 *   @param [in] dwLen:       Length of sVersion. If length will be too short,
 *                     this function will fail.
 *   @param [out] pdwVersion: Pointer to a value indicating the version number
 *                            of the WinDriver kernel module used by the WDC
 *                            library
 *
 * @return
 *  Returns WD_STATUS_SUCCESS (0) on success,
 *  or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_Version(_Outptr_ CHAR *pcVersion, _In_ DWORD dwLen,
    _Outptr_ DWORD *pdwVersion);

/**
*  Opens and stores a handle to WinDriver's kernel module and initializes the
*  WDC library according to the open options passed to it.
*  This function should be called once before calling any other WDC API.
*   @param [in] openOptions: A mask of any of the supported open flags,
*                     which determines the initialization actions that will be
*                     performed by the function.
*   @param [in] pcLicense:    WinDriver license registration string.
*                     This argument is ignored if the WDC_DRV_OPEN_REG_LIC flag
*                     is not set in the openOptions argument.
*                     If this parameter is a NULL pointer or an empty string,
*                     the function will attempt to register the demo WinDriver
*                     evaluation license.
*                     Therefore, when evaluating WinDriver pass NULL as this
*                     parameter. After registering your WinDriver toolkit,
*                     modify the code to pass your
*                     WinDriver license registration string.
*
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*
* @remarks
*          This function is currently only supported from the user mode.
*          This function is supported only for Windows and Linux.
*
* @snippet highlevel_examples.c WDC_DriverOpen
*/
DWORD DLLCALLCONV WDC_DriverOpen(_In_ WDC_DRV_OPEN_OPTIONS openOptions,
    _In_ const CHAR *pcLicense);

/**
*   Closes the WDC WinDriver handle
*   (acquired and stored by a previous call to WDC_DriverOpen()) and
*   uninitializes the WDC library.
*   Every WDC_DriverOpen() call should have a matching WDC_DriverClose() call,
*   which should be issued when you no longer need to use the WDC library.
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*
* @snippet highlevel_examples.c WDC_DriverClose
*/
DWORD DLLCALLCONV WDC_DriverClose(void);

/** -----------------------------------------------
    Scan bus (PCI)
   ----------------------------------------------- */
#ifndef __KERNEL__
/**
*  Scans the PCI bus for all devices with the specified vendor and device ID
*  combination and returns information regarding the matching devices
*  that were found and their locations.
*  The function performs the scan by iterating through all possible PCI buses
*  on the host platform, then through all possible PCI slots, and then through
*  all possible PCI functions.
*
*  Please note that with a regular WinDriver license, this function will not
*  work on a computer that has over 100 devices connected to it. In order to
*  make it work on a computer with over 100 devices, you must obtain a server
*  license from sales@jungo.com.
*
*   @param [in] dwVendorId:     Vendor ID to search for,
*                        or 0 to search for all vendor IDs
*   @param [in] dwDeviceId:     Device ID to search for,
*                        or 0 to search for all device IDs
*   @param [out] pPciScanResult: A pointer to a structure that will be updated
*                               by the function with the results of
*                               the PCI bus scan
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_PciScanDevices(_In_ DWORD dwVendorId,
    _In_ DWORD dwDeviceId, _Outptr_ WDC_PCI_SCAN_RESULT *pPciScanResult);

/**
*  Scans the PCI bus for all devices with the specified vendor and device ID
*  combination and returns information regarding the matching devices that were
*  found and their locations. The function performs the scan by topology i.e
*  for each located bridge the function scans the connected devices and
*  functions reported by the bridge, and only then proceeds to scan the next
*  bridge.
*
*  Please note that with a regular WinDriver license, this function will not
*  work on a computer that has over 100 devices connected to it. In order to
*  make it work on a computer with over 100 devices, you must obtain a server
*  license from sales@jungo.com.
*
*   @param [in] dwVendorId:     Vendor ID to search for,
*                        or 0 to search for all vendor IDs
*   @param [in] dwDeviceId:     Device ID to search for,
*                        or 0 to search for all device IDs
*   @param [out] pPciScanResult: A pointer to a structure that will be updated
*                               by the function with the results of the PCI
*                               bus scan
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_PciScanDevicesByTopology(_In_ DWORD dwVendorId,
    _In_ DWORD dwDeviceId, _Outptr_ WDC_PCI_SCAN_RESULT *pPciScanResult);

/**
*  Scans the PCI bus for all devices with the specified vendor and device ID
*  combination that have been registered to work with WinDriver,
*  and returns information regarding the matching devices that were found and
*  their locations. The function performs the scan by iterating through all
*  possible PCI buses on the host platform, then through all possible PCI
*  slots, and then through all possible PCI functions.
*
*  Please note that with a regular WinDriver license, this function will not
*  work on a computer that has over 100 devices connected to it. In order to
*  make it work on a computer with over 100 devices, you must obtain a server
*  license from sales@jungo.com.
*
*   @param [in] dwVendorId:     Vendor ID to search for,
*                        or 0 to search for all vendor IDs
*   @param [in] dwDeviceId:     Device ID to search for,
*                        or 0 to search for all device IDs
*   @param [out] pPciScanResult: A pointer to a structure that will be updated
*                                by the function with the results of the
*                                PCI bus scan
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_PciScanRegisteredDevices(_In_ DWORD dwVendorId,
    _In_ DWORD dwDeviceId, _Outptr_ WDC_PCI_SCAN_RESULT *pPciScanResult);

#if defined(PLATFORM_API)
/**
* Scans the platform for devices matching a specific device name.
* Used primarily for non-PCI or custom bus devices.
*
* @note This function is available only if PLATFORM_API is defined.
*
*   @param [in] pcDeviceName:          The name of the device to search for, or
*       NULL to search for all devices
*   @param [in] pSearchAddr:           Physical address of the device to search
*       for, or 0 for all addresses
*   @param [out] pPlatformScanResult: A pointer to a structure that will be 
*       updated with the results of the platform scan
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
* or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_PlatformScanDevices(_In_ const CHAR *pcDeviceName, 
    _In_ PHYS_ADDR pSearchAddr,
    _Outptr_ WDC_PLATFORM_SCAN_RESULT *pPlatformScanResult);
#endif

/**
* A comprehensive scan function that can search for either PCI devices 
* (by ID) or platform devices (by name).
*
*   @param [in] dwVendorId:           Vendor ID to search for, or 0 for all.
*   @param [in] dwDeviceId:           Device ID to search for, or 0 for all.
*   @param [out] pPciScanResult:      Pointer to PCI scan results. 
*       Pass NULL if PCI scan is not required.
*   @param [in] pcDeviceName:          **[Only if PLATFORM_API is defined]** 
*       Device name to search for on the platform, or NULL for all.
*   @param [in] pSearchAddr:          **[Only if PLATFORM_API is defined]** 
*       Physical address to search for on the platform, or 0 for all.
*   @param [out] pPlatformScanResult: **[Only if PLATFORM_API is defined]** 
*       Pointer to platform scan results. Pass NULL if platform scan is not 
*       required.
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
* or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_ScanDevices(_In_ DWORD dwVendorId,
    _In_ DWORD dwDeviceId, _Outptr_ WDC_PCI_SCAN_RESULT *pPciScanResult
#if defined(PLATFORM_API)
    , _In_ const CHAR *pcDeviceName, _In_ PHYS_ADDR pSearchAddr,
    _Outptr_ WDC_PLATFORM_SCAN_RESULT *pPlatformScanResult
#endif
    );
#endif

/** -----------------------------------------------
    Scan PCI Capabilities
   ----------------------------------------------- */
/**
*  Retrieves the PCI Express configuration registers' offset in the device's
*  configuration space. This offset varies between devices and needs to be
*  added to any extended configuration space register's fixed address.
*  The fixed addresses were defined in the PCI Express Base Specification.
*
*   @param [in] hDev:       Handle to a WDC PCI device structure,
*                    returned by WDC_PciDeviceOpen()
*   @param [out] pdwOffset: Pointer to the DWORD where the offset value
*                     will be written.
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_PciGetExpressOffset(_In_ WDC_DEVICE_HANDLE hDev,
    _Outptr_ DWORD *pdwOffset);
/**
*  Retrieves the PCI device's configuration space header type.
*  The header type is hardware dependent and determines how the configuration
*  space is arranged.
*
*   @param [in] hDev:  Handle to a WDC PCI device structure,
*                      returned by WDC_PciDeviceOpen()
*   @param [out] pHeaderType: A pointer to the structure where the header type
*                       will be updated. WDC_PCI_HEADER_TYPE definition and
*                      available values can be seen in
*                       WinDriver/include/pci_regs.h
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_PciGetHeaderType(_In_ WDC_DEVICE_HANDLE hDev,
    _Outptr_ WDC_PCI_HEADER_TYPE *pHeaderType);

/**
*  Scans the basic PCI capabilities of the given device for the specified
*  capability (or for all capabilities).
*
*   @param [in] hDev:     Handle to a WDC PCI device structure,
*                         returned by WDC_PciDeviceOpen().
*   @param [in] dwCapId:  ID of the basic PCI capability for which to
*                         search, or WD_PCI_CAP_ID_ALL to search for all
*                         basic PCI capabilities.
*   @param [out] pScanCapsResult: A pointer to a structure that will be updated
*                         by the function with the results of the basic
*                         PCI capabilities scan.
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*/
DWORD DLLCALLCONV WDC_PciScanCaps(_In_ WDC_DEVICE_HANDLE hDev,
    _In_ DWORD dwCapId, _Outptr_ WDC_PCI_SCAN_CAPS_RESULT *pScanCapsResult);

/**
*  Scans the basic PCI capabilities of the given device for the specified
*  capability (or for all capabilities).
*
*   @param [in] pPciSlot:  Pointer to a PCI device location information
*                          structure, which can be acquired by calling
*                          WDC_PciScanDevices().
*   @param [in] dwCapId:   ID of the basic PCI capability for which to search,
*                          or WD_PCI_CAP_ID_ALL to search for all
*                          basic PCI capabilities.
*   @param [out] pScanCapsResult: A pointer to a structure that will be updated
*                          by the function with the results of the basic
*                          PCI capabilities scan.
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*/
DWORD DLLCALLCONV WDC_PciScanCapsBySlot(_In_ WD_PCI_SLOT *pPciSlot,
    _In_ DWORD dwCapId, _Outptr_ WDC_PCI_SCAN_CAPS_RESULT *pScanCapsResult);

/**
*  Scans the extended (PCI Express) PCI capabilities of the given device for
*  the specified capability (or for all capabilities).
*
*   @param [in] hDev:      Pointer to a PCI device location information
*                          structure, which can be acquired by calling
*                          WDC_PciScanDevices().
*   @param [in] dwCapId:   ID of the extended PCI capability for which
*                          to search, or WD_PCI_CAP_ID_ALL to search for all
*                          extended PCI capabilities.
*   @param [out] pScanCapsResult: A pointer to a structure that will be updated
*                          by the function with the results of the  extended
*                          (PCI Express) PCI capabilities scan.
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_PciScanExtCaps(_In_ WDC_DEVICE_HANDLE hDev,
    _In_ DWORD dwCapId, _Outptr_ WDC_PCI_SCAN_CAPS_RESULT *pScanCapsResult);

/**
*  Retrieves the PCI Express generation of a device
*
*   @param [in] pPciSlot:  Pointer to a PCI device location information
*   structure, which can be acquired by calling WDC_PciScanDevices()
*
* @return  Returns 0 if device is not a PCI Express device,
*   otherwise returns the PCI Express generation of the device;
*/
DWORD DLLCALLCONV WDC_PciGetExpressGenBySlot(_In_ WD_PCI_SLOT *pPciSlot);

/**
*  Retrieves the PCI Express generation of a device
*
*   @param [in] hDev:  Handle to a WDC PCI device structure,
*               returned by WDC_PciDeviceOpen()
*
* @return  Returns 0 if device is not a PCI Express device,
*   otherwise returns the PCI Express generation of the device;
*/
DWORD DLLCALLCONV WDC_PciGetExpressGen(_In_ WDC_DEVICE_HANDLE hDev);

/** -------------------------------------------------
    Get device's resources information (PCI)
   ------------------------------------------------- */
#ifndef __KERNEL__
/**
*  Retrieves a PCI device's resources information
*  (memory and I/O ranges and interrupt information).
*
*   @param [in,out] pDeviceInfo: Pointer to a PCI device information structure
*   @param [in] pciSlot:  pDeviceInfo.pciSlot
*   @param [out] Card:  pDeviceInfo.Card
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*
* @snippet highlevel_examples.c WDC_PciGetDeviceInfo
*/
DWORD DLLCALLCONV WDC_PciGetDeviceInfo(_Inout_ WD_PCI_CARD_INFO *pDeviceInfo);

#if defined(PLATFORM_API)
/**
* Retrieves platform-specific device information for a non-PCI or 
* custom bus device.
*
* @note This function is available only if PLATFORM_API is defined.
*
*   @param [in,out] pDeviceInfo: Pointer to a platform device information 
*      structure
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
* or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_PlatformGetDeviceInfo(
    _Inout_ WD_PLATFORM_CARD_INFO *pDeviceInfo);
#endif

/**
* A general-purpose function to retrieve device resources. It can handle either
* PCI devices or platform devices.
*
*   @param [in,out] pPciDeviceInfo: Pointer to a PCI device information 
*      structure. Pass NULL if PCI information is not required.
*   @param [in,out] pPlatformInfo: **[Only if PLATFORM_API is defined]** 
*      Pointer to a platform device information structure. Pass NULL if 
*      platform information is not required.
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
* or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_GetDeviceInfo(_Inout_ WD_PCI_CARD_INFO *pPciDeviceInfo
#if defined(PLATFORM_API)
    , _Inout_ WD_PLATFORM_CARD_INFO *pPlatformInfo
#endif
    );

#endif

/** -------------------------------------------------
    Control device's SR-IOV capability (PCIe)
   ------------------------------------------------- */
/** SR-IOV API functions are not part of the standard WinDriver API, and not
 * included in the standard version of WinDriver. The functions are part of
 * "WinDriver for Server" API and require "WinDriver for Server" license.
 * Note that "WinDriver for Server" APIs are included in WinDriver evaluation
 * version. */

/**
*  Enables SR-IOV for a supported device.
*
*   @param [in] hDev:     Handle to a Plug-and-Play WDC device,
*                         returned by WDC_PciDeviceOpen()
*   @param [in] dwNumVFs: The number of virtual functions (VFs) to be assigned
*                         to hDev
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*
* @snippet highlevel_examples.c WDC_PciSriovEnable
*/
DWORD DLLCALLCONV WDC_PciSriovEnable(_In_ WDC_DEVICE_HANDLE hDev,
    _In_ DWORD dwNumVFs);

/**
*  Disables SR-IOV for a supported device and removes all the assigned VFs.
*
*   @param [in] hDev:     Handle to a PlugandPlay WDC device,
*                  returned by WDC_PciDeviceOpen()
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*
* @snippet highlevel_examples.c WDC_PciSriovDisable
*/
DWORD DLLCALLCONV WDC_PciSriovDisable(_In_ WDC_DEVICE_HANDLE hDev);

/**
*  Gets the number of virtual functions assigned to a supported device.
*
*   @param [in] hDev:      Handle to a PlugandPlay WDC device,
*                   returned by WDC_PciDeviceOpen()
*   @param [out] pdwNumVFs: A pointer to a DWORD to store the number of VFs
*                           assigned
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*
* @snippet highlevel_examples.c WDC_PciSriovGetNumVFs
*/
DWORD DLLCALLCONV WDC_PciSriovGetNumVFs(_In_ WDC_DEVICE_HANDLE hDev,
    _Outptr_ DWORD *pdwNumVFs);

/** -----------------------------------------------
    Open/Close device
   ----------------------------------------------- */
#if !defined(__KERNEL__)    
/**
*  Allocates and initializes a WDC PCI device structure, registers the device
*  with WinDriver, and returns a handle to the device.
*  - Verifies that none of the registered device resources
     (set in pDeviceInfo->Card.Item) are already locked for exclusive use.
*  - Maps the physical memory ranges found on the device both to kernel-mode
*    and user-mode address space, and stores the mapped addresses in the
*    allocated device structure for future use.
*  - Saves device resources information required for supporting the
*    communication with the device. For example, the function saves the
*    interrupt request (IRQ) number and the interrupt type, as well as
*    retrieves and saves an interrupt handle, and this information is
*    later used when the user calls functions to handle the device's interrupts
*
*   @param [out] phDev:      Pointer to a handle to the WDC device allocated
*                     by the function
*   @param [in] pDeviceInfo: Pointer to a card information structure, which
*                     contains information regarding the device to open
*   @param [in] pDevCtx:     Pointer to device context information,
*                     which will be stored in the device structure
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*
* @snippet highlevel_examples.c WDC_PciDeviceOpen
*/
DWORD DLLCALLCONV WDC_PciDeviceOpen(_Outptr_ WDC_DEVICE_HANDLE *phDev,
    _In_ const WD_PCI_CARD_INFO *pDeviceInfo, _In_ const PVOID pDevCtx);

/**
*  Allocates and initializes a WDC ISA device structure, registers the device
*  with WinDriver, and returns a handle to the device.
*  - Verifies that none of the registered device resources
     (set in pDeviceInfo->Card.Item) are already locked for exclusive use.
*  - Maps the device's physical memory ranges device both to kernel-mode and
*    user-mode address space, and stores the mapped addresses in the allocated
*    device structure for future use
*  - Saves device resources information required for supporting the
*    communication with the device. For example, the function saves the
*    interrupt request (IRQ) number and the interrupt type,
*    as well as retrieves and saves an interrupt handle,
*    and this information is later used when the user calls functions
*    to handle the device's interrupts.
*  - If the caller selects to use a Kernel PlugIn driver to communicate with
*    the device, the function opens a handle to this driver and stores it
*    for future use.
*
*   @param [out] phDev:      Pointer to a handle to the WDC device allocated
*                     by the function
*   @param [in] pDeviceInfo: Pointer to a card information structure, which
*                     contains information regarding the device to open
*   @param [in] pDevCtx:     Pointer to device context information,
*                     which will be stored in the device structure
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_IsaDeviceOpen(_Outptr_ WDC_DEVICE_HANDLE *phDev,
    _In_ const WD_CARD *pDeviceInfo, _In_ const PVOID pDevCtx);

#if defined(PLATFORM_API)
/**
* Allocates and initializes a WDC platform device structure, registers 
* the device with WinDriver, and returns a handle. 
* Used for non-PCI/ISA or custom bus devices.
*
* @note This function is available only if PLATFORM_API is defined.
*
*   @param [out] phDev:       Pointer to a handle to the WDC device allocated
*       by the function
*   @param [in] pDeviceInfo:  Pointer to a platform card information structure
*   @param [in] pDevCtx:      Pointer to device context information,
*       to be stored in the device structure
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
* or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_PlatformDeviceOpen(_Outptr_ WDC_DEVICE_HANDLE *phDev,
    _In_ const WD_PLATFORM_CARD_INFO *pDeviceInfo, _In_ const PVOID pDevCtx);
#endif

/**
* A versatile function that opens a handle for either a PCI device, 
* a platform device.
*
*   @param [out] phDev:              Pointer to the resulting WDC device handle
*   @param [in] pPciDeviceInfo:      Pointer to PCI card information. 
*       Pass NULL if not opening a PCI device.
*   @param [in] pPlatformDeviceInfo: **[Only if PLATFORM_API is defined]** 
*       Pointer to platform card information. Pass NULL if not opening a 
*       platform device.
*   @param [in] pDevCtx:             Pointer to user-defined device context
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
* or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_DeviceOpen(_Outptr_ WDC_DEVICE_HANDLE *phDev,
    _In_ const WD_PCI_CARD_INFO *pPciDeviceInfo,
#if defined(PLATFORM_API)
    _In_ const WD_PLATFORM_CARD_INFO *pPlatformDeviceInfo,
#endif
    _In_ const PVOID pDevCtx);

/**
*  Uninitializes a WDC PCI device structure and frees
*  the memory allocated for it.
*
*   @param [in] hDev: Handle to a WDC PCI device structure,
*              returned by WDC_PciDeviceOpen()
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*
* @snippet highlevel_examples.c WDC_PciDeviceClose
*/
DWORD DLLCALLCONV WDC_PciDeviceClose(_In_ WDC_DEVICE_HANDLE hDev);

/**
*  Uninitializes a WDC ISA device structure and frees
*  the memory allocated for it.
*
*   @param [in] hDev: Handle to a WDC ISA device structure,
*              returned by WDC_IsaDeviceOpen()
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_IsaDeviceClose(_In_ WDC_DEVICE_HANDLE hDev);

/**
* Uninitializes a WDC device structure (PCI, ISA, or Platform) and 
* frees the memory allocated for it.
*
*   @param [in] hDev: Handle to a WDC device structure, 
*       returned by WDC_PciDeviceOpen(), WDC_IsaDeviceOpen(), 
*       WDC_PlatformDeviceOpen(), or WDC_DeviceOpen()
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
* or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_DeviceClose(_In_ WDC_DEVICE_HANDLE hDev);
#endif

/* -----------------------------------------------
    Reset PCI device
   ----------------------------------------------- */
/**
* Reset the PCI function without affecting other functions on the device.
* This operation performs a Function Level Reset (FLR), if supported by the
* device.
* Note: The device must support this capability; otherwise, the operation will
*       fail.
*
*   @param [in] hDev: Handle to a WDC PCI device structure,
*              returned by WDC_PciDeviceOpen()
*
* @return Returns WD_STATUS_SUCCESS (0) on success,
*  or an appropriate error code otherwise
*
*/
DWORD DLLCALLCONV WDC_PciResetDevice(_In_ WDC_DEVICE_HANDLE hDev);

/** -----------------------------------------------
    Set card cleanup commands
   ----------------------------------------------- */

/**
*  Sets a list of transfer cleanup commands to be performed for the specified
*  card on any of the following occasions:
*  - The application exits abnormally.
*  - The application exits normally but without closing the specified card.
*  - If the bForceCleanup parameter is set to TRUE, the cleanup commands
*    will also be performed when the specified card is closed.
*
*   @param [in] hDev:          Handle to a WDC device, returned by
*                       WDC_xxxDeviceOpen()
*   @param [in] pTransCmds:    Pointer to an array of cleanup transfer
*                       commands to be performed.
*   @param [in] dwCmds:        Number of cleanup commands in the Cmd array.
*   @param [in] fForceCleanup: If FALSE: The cleanup transfer commands (Cmd)
*                       will be performed in either of the following cases:
*                       When the application exits abnormally.
*                       When the application exits normally without closing
*                       the card by calling one of the WDC_xxxDeviceClose()
*                       functions
*
*                       If TRUE: The cleanup transfer commands will
*                       be performed both in the two cases described above,
*                       as well as in the following case:
*                       When the relevant WD_xxxDeviceClose() function
*                       is called for the card.
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD WDC_CardCleanupSetup(_In_ WDC_DEVICE_HANDLE hDev,
    _In_ WD_TRANSFER *pTransCmds, _In_ DWORD dwCmds, _In_ BOOL fForceCleanup);

/** -----------------------------------------------
    Open a handle to Kernel PlugIn driver
   ----------------------------------------------- */

/**
*  Opens a handle to a Kernel PlugIn driver.
*
*   @param [in] hDev:       Handle to a WDC device,
*                        returned by WDC_xxxDeviceOpen()
*   @param [in] pcKPDriverName: Kernel PlugIn driver name
*   @param [in] pKPOpenData:    Kernel PlugIn driver open data to be passed to
*                        WD_KernelPlugInOpen() (see the WinDriver PCI Low-Level
*                        API Reference)
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_KernelPlugInOpen(_In_ WDC_DEVICE_HANDLE hDev,
    _In_ const CHAR *pcKPDriverName, _In_ PVOID pKPOpenData);

/** -----------------------------------------------
    Send Kernel PlugIn messages
   ----------------------------------------------- */

/**
*  Sends a message from a user-mode application to a Kernel PlugIn driver.
*  The function passes a message ID from the application to the Kernel PlugIn's
*  KP_Call function, which should be implemented to
*  handle the specified message ID, and returns the result from the Kernel
*  PlugIn to the user-mode application.
*
*   @param [in] hDev:       Handle to a WDC device, returned by
*                           WDC_xxxDeviceOpen()
*   @param [in] dwMsg:      A message ID to pass to the Kernel PlugIn driver
*                    (specifically to KP_Call)
*   @param [in,out] pData:  Pointer to data to pass between the Kernel PlugIn
*                   driver and the user-mode application
*   @param [out] pdwResult: Result returned by the Kernel PlugIn driver
*                    (KP_Call) for the operation performed in the kernel as
*                    a result of the message that was sent
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_CallKerPlug(_In_ WDC_DEVICE_HANDLE hDev,
    _In_ DWORD dwMsg, _Inout_ PVOID pData, _Outptr_ PDWORD pdwResult);

/** -----------------------------------------------
    Read/Write memory and I/O addresses
   ----------------------------------------------- */
/** Direct memory read/write macros */

/** reads 1 byte (8 bits) from a specified memory address.
   The address is read directly in the calling context
   (user mode / kernel mode).*/
#define WDC_ReadMem8(addr, off) *(volatile BYTE *)((UPTR)(addr) + (UPTR)(off))

/** reads 2 byte (16 bits) from a specified memory address.
   The address is read directly in the calling context
   (user mode / kernel mode).*/
#define WDC_ReadMem16(addr, off) \
    *(volatile WORD *)((UPTR)(addr) + (UPTR)(off))

/** reads 4 byte (32 bits) from a specified memory address.
   The address is read directly in the calling context
   (user mode / kernel mode).*/
#define WDC_ReadMem32(addr, off) \
    *(volatile UINT32 *)((UPTR)(addr) + (UPTR)(off))

/** reads 8 byte (64 bits) from a specified memory address.
   The address is read directly in the calling context
   (user mode / kernel mode).*/
#define WDC_ReadMem64(addr, off) \
    *(volatile UINT64 *)((UPTR)(addr) + (UPTR)(off))


/** writes 1 byte (8 bits) to a specified memory address.
  The address is written to directly in the calling context
  (user mode / kernel mode)*/
#define WDC_WriteMem8(addr, off, val) \
    *(volatile BYTE * )(((UPTR)(addr) + (UPTR)(off))) = (val)

/** writes 2 byte (16 bits) to a specified memory address.
   The address is written to directly in the calling context
   (user mode / kernel mode)*/
#define WDC_WriteMem16(addr, off, val) \
    *(volatile WORD * )(((UPTR)(addr) + (UPTR)(off))) = (val)

/** writes 4 byte (32 bits) to a specified memory address.
   The address is written to directly in the calling context
   (user mode / kernel mode)*/
#define WDC_WriteMem32(addr, off, val) \
    *(volatile UINT32 *)(((UPTR)(addr) + (UPTR)(off))) = (val)

/** writes 8 byte (64 bits) to a specified memory address.
    The address is written to directly in the calling context
    (user mode / kernel mode)*/
#define WDC_WriteMem64(addr, off, val) \
    *(volatile UINT64 *)(((UPTR)(addr) + (UPTR)(off))) = (val)


/** Read/write a device's address space (8/16/32/64 bits) */

/**
*  reads 1 byte (8 bits) from a specified memory or I/O address.
*
*   @param [in] hDev:        Handle to a WDC device,
*                     returned by WDC_xxxDeviceOpen()
*   @param [in] dwAddrSpace: The memory or I/O address space to read from
*   @param [in] dwOffset:    The offset from the beginning of the specified
*                            address space (dwAddrSpace) to read from
*   @param [out] pbVal:      Pointer to a buffer to be filled with the data
*                     that is read from the specified address
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_ReadAddr8(_In_ WDC_DEVICE_HANDLE hDev,
    _In_ DWORD dwAddrSpace, _In_ KPTR dwOffset, _Outptr_ BYTE *pbVal);

/**
*  reads 2 byte (16 bits) from a specified memory or I/O address.
*
*   @param [in] hDev:        Handle to a WDC device,
*                     returned by WDC_xxxDeviceOpen()
*   @param [in] dwAddrSpace: The memory or I/O address space to read from
*   @param [in] dwOffset:    The offset from the beginning of the specified
*                     address space (dwAddrSpace) to read from
*   @param [out] pwVal:       Pointer to a buffer to be filled with the data
*                     that is read from the specified address
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_ReadAddr16(_In_ WDC_DEVICE_HANDLE hDev,
    _In_ DWORD dwAddrSpace, _In_ KPTR dwOffset, _Outptr_ WORD *pwVal);

/**
*  reads 4 byte (32 bits) from a specified memory or I/O address.
*
*   @param [in] hDev:        Handle to a WDC device,
*                     returned by WDC_xxxDeviceOpen()
*   @param [in] dwAddrSpace: The memory or I/O address space to read from
*   @param [in] dwOffset:    The offset from the beginning of the specified
*                     address space (dwAddrSpace) to read from
*   @param [out] pdwVal:      Pointer to a buffer to be filled with the data
*                     that is read from the specified address
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_ReadAddr32(_In_ WDC_DEVICE_HANDLE hDev,
    _In_ DWORD dwAddrSpace, _In_ KPTR dwOffset, _Outptr_ UINT32 *pdwVal);

/**
*  reads 8 byte (64 bits) from a specified memory or I/O address.
*
*   @param [in] hDev:        Handle to a WDC device,
*                     returned by WDC_xxxDeviceOpen()
*   @param [in] dwAddrSpace: The memory or I/O address space to read from
*   @param [in] dwOffset:    The offset from the beginning of the specified
*                     address space (dwAddrSpace) to read from
*   @param [out] pqwVal:     Pointer to a buffer to be filled with the data
*                     that is read from the specified address
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_ReadAddr64(_In_ WDC_DEVICE_HANDLE hDev,
    _In_ DWORD dwAddrSpace, _In_ KPTR dwOffset, _Outptr_ UINT64 *pqwVal);

/**
*  Reads 1 byte (8 bits) from a specified memory or I/O address.
*
*   @param [in] pAddrDesc:   Pointer to one of the device's address space
*                     information structures to read from
*   @param [in] dwOffset:    The offset from the beginning of the specified
*                     address space (dwAddrSpace) to read from
*   @param [out] pbVal:      Pointer to a buffer to be filled with the data
*                     that is read from the specified address
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_ReadAddrByAddrDesc8(_In_ struct WDC_ADDR_DESC *pAddrDesc,
    _In_ KPTR dwOffset, _In_ BYTE *pbVal);

/**
*  Reads 2 byte (16 bits) from a specified memory or I/O address.
*
*   @param [in] pAddrDesc:   Pointer to one of the device's address space
*                     information structures to read from
*   @param [in] dwOffset:    The offset from the beginning of the specified
*                     address space (dwAddrSpace) to read from
*   @param [out] pwVal:      Pointer to a buffer to be filled with the data
*                     that is read from the specified address
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_ReadAddrByAddrDesc16(_In_ struct WDC_ADDR_DESC *pAddrDesc,
    _In_ KPTR dwOffset, _In_ WORD *pwVal);

/**
*  Reads 4 byte (32 bits) from a specified memory or I/O address.
*
*   @param [in] pAddrDesc:   Pointer to one of the device's address space
*                     information structures to read from
*   @param [in] dwOffset:    The offset from the beginning of the specified
*                     address space (dwAddrSpace) to read from
*   @param [out] pdwVal:     Pointer to a buffer to be filled with the data
*                     that is read from the specified address
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_ReadAddrByAddrDesc32(_In_ struct WDC_ADDR_DESC *pAddrDesc,
    _In_ KPTR dwOffset, _In_ UINT32 *pdwVal);

/**
*  Reads 8 byte (64 bits) from a specified memory or I/O address.
*
*   @param [in] pAddrDesc:   Pointer to one of the device's address space
*                     information structures to read from
*   @param [in] dwOffset:    The offset from the beginning of the specified
*                     address space (dwAddrSpace) to read from
*   @param [out] pqwVal:     Pointer to a buffer to be filled with the data
*                     that is read from the specified address
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_ReadAddrByAddrDesc64(_In_ struct WDC_ADDR_DESC *pAddrDesc,
    _In_ KPTR dwOffset, _In_ UINT64 *pqwVal);

/**
*  writes 1 byte (8 bits) to a specified memory or I/O address.
*
*   @param [in] hDev:        Handle to a WDC device,
*                     returned by WDC_xxxDeviceOpen()
*   @param [in] dwAddrSpace: The memory or I/O address space to write to
*   @param [in] dwOffset:    The offset from the beginning of the specified
*                     address space (dwAddrSpace) to write to
*   @param [in] bVal:         The data to write to the specified address
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_WriteAddr8(_In_ WDC_DEVICE_HANDLE hDev,
    _In_ DWORD dwAddrSpace, _In_ KPTR dwOffset, _In_ BYTE bVal);

/**
*  writes 2 byte (16 bits) to a specified memory or I/O address.
*
*   @param [in] hDev:        Handle to a WDC device,
*                     returned by WDC_xxxDeviceOpen()
*   @param [in] dwAddrSpace: The memory or I/O address space to write to
*   @param [in] dwOffset:    The offset from the beginning of the specified
*                     address space (dwAddrSpace) to write to
*   @param [in] wVal:        The data to write to the specified address
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_WriteAddr16(_In_ WDC_DEVICE_HANDLE hDev,
    _In_ DWORD dwAddrSpace, _In_ KPTR dwOffset, _In_ WORD wVal);

/**
*  writes 4 byte (32 bits) to a specified memory or I/O address.
*
*   @param [in] hDev:        Handle to a WDC device,
*                     returned by WDC_xxxDeviceOpen()
*   @param [in] dwAddrSpace: The memory or I/O address space to write to
*   @param [in] dwOffset:    The offset from the beginning of the specified
*                     address space (dwAddrSpace) to write to
*   @param [in] dwVal:       The data to write to the specified address
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_WriteAddr32(_In_ WDC_DEVICE_HANDLE hDev,
    _In_ DWORD dwAddrSpace, _In_ KPTR dwOffset, _In_ UINT32 dwVal);

/**
*  writes 8 byte (64 bits) to a specified memory or I/O address.
*
*   @param [in] hDev:        Handle to a WDC device,
*                     returned by WDC_xxxDeviceOpen()
*   @param [in] dwAddrSpace: The memory or I/O address space to write to
*   @param [in] dwOffset:    The offset from the beginning of the specified
*                     address space (dwAddrSpace) to write to
*   @param [in] qwVal:       The data to write to the specified address
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_WriteAddr64(_In_ WDC_DEVICE_HANDLE hDev,
    _In_ DWORD dwAddrSpace, _In_ KPTR dwOffset, _In_ UINT64 qwVal);

/**
*  Writes 1 byte (8 bits) to a specified memory or I/O address.
*
*   @param [in] pAddrDesc:   Pointer to one of the device's address space
*                     information structures to read from
*   @param [in] dwOffset:    The offset from the beginning of the specified
*                     address space (dwAddrSpace) to write to
*   @param [in] bVal:        The data to write to the specified address
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_WriteAddrByAddrDesc8(_In_ struct WDC_ADDR_DESC *pAddrDesc,
    _In_ KPTR dwOffset, _In_ BYTE bVal);

/**
*  Writes 2 byte (16 bits) to a specified memory or I/O address.
*
*   @param [in] pAddrDesc:   Pointer to one of the device's address space
*                     information structures to read from
*   @param [in] dwOffset:    The offset from the beginning of the specified
*                     address space (dwAddrSpace) to write to
*   @param [in] wVal:        The data to write to the specified address
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_WriteAddrByAddrDesc16(_In_ struct WDC_ADDR_DESC *pAddrDesc,
    _In_ KPTR dwOffset, _In_ WORD wVal);

/**
*  Writes 4 byte (32 bits) to a specified memory or I/O address.
*
*   @param [in] pAddrDesc:   Pointer to one of the device's address space
*                     information structures to read from
*   @param [in] dwOffset:    The offset from the beginning of the specified
*                     address space (dwAddrSpace) to write to
*   @param [in] dwVal:       The data to write to the specified address
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_WriteAddrByAddrDesc32(_In_ struct WDC_ADDR_DESC *pAddrDesc,
    _In_ KPTR dwOffset, _In_ UINT32 dwVal);

/**
*  Writes 8 byte (64 bits) to a specified memory or I/O address.
*
*   @param [in] pAddrDesc:   Pointer to one of the device's address space
*                     information structures to read from
*   @param [in] dwOffset:    The offset from the beginning of the specified
*                     address space (dwAddrSpace) to write to
*   @param [in] qwVal:       The data to write to the specified address
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_WriteAddrByAddrDesc64(_In_ struct WDC_ADDR_DESC *pAddrDesc,
    _In_ KPTR dwOffset, _In_ UINT64 qwVal);


/**
*  Reads a block of data from the device.
*
*   @param [in] hDev:        Handle to a WDC device,
*                     returned by WDC_xxxDeviceOpen()
*   @param [in] dwAddrSpace: The memory or I/O address space to read from
*   @param [in] dwOffset:    The offset from the beginning of the specified
*                     address space (dwAddrSpace) to read from
*   @param [in] u64Bytes:     The number of bytes to read
*   @param [out] pData:      Pointer to a buffer to be filled with the data
*                     that is read from the device
*   @param [in] mode:        The read access mode -see WDC_ADDR_MODE
*   @param [in] options:     A bit mask that determines how the data will be
*                            read
*                     see WDC_ADDR_RW_OPTIONS.
*                     The function automatically sets the WDC_RW_BLOCK flag.
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_ReadAddrBlock(_In_ WDC_DEVICE_HANDLE hDev,
    _In_ DWORD dwAddrSpace,_In_ KPTR dwOffset, _In_ UINT64 u64Bytes,
    _Outptr_ PVOID pData, _In_ WDC_ADDR_MODE mode,
    _In_ WDC_ADDR_RW_OPTIONS options);

/**
*  Writes a block of data to the device.
*
*   @param [in] hDev:        Handle to a WDC device,
*                     returned by WDC_xxxDeviceOpen()
*   @param [in] dwAddrSpace: The memory or I/O address space to write to
*   @param [in] dwOffset:    The offset from the beginning of the specified
*                     address space (dwAddrSpace) to write to
*   @param [in] u64Bytes:     The number of bytes to write
*   @param [in] pData:       Pointer to a buffer that holds the data to write
*                     to the device
*   @param [in] mode:        The write access mode  -see WDC_ADDR_MODE
*   @param [in] options:     A bit mask that determines how the data will be
*                     written see WDC_ADDR_RW_OPTIONS.
*                     The function automatically sets the WDC_RW_BLOCK flag.
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_WriteAddrBlock(_In_ WDC_DEVICE_HANDLE hDev,
    _In_ DWORD dwAddrSpace, _In_ KPTR dwOffset, _In_ UINT64 u64Bytes,
    _In_ PVOID pData, _In_ WDC_ADDR_MODE mode,
    _In_ WDC_ADDR_RW_OPTIONS options);

/** WDC_ReadAddrBlock with 1 byte mode */
/* @snippet highlevel_examples.c WDC_ReadAddrBlock8 */
#define WDC_ReadAddrBlock8(hDev,dwAddrSpace,dwOffset,dwBytes,pData,options) \
    WDC_ReadAddrBlock(hDev, dwAddrSpace, dwOffset, dwBytes, pData, \
        WDC_MODE_8, options)

/** WDC_ReadAddrBlock with 2 bytes mode */
/* @snippet highlevel_examples.c WDC_ReadAddrBlock16 */
#define WDC_ReadAddrBlock16(hDev,dwAddrSpace,dwOffset,dwBytes,pData,options) \
    WDC_ReadAddrBlock(hDev, dwAddrSpace, dwOffset, dwBytes, pData, \
        WDC_MODE_16, options)

/** WDC_ReadAddrBlock with 4 bytes mode */
/* @snippet highlevel_examples.c WDC_ReadAddrBlock32 */
#define WDC_ReadAddrBlock32(hDev,dwAddrSpace,dwOffset,dwBytes,pData,options) \
    WDC_ReadAddrBlock(hDev, dwAddrSpace, dwOffset, dwBytes, pData, \
        WDC_MODE_32, options)

/** WDC_ReadAddrBlock with 8 bytes mode */
/* @snippet highlevel_examples.c WDC_ReadAddrBlock64 */
#define WDC_ReadAddrBlock64(hDev,dwAddrSpace,dwOffset,dwBytes,pData,options) \
    WDC_ReadAddrBlock(hDev, dwAddrSpace, dwOffset, dwBytes, pData, \
        WDC_MODE_64, options)

/** WDC_WriteAddrBlock with 1 byte mode */
/* @snippet highlevel_examples.c WDC_WriteAddrBlock8 */
#define WDC_WriteAddrBlock8(hDev,dwAddrSpace,dwOffset,dwBytes,pData,options) \
    WDC_WriteAddrBlock(hDev, dwAddrSpace, dwOffset, dwBytes, pData, \
        WDC_MODE_8, options)

/** WDC_WriteAddrBlock with 2 bytes mode */
/* @snippet highlevel_examples.c WDC_WriteAddrBlock16 */
#define WDC_WriteAddrBlock16(hDev,dwAddrSpace,dwOffset,dwBytes,pData,options) \
    WDC_WriteAddrBlock(hDev, dwAddrSpace, dwOffset, dwBytes, pData, \
        WDC_MODE_16, options)

/** WDC_WriteAddrBlock with 4 bytes mode */
/* @snippet highlevel_examples.c WDC_WriteAddrBlock32 */
#define WDC_WriteAddrBlock32(hDev,dwAddrSpace,dwOffset,dwBytes,pData,options) \
    WDC_WriteAddrBlock(hDev, dwAddrSpace, dwOffset, dwBytes, pData, \
        WDC_MODE_32, options)

/** WDC_WriteAddrBlock with 8 bytes mode */
/* @snippet highlevel_examples.c WDC_WriteAddrBlock64 */
#define WDC_WriteAddrBlock64(hDev,dwAddrSpace,dwOffset,dwBytes,pData,options) \
    WDC_WriteAddrBlock(hDev, dwAddrSpace, dwOffset, dwBytes, pData, \
        WDC_MODE_64, options)

/**
*  Performs a group of memory and/or I/O read/write transfers.
*
*   @param [in] pTransCmds: Pointer to an array of transfer commands
*                           information structures
*   @param [in] dwNumTrans: Number of transfer commands in the pTrans array
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_MultiTransfer(_In_ WD_TRANSFER *pTransCmds,
    _In_ DWORD dwNumTrans);

/**
*  Checks if the specified memory or I/O address space is active ,i.e.,
*  if its size is not zero.
*
*   @param [in] hDev:        Handle to a WDC device, returned by
*                            WDC_xxxDeviceOpen().
*   @param [in] dwAddrSpace: The memory or I/O address space to look for.
*
* @return  Returns TRUE if the specified address space is active;
*   otherwise returns FALSE.
*/
BOOL DLLCALLCONV WDC_AddrSpaceIsActive(_In_ WDC_DEVICE_HANDLE hDev,
    _In_ DWORD dwAddrSpace);

/** -----------------------------------------------
    Access PCI configuration space
   ----------------------------------------------- */
/** Read/write a block of any length from the PCI configuration space */
  /** Identify device by its location. */

/**
*  Reads data from a specified offset in a PCI device's configuration space or
*  a PCI Express device's extended configuration space.
*  The device is identified by its location on the PCI bus.
*
*   @param [in] pPciSlot: Pointer to a PCI device location information
*                  structure, which can be acquired by calling
*                  WDC_PciScanDevices()
*   @param [in] dwOffset: The offset from the beginning of the PCI
*                  configuration space to read from.
*   @param [out] pData:   Pointer to a buffer to be filled with the data that
*                  is read from the PCI configuration space.
*   @param [in] dwBytes:  The number of bytes to read.
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_PciReadCfgBySlot(_In_ WD_PCI_SLOT *pPciSlot,
    _In_ DWORD dwOffset, _Outptr_ PVOID pData, _In_ DWORD dwBytes);

/**
*  Write data to a specified offset in a PCI device's configuration space
*  or a PCI Express device's extended configuration space.
*  The device is identified by its location on the PCI bus.
*
*   @param [in] pPciSlot: Pointer to a PCI device location information
*                  structure, which can be acquired by calling
*                  WDC_PciScanDevices()
*   @param [in] dwOffset: The offset from the beginning of the PCI
*                  configuration space to write to.
*   @param [in] pData:    Pointer to a data buffer that holds the data to write
*   @param [in] dwBytes:  The number of bytes to write
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_PciWriteCfgBySlot(_In_ WD_PCI_SLOT *pPciSlot,
    _In_ DWORD dwOffset, _In_ PVOID pData, _In_ DWORD dwBytes);


/** Identify device by handle */

/**
*  Reads data from a specified offset in a PCI device's configuration space or
*  a PCI Express device's extended configuration space.
*
*   @param [in] hDev:     Handle to a WDC PCI device structure,
*                  returned by WDC_PciDeviceOpen()
*   @param [in] dwOffset: The offset from the beginning of the PCI
*                  configuration space to read from.
*   @param [out] pData:   Pointer to a buffer to be filled with the data that
*                  is read from the PCI configuration space
*   @param [in] dwBytes:  The number of bytes to read
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_PciReadCfg(_In_ WDC_DEVICE_HANDLE hDev,
    _In_ DWORD dwOffset, _Outptr_ PVOID pData, _In_ DWORD dwBytes);

/**
*  Writes data to a specified offset in a PCI device's configuration space or
*  a PCI Express device's extended configuration space.
*
*   @param [in] hDev:     Handle to a WDC PCI device structure,
*                  returned by WDC_PciDeviceOpen()
*   @param [in] dwOffset: The offset from the beginning of the PCI
*                  configuration space to write to.
*   @param [in] pData:    Pointer to a data buffer that holds the data to write
*   @param [in] dwBytes:  The number of bytes to write
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_PciWriteCfg(_In_ WDC_DEVICE_HANDLE hDev,
    _In_ DWORD dwOffset, _In_ PVOID pData, _In_ DWORD dwBytes);

/** Read/write 8/16/32/64 bits from the PCI configuration space.
   Identify device by its location. */

/**
*  Reads 1 byte (8 bits) from a specified offset in a PCI device's
*  configuration space or a PCI Express device's extended configuration space.
* The device is identified by its location on the PCI bus.
*
*   @param [in] pPciSlot: Pointer to a PCI device location information
*                  structure, which can be acquired by calling
*                  WDC_PciScanDevices()
*   @param [in] dwOffset: The offset from the beginning of the PCI
*                   configuration space to read from.
*   @param [out] pbVal:   Pointer to a buffer to be filled with the data that
*                  is read from the PCI configuration space
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_PciReadCfgBySlot8(_In_ WD_PCI_SLOT *pPciSlot,
     _In_ DWORD dwOffset, _Outptr_ BYTE *pbVal);

/**
*  Reads 2 bytes (16 bits) from a specified offset in a PCI device's
*  configuration space or a PCI Express device's extended configuration space.
* The device is identified by its location on the PCI bus.
*
*   @param [in] pPciSlot: Pointer to a PCI device location information
*                  structure,
*                  which can be acquired by calling WDC_PciScanDevices()
*   @param [in] dwOffset: The offset from the beginning of the PCI
*                  configuration space to read from.
*   @param [out] pwVal:   Pointer to a buffer to be filled with the data that
*                  is read from the PCI configuration space
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_PciReadCfgBySlot16(_In_ WD_PCI_SLOT *pPciSlot,
    _In_ DWORD dwOffset, _Outptr_ WORD *pwVal);

/**
*  Reads 4 bytes (32 bits) from a specified offset in a PCI device's
*  configuration space or a PCI Express device's extended configuration space.
* The device is identified by its location on the PCI bus.
*
*   @param [in] pPciSlot: Pointer to a PCI device location information
*                  structure, which can be acquired by calling
*                   WDC_PciScanDevices()
*   @param [in] dwOffset: The offset from the beginning of the PCI
*                  configuration space to read from.
*   @param [out] pdwVal:  Pointer to a buffer to be filled with the data
*                   that is read from the PCI configuration space
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_PciReadCfgBySlot32(_In_ WD_PCI_SLOT *pPciSlot,
    _In_ DWORD dwOffset, _Outptr_ UINT32 *pdwVal);

/**
*  Reads 8 bytes (64 bits) from a specified offset in a PCI device's
*  configuration space or a PCI Express device's extended configuration space.
* The device is identified by its location on the PCI bus.
*
*   @param [in] pPciSlot: Pointer to a PCI device location information
*                 structure, which can be acquired by calling
*                 WDC_PciScanDevices()
*   @param [in] dwOffset: The offset from the beginning of the PCI
*                  configuration space to read from.
*   @param [out] pqwVal:  Pointer to a buffer to be filled with the data
*                  that is read from the PCI configuration space
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_PciReadCfgBySlot64(_In_ WD_PCI_SLOT *pPciSlot,
    _In_ DWORD dwOffset, _Outptr_ UINT64 *pqwVal);

/**
*  writes 1 byte (8 bits) to a specified offset in a PCI device's
*  configuration space or a PCI Express device's extended configuration space.
*  The device is identified by its location on the PCI bus.
*
*   @param [in] pPciSlot: Pointer to a PCI device location information
*                  structure, which can be acquired by calling
*                  WDC_PciScanDevices()
*   @param [in] dwOffset: The offset from the beginning of the PCI
*                  configuration space to write to.
*   @param [in] bVal:     The data to write to the PCI configuration space
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_PciWriteCfgBySlot8(_In_ WD_PCI_SLOT *pPciSlot,
    _In_ DWORD dwOffset, _In_ BYTE bVal);

/**
*  writes 2 bytes (16 bits) to a specified offset in a PCI device's
*  configuration space or a PCI Express device's extended configuration space.
*  The device is identified by its location on the PCI bus.
*
*   @param [in] pPciSlot: Pointer to a PCI device location information
*                  structure, which can be acquired by calling
*                  WDC_PciScanDevices()
*   @param [in] dwOffset: The offset from the beginning of the PCI
*                  configuration space to write to.
*   @param [in] wVal:     The data to write to the PCI configuration space
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_PciWriteCfgBySlot16(_In_ WD_PCI_SLOT *pPciSlot,
    _In_ DWORD dwOffset, _In_ WORD wVal);

/**
*  writes 4 bytes (32 bits) to a specified offset in a PCI device's
*  configuration space or a PCI Express device's extended configuration space.
*  The device is identified by its location on the PCI bus.
*
*   @param [in] pPciSlot: Pointer to a PCI device location information
*                  structure, which can be acquired by calling
*                  WDC_PciScanDevices()
*   @param [in] dwOffset: The offset from the beginning of the PCI
*                  configuration
*                  space to write to.
*   @param [in] dwVal:    The data to write to the PCI configuration space
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_PciWriteCfgBySlot32(_In_ WD_PCI_SLOT *pPciSlot,
    _In_ DWORD dwOffset, _In_ UINT32 dwVal);

/**
*  writes 8 bytes (64 bits) to a specified offset in a PCI device's
*  configuration space or a PCI Express device's extended configuration space.
*  The device is identified by its location on the PCI bus.
*
*   @param [in] pPciSlot: Pointer to a PCI device location information
*                  structure, which can be acquired by calling
*                  WDC_PciScanDevices()
*   @param [in] dwOffset: The offset from the beginning of the PCI
*                  configuration space to write to.
*   @param [in] qwVal:    The data to write to the PCI configuration space
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_PciWriteCfgBySlot64(_In_ WD_PCI_SLOT *pPciSlot,
    _In_ DWORD dwOffset, _In_ UINT64 qwVal);

/** Read/write 8/16/32/64 bits from the PCI configuration space.
   Identify device by handle */

/**
*  Reads 1 byte (8 bits) from a specified offset in a PCI device's
*  configuration space or a PCI Express device's extended configuration space.
*
*   @param [in] hDev:     Handle to a WDC PCI device structure,
*                  returned by WDC_PciDeviceOpen()
*   @param [in] dwOffset: The offset from the beginning of the PCI
*                  configuration space to read from.
*   @param [out] pbVal:   Pointer to a buffer to be filled with the data that
*                  is read from the PCI configuration space
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_PciReadCfg8(_In_ WDC_DEVICE_HANDLE hDev,
    _In_ DWORD dwOffset, _Outptr_ BYTE *pbVal);

/**
*  Reads 2 bytes (16 bits) from a specified offset in a PCI device's
*  configuration space or a PCI Express device's extended configuration space.
*
*   @param [in] hDev:     Handle to a WDC PCI device structure,
*                  returned by WDC_PciDeviceOpen()
*   @param [in] dwOffset: The offset from the beginning of the PCI
*                  configuration space to read from.
*   @param [out] pwVal:   Pointer to a buffer to be filled with the data that
*                  is read from the PCI configuration space
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_PciReadCfg16(_In_ WDC_DEVICE_HANDLE hDev,
    _In_ DWORD dwOffset, _Outptr_ WORD *pwVal);

/**
*  Reads 4 bytes (32 bits) from a specified offset in a PCI device's
*  configuration space or a PCI Express device's extended configuration space.
*
*   @param [in] hDev:     Handle to a WDC PCI device structure,
*                  returned by WDC_PciDeviceOpen()
*   @param [in] dwOffset: The offset from the beginning of the PCI
*                  configuration space to read from.
*   @param [out] pdwVal:  Pointer to a buffer to be filled with the data
*                  that is read from the PCI configuration space
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_PciReadCfg32(_In_ WDC_DEVICE_HANDLE hDev,
    _In_ DWORD dwOffset, _Outptr_ UINT32 *pdwVal);

/**
*  Reads 8 bytes (64 bits) from a specified offset in a PCI device's
*  configuration space or a PCI Express device's extended configuration space.
*
*   @param [in] hDev:     Handle to a WDC PCI device structure,
*                  returned by WDC_PciDeviceOpen()
*   @param [in] dwOffset: The offset from the beginning of the PCI
*                  configuration space to read from.
*   @param [out] pqwVal:  Pointer to a buffer to be filled with the data
*                  that is read from the PCI configuration space
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_PciReadCfg64(_In_ WDC_DEVICE_HANDLE hDev,
    _In_ DWORD dwOffset, _Outptr_ UINT64 *pqwVal);

/**
*  Writes 1 byte (8 bits) to a specified offset in a PCI device's
*  configuration space or a PCI Express device's extended configuration space.
*
*   @param [in] hDev:     Handle to a WDC PCI device structure,
*                  returned by WDC_PciDeviceOpen()
*   @param [in] dwOffset: The offset from the beginning of the PCI
*                  configuration space to write to.
*   @param [in] bVal:     The data to write to the PCI configuration space
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_PciWriteCfg8(_In_ WDC_DEVICE_HANDLE hDev,
    _In_ DWORD dwOffset, _In_ BYTE bVal);

/**
*  Writes 2 bytes (16 bits) to a specified offset in a PCI device's
*  configuration space or a PCI Express device's extended configuration space.
*
*   @param [in] hDev:     Handle to a WDC PCI device structure,
*                  returned by WDC_PciDeviceOpen()
*   @param [in] dwOffset: The offset from the beginning of the PCI
*                  configuration space to write to.
*   @param [in] wVal:     The data to write to the PCI configuration space
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_PciWriteCfg16(_In_ WDC_DEVICE_HANDLE hDev,
    _In_ DWORD dwOffset, _In_ WORD wVal);

/**
*  Writes 4 bytes (32 bits) to a specified offset in a PCI device's
*  configuration space or a PCI Express device's extended configuration space.
*
*   @param [in] hDev:     Handle to a WDC PCI device structure,
*                  returned by WDC_PciDeviceOpen()
*   @param [in] dwOffset: The offset from the beginning of the PCI
*                  configuration space to write to.
*   @param [in] dwVal:    The data to write to the PCI configuration space
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_PciWriteCfg32(_In_ WDC_DEVICE_HANDLE hDev,
    _In_ DWORD dwOffset, _In_ UINT32 dwVal);

/**
*  Writes 8 bytes (64 bits) to a specified offset in a PCI device's
*  configuration space or a PCI Express device's extended configuration space.
*
*   @param [in] hDev:     Handle to a WDC PCI device structure,
*                  returned by WDC_PciDeviceOpen()
*   @param [in] dwOffset: The offset from the beginning of the PCI configuration
*                  space to write to.
*   @param [in] qwVal:    The data to write to the PCI configuration space
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*/
DWORD DLLCALLCONV WDC_PciWriteCfg64(_In_ WDC_DEVICE_HANDLE hDev,
    _In_ DWORD dwOffset, _In_ UINT64 qwVal);

/** -----------------------------------------------
    DMA (Direct Memory Access)
   ----------------------------------------------- */
#if !defined(__KERNEL__)
/**
*  Allocates a contiguous DMA buffer, locks it in physical memory, and returns
*  mappings of the allocated buffer to physical address space and to user-mode
*  and kernel virtual address spaces.
*
*   @param [in] hDev:        Handle to a WDC device, returned by
*                            WDC_xxxDeviceOpen()
*   @param [out] ppBuf:      Pointer to a pointer to be filled by the function
*                        with the user-mode mapped address of the allocated DMA
*                        buffer
*   @param [in] dwOptions:   A bit mask of any of the following flags
*                     (defined in an enumeration in windrvr.h):
*                        DMA_FROM_DEVICE: Synchronize the DMA buffer for
*                      transfers from the device to memory.
*                        DMA_TO_DEVICE: Synchronize the DMA buffer for
*                      transfers from memory to the device.
*                        DMA_TO_FROM_DEVICE: Synchronize the DMA buffer for
*                      transfers in both directions  i.e., from the device to
*                      memory and from memory to the device
*                      (<=> DMA_FROM_DEVICE | DMA_TO_DEVICE).
*                        DMA_ALLOW_CACHE: Allow caching of the memory.
*                        DMA_KBUF_BELOW_16M: Allocate the physical DMA buffer
*                      within the lower 16MB of the main memory.
*                        DMA_ALLOW_64BIT_ADDRESS: Allow allocation of
*                      64-bit DMA addresses.
*                        DMA_KBUF_ALLOC_SPECIFY_ADDRESS_WIDTH: When using this
*                      flag, the width of the address must be entered in the
*                      fourth byte of dwOptions and then the allocated address
*                      will be limited to this width. Linux: works with
*                      contiguous buffers only.
*                        DMA_GET_PREALLOCATED_BUFFERS_ONLY: Windows: Only
*                      preallocated buffer is allowed.
*   @param [in] dwDMABufSize: The size (in bytes) of the DMA buffer.
*   @param [out] ppDma:       Pointer to a pointer to a DMA buffer information
*                      structure, which is allocated by the function.
*                      The pointer to this structure (*ppDma) should be passed
*                      to WDC_DMABufUnlock() when the
*                      DMA buffer is no longer needed.
*
*
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*
* For more detailed usage of the WDC contiguous-buffer DMA implementaiton,
* please refer to @ref ch11_2_3_implementing_contiguous-buffer_dma
*/
DWORD DLLCALLCONV WDC_DMAContigBufLock(_In_ WDC_DEVICE_HANDLE hDev,
    _Outptr_ PVOID *ppBuf, _In_ DWORD dwOptions, _In_ DWORD dwDMABufSize,
    _Outptr_ WD_DMA **ppDma);

/**
*  Locks a pre-allocated user-mode memory buffer for DMA and returns the
*  corresponding physical mappings of the locked DMA pages. On Windows the
*  function also returns a kernel-mode mapping of the buffer.
*
*   @param [in] hDev:        Handle to a WDC device, returned by
*                            WDC_xxxDeviceOpen()
*   @param [in] pBuf:        Pointer to a user-mode buffer to be mapped to the
*                     allocated physical DMA buffer(s)
*   @param [in] dwOptions:   A bit mask of any of the following flags
*                     (defined in an enumeration in windrvr.h):
*                        DMA_FROM_DEVICE: Synchronize the DMA buffer for
*                      transfers from the device to memory.
*                        DMA_TO_DEVICE: Synchronize the DMA buffer for
*                      transfers from memory to the device.
*                        DMA_TO_FROM_DEVICE: Synchronize the DMA buffer for
*                      transfers in both directions i.e., from the device to
*                      memory and from memory to the device
*                      (<=> DMA_FROM_DEVICE | DMA_TO_DEVICE).
*                        DMA_ALLOW_CACHE: Allow caching of the memory.
*                        DMA_ALLOW_64BIT_ADDRESS: Allow allocation of
*                      64-bit DMA addresses.
*   @param [in] dwDMABufSize: The size (in bytes) of the DMA buffer.
*   @param [out] ppDma:       Pointer to a pointer to a DMA buffer information
*                      structure, which is allocated by the function.
*                      The pointer to this structure (*ppDma) should be passed
*                      to WDC_DMABufUnlock() when the
*                      DMA buffer is no longer needed.
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*
* For more detailed usage of the WDC Scatter/Gather DMA implementaiton,
* please refer to @ref ch11_2_2_implementing_scatter-gather_dma
*/
DWORD DLLCALLCONV WDC_DMASGBufLock(_In_ WDC_DEVICE_HANDLE hDev,
    _In_ PVOID pBuf, _In_ DWORD dwOptions, _In_ DWORD dwDMABufSize,
    _Outptr_ WD_DMA **ppDma);


typedef struct {
    WD_TRANSFER *pTransCmds;
    DWORD dwNumCmds;
    DWORD dwOptions;
    INT_HANDLER funcIntHandler;
    PVOID pData;
    BOOL fUseKP;
} WDC_INTERRUPT_PARAMS;

/**
*  Initializes the transaction, allocates a contiguous DMA buffer,
*  locks it in physical memory, and returns mappings of the allocated buffer
*  to physical address space and to user-mode and kernel
*  virtual address spaces.
*
*   @param [in] hDev:        Handle to a WDC device, returned by
*                            WDC_xxxDeviceOpen()
*   @param [out] ppBuf:      Pointer to a pointer to be filled by the function
*                    with the user-mode mapped address of the allocated DMA
*                    buffer.
*   @param [in] dwOptions:   A bit mask of any of the following flags
*                     (defined in an enumeration in windrvr.h):
*                       DMA_FROM_DEVICE: Synchronize the DMA buffer for
*                      transfers from the device to memory.
*                        DMA_TO_DEVICE: Synchronize the DMA buffer for
*                      transfers from memory to the device.
*                        DMA_TO_FROM_DEVICE: Synchronize the DMA buffer for
*                      transfers in both directions  i.e., from the device to
*                      memory and from memory to the device
*                      (<=> DMA_FROM_DEVICE | DMA_TO_DEVICE).
*                        DMA_ALLOW_CACHE: Enables caching for the allocated DMA 
*                      memory. If not set, memory is allocated as non-cached. 
*                      If set, memory is cached, and the transaction 
*                      automatically handles cache flushing and invalidation
*                      before and after the transfer. On Windows, this flag is 
*                      ignored, the memory is always cached and managed 
*                      automatically.
*                        DMA_ALLOW_64BIT_ADDRESS: Allow allocation of
*                        64-bit DMA addresses.
*   @param [in] dwDMABufSize:     The size (in bytes) of the DMA buffer.
*   @param [out] ppDma:    Pointer to a pointer to a DMA buffer
*                          information structur, which is allocated
*                          by the function. The pointer to this structure
*                          (*ppDma) should be passed to
*                          WDC_DMATransactionUninit() when the DMA
*                          buffer is no longer needed.
*   @param [in] pInterruptParams: WDC_DMATransactionContigInit() invokes
*                          WDC_IntEnable() with the relevant parameters
*                          from the structure (WDC_INTERRUPT_PARAMS). No action
*                          will be taken if this parameter is NULL.
*   @param [in] dwAlignment:      This value specifies the alignment
*                          requirement for the contiguous buffer.
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*
* For more detailed usage of the WDC_DMA Transactions functions,
* please refer to @ref ch11_3_performing_direct_memory_access_dma_transactions
*/
DWORD DLLCALLCONV WDC_DMATransactionContigInit(_In_ WDC_DEVICE_HANDLE hDev,
    _Outptr_ PVOID *ppBuf, _In_ DWORD dwOptions, _In_ DWORD dwDMABufSize,
    _Outptr_ WD_DMA **ppDma, _In_ WDC_INTERRUPT_PARAMS *pInterruptParams,
    _In_ DWORD dwAlignment);

/**
*  Initializes the transaction and locks a pre-allocated user-mode memory
*  buffer for DMA.
*
*   @param [in] hDev:       Handle to a WDC device, returned by
*                           WDC_xxxDeviceOpen()
*   @param [in] pBuf:       Pointer to a user-mode buffer to be mapped to the
*                    allocated physical DMA buffer(s).
*   @param [in] dwOptions:   A bit mask of any of the following flags
*                     (defined in an enumeration in windrvr.h):
*                        DMA_FROM_DEVICE: Synchronize the DMA buffer for
*                      transfers from the device to memory.
*                        DMA_TO_DEVICE: Synchronize the DMA buffer for
*                      transfers from memory to the device.
*                        DMA_TO_FROM_DEVICE: Synchronize the DMA buffer for
*                      transfers in both directions i.e., from the device to
*                      memory and from memory to the device
*                      (<=> DMA_FROM_DEVICE | DMA_TO_DEVICE).
*                        DMA_ALLOW_CACHE: Allow caching of the memory.
*                        DMA_ALLOW_64BIT_ADDRESS: Allow allocation of
*                        64-bit DMA addresses.
*   @param [in] dwDMABufSize:      The size (in bytes) of the DMA buffer.
*   @param [out] ppDma:            Pointer to a pointer to a DMA buffer
*                           information structure, which is allocated by the
*                           function. The pointer to this structure (*ppDma)
*                           should be passed to WDC_DMATransactionUninit() when
*                           the DMA buffer is no longer needed.
*   @param [in] pInterruptParams:   WDC_DMATransactionSGInit() invokes
*                           WDC_IntEnable() with the relevant parameters
*                           from the structure (WDC_INTERRUPT_PARAMS).
*                           No action will be taken if this parameter is NULL.
*   @param [in] dwMaxTransferSize: The maximum size for each of the transfers.
*   @param [in] dwTransferElementSize: The size (in bytes) of the DMA transfer
*                               element (descriptor).
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*
* For more detailed usage of the WDC_DMA Transactions functions,
* please refer to @ref ch11_3_performing_direct_memory_access_dma_transactions
*/
DWORD DLLCALLCONV WDC_DMATransactionSGInit(_In_ WDC_DEVICE_HANDLE hDev,
    _In_ PVOID pBuf, _In_ DWORD dwOptions, _In_ DWORD dwDMABufSize,
    _Outptr_ WD_DMA **ppDma, _In_ WDC_INTERRUPT_PARAMS *pInterruptParams,
    _In_ DWORD dwMaxTransferSize, _In_ DWORD dwTransferElementSize);

/**
*  Begins the execution of a specified DMA transaction.
*
*   @param [in,out] pDma:           Pointer to a DMA information
*                                   structure, received from a previous call to
*                                   WDC_DMATransactionContigInit()
*                                   (for a Contiguous DMA Buffer Transaction)
*                                   or WDC_DMATransactionSGInit()
*                                   (for a Scatter/Gather DMA buffer
*                                   Transaction)  *ppDma returned by these
*                                   functions
*
*   @param [in] funcDMATransactionCallback:    If the execution is completed
*                                   successfully, this callback function will
*                                   be called directly with
*                                   DMATransactionCallbackCtx as. context
*                                   No action will be taken if this
*                                   parameter is NULL.
*   @param [in] DMATransactionCallbackCtx: Pointer to a DMA transaction
*                                   callback context
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*
* For more detailed usage of the WDC_DMA Transactions functions,
* please refer to @ref ch11_3_performing_direct_memory_access_dma_transactions
*/
DWORD DLLCALLCONV WDC_DMATransactionExecute(_Inout_ WD_DMA *pDma,
    _In_ DMA_TRANSACTION_CALLBACK funcDMATransactionCallback,
    _In_ PVOID DMATransactionCallbackCtx);

/**
*  Notifies WinDriver that a device's DMA transfer operation is completed.
*
*   @param [in,out] pDma:     Pointer to a DMA information structure,
*                      received from a previous call to
*                      WDC_DMATransactionContigInit()
*                      (for a Contiguous DMA Buffer Transaction)
*                      or WDC_DMATransactionSGInit()
*                      (for a Scatter/Gather DMA buffer
*                      Transaction)  *ppDma returned by these
*                      functions
*
*   @param [in] fRunCallback: If this value is TRUE and the transaction is not
*                      over (there are more transfers to be made) the callback
*                      function that was previously provided as a callback to
*                      the WDC_DMATransactionExecute() function will be
*                      invoked. Otherwise, the callback function will not
*                      be invoked.
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*
* For more detailed usage of the WDC_DMA Transactions functions,
* please refer to @ref ch11_3_performing_direct_memory_access_dma_transactions
*/
DWORD DLLCALLCONV WDC_DMATransferCompletedAndCheck(_Inout_ WD_DMA *pDma,
    _In_ BOOL fRunCallback);

/**
*  Terminates a specified DMA transaction without deleting the associated
*  WD_DMA transaction structure.
*
*   @param [in] pDma: Pointer to a DMA information structure,
*              received from a previous call to
*              WDC_DMATransactionContigInit()
*              (for a Contiguous DMA Buffer Transaction)
*              or WDC_DMATransactionSGInit()
*              (for a Scatter/Gather DMA buffer
*              Transaction)  *ppDma returned by these functions
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*
* For more detailed usage of the WDC_DMA Transactions functions,
* please refer to @ref ch11_3_performing_direct_memory_access_dma_transactions
*/
DWORD DLLCALLCONV WDC_DMATransactionRelease(_In_ WD_DMA *pDma);

/**
*  Unlocks and frees the memory allocated for a DMA buffer transaction by a
*  previous call to WDC_DMATransactionContigInit()
*  or WDC_DMATransactionSGInit()
*
*   @param [in] pDma: Pointer to a DMA information structure,
*              received from a previous call to
*              WDC_DMATransactionContigInit()
*              (for a Contiguous DMA Buffer Transaction)
*              or WDC_DMATransactionSGInit()
*              (for a Scatter/Gather DMA buffer
*              Transaction)  *ppDma returned by these functions
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*
* For more detailed usage of the WDC_DMA Transactions functions,
* please refer to @ref ch11_3_performing_direct_memory_access_dma_transactions
*/
DWORD DLLCALLCONV WDC_DMATransactionUninit(_In_ WD_DMA *pDma);

/**
*  Locks a physical reserved memory buffer for DMA and returns the
*  corresponding user mode address of locked DMA buffer.
*
*   @param [in] hDev:        Handle to a WDC device, returned by
*                     WDC_xxxDeviceOpen()
*   @param [in] qwAddr:      Physical address of the reserved buffer to lock
*   @param [out] ppBuf:      Pointer to a pointer to be filled by the function
*                     with the user-mode mapped address of the locked DMA
*                     buffer
*   @param [in] dwOptions:   A bit mask of any of the following flags
*                     (defined in an enumeration in windrvr.h):
*                        DMA_FROM_DEVICE: Synchronize the DMA buffer for
*                        transfers from the device to memory.
*                        DMA_TO_DEVICE: Synchronize the DMA buffer for
*                        transfers from memory to the device.
*                        DMA_TO_FROM_DEVICE: Synchronize the DMA buffer for
*                        transfers in both directions i.e., from the device to
*                        memory and from memory to the device
*                        (<=> DMA_FROM_DEVICE | DMA_TO_DEVICE).
*                        DMA_ALLOW_64BIT_ADDRESS: Allow allocation of 64-bit
*                      DMA addresses. This flag is supported on Windows and
*                      Linux.
*   @param [in] dwDMABufSize: The size (in bytes) of the DMA buffer
*   @param [out] ppDma:       Pointer to a pointer to a DMA buffer information
*                      structure, which is allocated by the function.
*                      The pointer to this structure (*ppDma) should be passed
*                      to WDC_DMABufUnlock() when the
*                      DMA buffer is no longer needed.
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*
* For more detailed usage of the WDC_DMA functions, please refer to 
* @ref ch11_2_performing_direct_memory_access_dma
*/
DWORD DLLCALLCONV WDC_DMAReservedBufLock(_In_ WDC_DEVICE_HANDLE hDev,
    _In_ PHYS_ADDR qwAddr, _Outptr_ PVOID *ppBuf, _In_ DWORD dwOptions,
    _In_ DWORD dwDMABufSize, _Outptr_ WD_DMA **ppDma);


/**
*  Unlocks and frees the memory allocated for a DMA buffer by a previous
*  call to WDC_DMAContigBufLock(), WDC_DMASGBufLock() or
*  WDC_DMAReservedBufLock()
*
*   @param [in] pDma: Pointer to a DMA information structure,
*              received from a previous call to
*              WDC_DMATransactionContigInit()
*              (for a Contiguous DMA Buffer Transaction)
*              or WDC_DMATransactionSGInit()
*              (for a Scatter/Gather DMA buffer
*              Transaction)  *ppDma returned by these functions
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*
* For more detailed usage of the WDC_DMA functions,
* please refer to @ref ch11_2_performing_direct_memory_access_dma
*/
DWORD DLLCALLCONV WDC_DMABufUnlock(_In_ WD_DMA *pDma);

/** Utility macro that returns a contiguous DMA global handle that can be used
   for buffer sharing between multiple processes. */
#define WDC_DMAGetGlobalHandle(pDma) ((pDma)->hDma)

/**
*  Retrieves a contiguous DMA buffer which was allocated by another process.
*
*   @param [in] hDma:   DMA buffer handle.
*   @param [out] ppDma: Pointer to a pointer to a DMA buffer information
*                structure, which is associated with hDma.
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*
* For more detailed usage of the WDC_DMA functions,
* please refer to @ref ch11_2_performing_direct_memory_access_dma
*
* @snippet highlevel_examples.c WDC_DMABufGet
*/
DWORD DLLCALLCONV WDC_DMABufGet(_In_ DWORD hDma, _Outptr_ WD_DMA **ppDma);

/**
*  Synchronizes the cache of all CPUs with the DMA buffer,
*  by flushing the data from the CPU caches.
*
*   @param [in] pDma: Pointer to a DMA information structure,
*              received from a previous call to
*              WDC_DMATransactionContigInit()
*              (for a Contiguous DMA Buffer Transaction)
*              or WDC_DMATransactionSGInit()
*              (for a Scatter/Gather DMA buffer
*              Transaction)  *ppDma returned by these functions
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*
* For more detailed usage of the WDC_DMA functions,
* please refer to @ref ch11_2_performing_direct_memory_access_dma
*/
DWORD DLLCALLCONV WDC_DMASyncCpu(_In_ WD_DMA *pDma);

/**
*  Synchronizes the I/O caches with the DMA buffer, by flushing the data
*  from the I/O caches and updating the CPU caches.
*
*   @param [in] pDma: Pointer to a DMA information structure,
*              received from a previous call to
*              WDC_DMATransactionContigInit()
*              (for a Contiguous DMA Buffer Transaction)
*              or WDC_DMATransactionSGInit()
*              (for a Scatter/Gather DMA buffer
*              Transaction)  *ppDma returned by these functions
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*
* For more detailed usage of the WDC_DMA functions,
* please refer to @ref ch11_2_performing_direct_memory_access_dma
*/
DWORD DLLCALLCONV WDC_DMASyncIo(_In_ WD_DMA *pDma);
#endif
/* -----------------------------------------------
    Interrupts
   ----------------------------------------------- */
#if !defined(__KERNEL__)
/**
*  Enables interrupt handling for the device.
*
*  On Linux and Windows, when attempting to enable interrupts
*  for a PCI device that supports Extended Message-Signaled Interrupts (MSI-X)
*  or Message-Signaled Interrupts (MSI) (and was installed with a relevant INF
*  file on Windows), the function first tries to enable MSI-X or MSI;
*  if this fails, or if the target OS does not support MSI/MSI-X, the function
*  attempts to enable legacy level-sensitive interrupts
*  (if supported by the device).
*  On Linux, you can use the function's dwOptions parameter to specify the
*  types of PCI interrupts that may be enabled for the device
*  (see the explanation in the parameter description).
*  For other types of hardware (PCI with no MSI/MSI-X support / ISA),
*  the function attempts to enable the legacy interrupt type supported by the
*  device (Level Sensitive / Edge Triggered)
*
*  If the caller selects to handle the interrupts in the kernel,
*  using a Kernel PlugIn driver, the Kernel PlugIn KP_IntAtIrql
*  (legacy interrupts) or KP_IntAtIrqlMSI (MSI/MSI-X) function,
*  which runs at high interrupt request level (IRQL), will be invoked
*  immediately when an interrupt is received.
*
*  The function can receive transfer commands information, which will be
*  performed by WinDriver at the kernel, at high IRQ level, when an interrupt
*  is received. If a Kernel PlugIn driver is used to handle the interrupts,
*  any transfer commands set by the caller will be executed by WinDriver after
*  the Kernel PlugIn KP_IntAtDpc or KP_IntAtDpcMSI function completes
*  its execution.
*
*  When handling level-sensitive interrupts (such as legacy PCI interrupts)
*  from the user mode, without a Kernel PlugIn driver,
*  you must prepare and pass to the function transfer commands for
*  acknowledging the interrupt. When using a Kernel PlugIn driver,
*  the information for acknowledging the interrupts should be implemented
*  in the Kernel PlugIn KP_IntAtIrql function, so the transfer commands in
*  the call to WDC_IntEnable() are not required
*  (although they can still be used).
*
*  The function receives a user-mode interrupt handler routine, which will be
*  called by WinDriver after the kernel-mode interrupt processing is completed.
*  If the interrupts are handled using a Kernel PlugIn driver, the return value
*  of the Kernel PlugIn deferred interrupt handler function KP_IntAtDpc
*  (legacy interrupts) or KP_IntAtDpcMSI (MSI/MSI-X) will determine
*  how many times (if at all) the user-mode interrupt handler will be called
*  (provided KP_IntAtDpc or KP_IntAtDpcMSI itself is executed
*  which is determined by the return value of the Kernel PlugIn KP_IntAtIrql or
*  KP_IntAtIrqlMSI function).
*
*   @param [in] hDev:    Handle to a WDC device, returned by
*                        WDC_xxxDeviceOpen()
*   @param [in] pTransCmds:     An array of transfer commands information
*                        structures that define the operations to be performed
*                        at the kernel level upon the detection of an
*                        interrupt, or NULL if no transfer commands are
*                        required.
*                        - NOTE:
*                          Memory allocated for the transfer commands must
*                          remain available until the interrupts are disabled
*                          When handling level-sensitive interrupts
*                          (such as legacy PCI interrupts) without a
*                          Kernel PlugIn, you must use this array
*                          to define the hardware-specific commands for
*                          acknowledging the interrupts in the kernel,
*                          immediately when they are received.
*                        - For an explanation on how to set the transfer
*                        commands, refer to the description of WD_TRANSFER
*                        in Section B.7.10, and to the explanation in Section
*                        9.3.6.
*   @param [in] dwNumCmds:      Number of transfer commands in the pTransCmds
*                               array
*   @param [in] dwOptions:      A bit mask of interrupt handling flags can be
*                               set
*                        to zero for no options, or to a combination of any
*                        of the following flags:
*                        INTERRUPT_CMD_COPY: If set, WinDriver will copy any
*                        data read in the kernel as a result of a read transfer
*                        command, and return it to the user within the relevant
*                        transfer command structure.
*                        The user will be able to access the data from his
*                        user-mode interrupt handler routine (funcIntHandler).
*                        - The following flags are applicable only to PCI
*                        interrupts on Linux. If set, these flags determine
*                        the types of interrupts that may be enabled for the
*                        device the function will attempt to enable only
*                        interrupts of the specified types, using the following
*                        precedence order, provided the type is reported as
*                        supported by the device:
*                        - INTERRUPT_MESSAGE_X: Extended Message-Signaled
*                        Interrupts (MSI-X)
*                        - INTERRUPT_MESSAGE: Message-Signaled Interrupts (MSI)
*                        - INTERRUPT_LEVEL_SENSITIVE Legacy level-sensitive
*                        interrupts
*                        - INTERRUPT_DONT_GET_MSI_MESSAGE: Do not read the msi
*                        message from the card.
*   @param [in] funcIntHandler: A user-mode interrupt handler callback
*                        function, which will be executed after an interrupt is
*                        received and processed in the kernel.
*                        (The prototype of the interrupt handler
*                        INT_HANDLER is defined in windrvr_int_thread.h).
*   @param [in] pData:   Data for the user-mode interrupt handler callback
*                        routine (funcIntHandler)
*   @param [in] fUseKP:  If TRUE The device's Kernel PlugIn driver's
*                        KP_IntAtIrql or KP_IntAtIrqlMSI function, which
*                        runs at high interrupt request level (IRQL), will be
*                        executed immediately when an interrupt is received.
*                        The Kernel PlugIn driver to be used for the device should
*                        be opened with WDC_KernelPlugInOpen() with the same
*                        device handle (hDev) used as a parameter to this function.
*                        If the caller also passes transfer commands to the
*                        function (pTransCmds), these commands will be executed
*                        by WinDriver at the kernel, at high IRQ level, after
*                        KP_IntAtIrql or KP_IntAtIrqlMSI completes its
*                        execution.
*                        If the high-IRQL handler returns TRUE, the Kernel
*                        PlugIn deferred interrupt processing routine
*                        KP_IntAtDpc or KP_IntAtDpcMSI will be invoked.
*                        The return value of this function determines how many
*                        times (if at all) the user-mode interrupt handler
*                        (funcIntHandler) will be executed once the control
*                        returns to the user mode.
*                        If FALSE When an interrupt is received, any transfer
*                        commands set by the user in pTransCmds will be
*                        executed by WinDriver at the kernel, at high IRQ level
*                        and the user-mode interrupt handler routine
*                        (funcIntHandler) will be executed when the control
*                        returns to the user mode.
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*
* For more detailed information about interrupt handling,
* please refer to @ref ch10_1_handling_interrupts
*
* For a sample user-mode interrupt handling code,
* please refer to @ref ch10_1_8_sample_user-mode_windriver_interrupt_handling_code
*/
DWORD DLLCALLCONV WDC_IntEnable(_In_ WDC_DEVICE_HANDLE hDev,
    _In_ WD_TRANSFER *pTransCmds, _In_ DWORD dwNumCmds, _In_ DWORD dwOptions,
    _In_ INT_HANDLER funcIntHandler, _In_ PVOID pData, _In_ BOOL fUseKP);

/**
*  Disables interrupt interrupt handling for the device,
*  pursuant to a previous call to WDC_IntEnable()
*
*   @param [in] hDev: Handle to a WDC device, returned by WDC_xxxDeviceOpen()
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*
* For more detailed information about interrupt handling,
* please refer to @ref ch10_1_handling_interrupts
*
* For a sample user-mode interrupt handling code,
* please refer to @ref ch10_1_8_sample_user-mode_windriver_interrupt_handling_code
*/
DWORD DLLCALLCONV WDC_IntDisable(_In_ WDC_DEVICE_HANDLE hDev);

#endif

/**
*  Checks if a device's interrupts are currently enabled.
*
*   @param [in] hDev: Handle to a WDC device, returned by WDC_xxxDeviceOpen()
*
* @return  Returns TRUE if the device's interrupts are enabled;
*   otherwise returns FALSE.
*
* For more detailed information about interrupt handling,
* please refer to @ref ch10_1_handling_interrupts
*
* For a sample user-mode interrupt handling code,
* please refer to @ref ch10_1_8_sample_user-mode_windriver_interrupt_handling_code
*/
BOOL DLLCALLCONV WDC_IntIsEnabled(_In_ WDC_DEVICE_HANDLE hDev);

/**
*  Converts interrupt type to string.
*
*   @param [in] dwIntType: Interrupt types bit-mask
*
* @return  Returns the string representation that corresponds to the
*   specified numeric code.
*
* For more detailed information about interrupt handling,
* please refer to @ref ch10_1_handling_interrupts
*
* For a sample user-mode interrupt handling code,
* please refer to @ref ch10_1_8_sample_user-mode_windriver_interrupt_handling_code
*/
const CHAR * DLLCALLCONV WDC_IntType2Str(_In_ DWORD dwIntType);

/** -----------------------------------------------
    Plug-and-play and power management events
   ----------------------------------------------- */

#if !defined(__KERNEL__)
/**
*  Registers the application to receive Plug-and-Play and power management
*  events notifications for the device.
*
*   @param [in] hDev:             Handle to a Plug-and-Play WDC device,
*                          returned by WDC_PciDeviceOpen()
*   @param [in] dwActions:        A bit mask of flags indicating which events
*                          to register to:
*                          Plug-and-Play events:
*                               WD_INSERT Device inserted
*                               WD_REMOVE Device removed
*                          Device power state change events:
*                               WD_POWER_CHANGED_D0 Full power
*                               WD_POWER_CHANGED_D1 Low sleep
*                               WD_POWER_CHANGED_D2 Medium sleep
*                               WD_POWER_CHANGED_D3 Full sleep
*                               WD_POWER_SYSTEM_WORKING Fully on
*                          Systems power state:
*                               WD_POWER_SYSTEM_SLEEPING1 Fully on but sleeping
*                               WD_POWER_SYSTEM_SLEEPING2 CPU off, memory on,
*                             PCI on
*                               WD_POWER_SYSTEM_SLEEPING3 CPU off,
*                             Memory is in refresh, PCI on aux power
*                               WD_POWER_SYSTEM_HIBERNATE OS saves context
*                             before shutdown
*                               WD_POWER_SYSTEM_SHUTDOWN No context saved
*   @param [in] funcEventHandler: A user-mode event handler callback function,
*                          which will be called when an event for which
*                          the caller registered to receive notifications
*                          (see dwActions) occurs.
*                          (The prototype of the event handler
*                              EVENT_HANDLER is defined in windrvr_events.h.)
*   @param [in] pData:            Data for the user-mode event handler callback
*                          routine (funcEventHandler)
*   @param [in] fUseKP:           If TRUE When an event for which the caller
*                          registered to receive notifications (dwActions)
*                          occurs, the device's Kernel PlugIn driver's KP_Event
*                          function will be called. (The Kernel PlugIn driver
*                          to be used for the device is passed to
*                          WDC_xxxDeviceOpen() and stored in the
*                          WDC device structure).
*                          If this function returns TRUE, the user-mode events
*                          handler callback function (funcEventHandler) will be
*                          called when the kernel-mode event processing
*                          is completed.
*                          If FALSE When an event for which the caller
*                          registered to receive notifications (dwActions)
*                          occurs, the user-mode events handler callback
*                          function will be called.
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise.
*
* @snippet highlevel_examples.c WDC_EventRegister
*/
DWORD DLLCALLCONV WDC_EventRegister(_In_ WDC_DEVICE_HANDLE hDev,
    _In_ DWORD dwActions, _In_ EVENT_HANDLER funcEventHandler,
    _In_ PVOID pData, _In_ BOOL fUseKP);

/**
*  Unregisters an application from a receiving Plug-and-Play and power
*  management notifications for a device, pursuant to a previous call
*  to WDC_EventRegister()
*
*   @param [in] hDev: Handle to a Plug-and-Play WDC device,
*              returned by WDC_PciDeviceOpen()
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*
* @snippet highlevel_examples.c WDC_EventUnregister
*/
DWORD DLLCALLCONV WDC_EventUnregister(_In_ WDC_DEVICE_HANDLE hDev);

#endif

/**
*  Checks if the application is currently registered to receive Plug-and-Play
*  and power management notifications for the device.
*
*   @param [in] hDev: Handle to a Plug-and-Play WDC device,
*              returned by WDC_PciDeviceOpen()
*
* @return  Returns TRUE if the application is currently registeredto receive
*   Plug-and-Play and power management notifications for the device;
*   otherwise returns FALSE.
*
* @snippet highlevel_examples.c WDC_EventIsRegistered
*/
BOOL DLLCALLCONV WDC_EventIsRegistered(_In_ WDC_DEVICE_HANDLE hDev);

/** -----------------------------------------------
    Debugging and error handling
   ----------------------------------------------- */

/**
*  Sets debug options for the WDC library - see the description of
*  WDC_DBG_OPTIONS for details regarding the possible debug options to set.
*
*  This function is typically called at the beginning of the application,
*  after the call to WDC_DriverOpen(), and can be re-called at any time
*  while the WDC library is in use (i.e, WDC_DriverClose() has not been called)
*  in order to change the debug settings.
*
*  Until the function is called, the WDC library uses the default debug options
*  - see WDC_DBG_DEFAULT.
*
*  When the function is recalled, it performs any required cleanup for the
*  previous debug settings and sets the default debug options before
*  attempting to set the new options specified by the caller.
*
*   @param [in] dbgOptions: A bit mask of flags indicating the desired debug
*                    settings - see WDC_DBG_OPTIONS.
*                    If this parameter is set to zero,
*                    the default debug options will be used
*                     - see WDC_DBG_DEFAULT
*   @param [in] pcDbgFile:  WDC debug output file.
*                   This parameter is relevant only if the WDC_DBG_OUT_FILE
*                   flag is set in the debug options (dbgOptions)
*                   (either directly or via one of the convenience debug
*                   options combinations
*                     - see WDC_DBG_OPTIONS).
*                   If the WDC_DBG_OUT_FILE debug flag is set and sDbgFile is
*                   NULL, WDC debug messages will be logged to the default
*                   debug file stderr.
*
* @return  Returns WD_STATUS_SUCCESS (0) on success,
*   or an appropriate error code otherwise
*
* @snippet highlevel_examples.c WDC_SetDebugOptions
*/
DWORD DLLCALLCONV WDC_SetDebugOptions(_In_ WDC_DBG_OPTIONS dbgOptions,
    _In_ const CHAR *pcDbgFile);

/**
*  Displays debug error messages according to the WDC debug options
*     - see WDC_DBG_OPTIONS and WDC_SetDebugOptions()
*
*   @param [in] format:   Format-control string, which contains the error
*                  message to display. The string is limited to 256 characters
*                  (CHAR)
*   @param [in] ...: Optional arguments for the format string
*
* @return  None
*/
void DLLCALLCONV WDC_Err(const CHAR *format, ...);

/**
*  Displays debug trace messages according to the WDC debug options
*     - see WDC_DBG_OPTIONS and WDC_SetDebugOptions()
*
*   @param [in] format: Format-control string, which contains the error message
*                  to display. The string is limited to 256 characters (CHAR)
*   @param [in] ...: Optional arguments for the format string
*
* @return  None
*/
void DLLCALLCONV WDC_Trace(const CHAR *format, ...);

#ifdef __cplusplus
}
#endif

#endif /* _WDC_LIB_H_ */

