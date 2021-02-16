/*******************************************************
 HIDAPII - Multi-Platform library for
 communication with HID devices.

 Alan Ott
 Signal 11 Software

 8/22/2009

 Copyright 2009, All Rights Reserved.

 At the discretion of the user of this library,
 this software may be licensed under the terms of the
 GNU General Public License v3, a BSD-Style license, or the
 original HIDAPII license as outlined in the LICENSE.txt,
 LICENSE-gpl3.txt, LICENSE-bsd.txt, and LICENSE-orig.txt
 files located at the root of the source distribution.
 These files may also be found in the public source
 code repository located at:
        https://github.com/libusb/hiddapii .
********************************************************/

/** @file
 * @defgroup APII hiddapii APII
 */

#ifndef HIDAPII_H__
#define HIDAPII_H__

#include <wchar.h>

#ifdef _WIN32
#define HID_APII_EXPORT __declspec(dllexport)
#define HID_APII_CALL
#else
#define HID_APII_EXPORT /**< APII export macro */
#define HID_APII_CALL   /**< APII call macro */
#endif

#define HID_APII_EXPORT_CALL HID_APII_EXPORT HID_APII_CALL /**< APII export and call macro*/

/** @brief Static/compile-time major version of the library.

    @ingroup APII
*/
#define HID_APII_VERSION_MAJOR 0
/** @brief Static/compile-time minor version of the library.

    @ingroup APII
*/
#define HID_APII_VERSION_MINOR 10
/** @brief Static/compile-time patch version of the library.

    @ingroup APII
*/
#define HID_APII_VERSION_PATCH 1

/* Helper macros */
#define HID_APII_AS_STR_IMPL(x) #x
#define HID_APII_AS_STR(x) HID_APII_AS_STR_IMPL(x)
#define HID_APII_TO_VERSION_STR(v1, v2, v3) HID_APII_AS_STR(v1.v2.v3)

/** @brief Static/compile-time string version of the library.

    @ingroup APII
*/
#define HID_APII_VERSION_STR                                                                       \
    HID_APII_TO_VERSION_STR(HID_APII_VERSION_MAJOR, HID_APII_VERSION_MINOR, HID_APII_VERSION_PATCH)

#ifdef __cplusplus
extern "C" {
#endif
struct hidd_apii_version {
    int major;
    int minor;
    int patch;
};

struct hidd_device_;
typedef struct hidd_device_ hidd_device; /**< opaque hiddapii structure */

/** hiddapii info structure */
struct hidd_device_info {
    /** Platform-specific device path */
    char* path;
    /** Device Vendor ID */
    unsigned short vendor_id;
    /** Device Product ID */
    unsigned short product_id;
    /** Serial Number */
    wchar_t* serial_number;
    /** Device Release Number in binary-coded decimal,
        also known as Device Version Number */
    unsigned short release_number;
    /** Manufacturer String */
    wchar_t* manufacturer_string;
    /** Product string */
    wchar_t* product_string;
    /** Usage Page for this Device/Interface
        (Windows/Mac/hiddraw only) */
    unsigned short usage_page;
    /** Usage for this Device/Interface
        (Windows/Mac/hiddraw only) */
    unsigned short usage;
    /** The USB interface which this logical device
        represents.

        * Valid on both Linux implementations in all cases.
        * Valid on the Windows implementation only if the device
          contains more than one interface.
        * Valid on the Mac implementation if and only if the device
          is a USB HID device. */
    int interface_number;

    /** Pointer to the next device */
    struct hidd_device_info* next;
};

/** @brief Initialize the HIDAPII library.

    This function initializes the HIDAPII library. Calling it is not
    strictly necessary, as it will be called automatically by
    hidd_enumerate() and any of the hidd_open_*() functions if it is
    needed.  This function should be called at the beginning of
    execution however, if there is a chance of HIDAPII handles
    being opened by different threads simultaneously.

    @ingroup APII

    @returns
        This function returns 0 on success and -1 on error.
*/
int HID_APII_EXPORT HID_APII_CALL hidd_init(void);

/** @brief Finalize the HIDAPII library.

    This function frees all of the static data associated with
    HIDAPII. It should be called at the end of execution to avoid
    memory leaks.

    @ingroup APII

    @returns
        This function returns 0 on success and -1 on error.
*/
int HID_APII_EXPORT HID_APII_CALL hidd_exit(void);

/** @brief Enumerate the HID Devices.

    This function returns a linked list of all the HID devices
    attached to the system which match vendor_id and product_id.
    If @p vendor_id is set to 0 then any vendor matches.
    If @p product_id is set to 0 then any product matches.
    If @p vendor_id and @p product_id are both set to 0, then
    all HID devices will be returned.

    @ingroup APII
    @param vendor_id The Vendor ID (VID) of the types of device
        to open.
    @param product_id The Product ID (PID) of the types of
        device to open.

    @returns
        This function returns a pointer to a linked list of type
        struct #hidd_device_info, containing information about the HID devices
        attached to the system, or NULL in the case of failure. Free
        this linked list by calling hidd_free_enumeration().
*/
struct hidd_device_info HID_APII_EXPORT* HID_APII_CALL hidd_enumerate(unsigned short vendor_id,
                                                                      unsigned short product_id);

/** @brief Free an enumeration Linked List

    This function frees a linked list created by hidd_enumerate().

    @ingroup APII
    @param devs Pointer to a list of struct_device returned from
              hidd_enumerate().
*/
void HID_APII_EXPORT HID_APII_CALL hidd_free_enumeration(struct hidd_device_info* devs);

/** @brief Open a HID device using a Vendor ID (VID), Product ID
    (PID) and optionally a serial number.

    If @p serial_number is NULL, the first device with the
    specified VID and PID is opened.

    This function sets the return value of hidd_error().

    @ingroup APII
    @param vendor_id The Vendor ID (VID) of the device to open.
    @param product_id The Product ID (PID) of the device to open.
    @param serial_number The Serial Number of the device to open
                       (Optionally NULL).

    @returns
        This function returns a pointer to a #hidd_device object on
        success or NULL on failure.
*/
HID_APII_EXPORT hidd_device* HID_APII_CALL hidd_open(unsigned short vendor_id,
                                                     unsigned short product_id,
                                                     const wchar_t* serial_number);

/** @brief Open a HID device by its path name.

    The path name be determined by calling hidd_enumerate(), or a
    platform-specific path name can be used (eg: /dev/hiddraw0 on
    Linux).

    This function sets the return value of hidd_error().

    @ingroup APII
    @param path The path name of the device to open

    @returns
        This function returns a pointer to a #hidd_device object on
        success or NULL on failure.
*/
HID_APII_EXPORT hidd_device* HID_APII_CALL hidd_open_path(const char* path);

/** @brief Write an Output report to a HID device.

    The first byte of @p data[] must contain the Report ID. For
    devices which only support a single report, this must be set
    to 0x0. The remaining bytes contain the report data. Since
    the Report ID is mandatory, calls to hidd_write() will always
    contain one more byte than the report contains. For example,
    if a hidd report is 16 bytes long, 17 bytes must be passed to
    hidd_write(), the Report ID (or 0x0, for devices with a
    single report), followed by the report data (16 bytes). In
    this example, the length passed in would be 17.

    hidd_write() will send the data on the first OUT endpoint, if
    one exists. If it does not, it will send the data through
    the Control Endpoint (Endpoint 0).

    This function sets the return value of hidd_error().

    @ingroup APII
    @param dev A device handle returned from hidd_open().
    @param data The data to send, including the report number as
        the first byte.
    @param length The length in bytes of the data to send.

    @returns
        This function returns the actual number of bytes written and
        -1 on error.
*/
int HID_APII_EXPORT HID_APII_CALL hidd_write(hidd_device* dev, const unsigned char* data,
                                             size_t length);

/** @brief Read an Input report from a HID device with timeout.

    Input reports are returned
    to the host through the INTERRUPT IN endpoint. The first byte will
    contain the Report number if the device uses numbered reports.

    This function sets the return value of hidd_error().

    @ingroup APII
    @param dev A device handle returned from hidd_open().
    @param data A buffer to put the read data into.
    @param length The number of bytes to read. For devices with
        multiple reports, make sure to read an extra byte for
        the report number.
    @param milliseconds timeout in milliseconds or -1 for blocking wait.

    @returns
        This function returns the actual number of bytes read and
        -1 on error. If no packet was available to be read within
        the timeout period, this function returns 0.
*/
int HID_APII_EXPORT HID_APII_CALL hidd_read_timeout(hidd_device* dev, unsigned char* data,
                                                    size_t length, int milliseconds);

/** @brief Read an Input report from a HID device.

    Input reports are returned
    to the host through the INTERRUPT IN endpoint. The first byte will
    contain the Report number if the device uses numbered reports.

    This function sets the return value of hidd_error().

    @ingroup APII
    @param dev A device handle returned from hidd_open().
    @param data A buffer to put the read data into.
    @param length The number of bytes to read. For devices with
        multiple reports, make sure to read an extra byte for
        the report number.

    @returns
        This function returns the actual number of bytes read and
        -1 on error. If no packet was available to be read and
        the handle is in non-blocking mode, this function returns 0.
*/
int HID_APII_EXPORT HID_APII_CALL hidd_read(hidd_device* dev, unsigned char* data, size_t length);

/** @brief Set the device handle to be non-blocking.

    In non-blocking mode calls to hidd_read() will return
    immediately with a value of 0 if there is no data to be
    read. In blocking mode, hidd_read() will wait (block) until
    there is data to read before returning.

    Nonblocking can be turned on and off at any time.

    @ingroup APII
    @param dev A device handle returned from hidd_open().
    @param nonblock enable or not the nonblocking reads
     - 1 to enable nonblocking
     - 0 to disable nonblocking.

    @returns
        This function returns 0 on success and -1 on error.
*/
int HID_APII_EXPORT HID_APII_CALL hidd_set_nonblocking(hidd_device* dev, int nonblock);

/** @brief Send a Feature report to the device.

    Feature reports are sent over the Control endpoint as a
    Set_Report transfer.  The first byte of @p data[] must
    contain the Report ID. For devices which only support a
    single report, this must be set to 0x0. The remaining bytes
    contain the report data. Since the Report ID is mandatory,
    calls to hidd_send_feature_report() will always contain one
    more byte than the report contains. For example, if a hidd
    report is 16 bytes long, 17 bytes must be passed to
    hidd_send_feature_report(): the Report ID (or 0x0, for
    devices which do not use numbered reports), followed by the
    report data (16 bytes). In this example, the length passed
    in would be 17.

    This function sets the return value of hidd_error().

    @ingroup APII
    @param dev A device handle returned from hidd_open().
    @param data The data to send, including the report number as
        the first byte.
    @param length The length in bytes of the data to send, including
        the report number.

    @returns
        This function returns the actual number of bytes written and
        -1 on error.
*/
int HID_APII_EXPORT HID_APII_CALL hidd_send_feature_report(hidd_device* dev,
                                                           const unsigned char* data,
                                                           size_t length);

/** @brief Get a feature report from a HID device.

    Set the first byte of @p data[] to the Report ID of the
    report to be read.  Make sure to allow space for this
    extra byte in @p data[]. Upon return, the first byte will
    still contain the Report ID, and the report data will
    start in data[1].

    This function sets the return value of hidd_error().

    @ingroup APII
    @param dev A device handle returned from hidd_open().
    @param data A buffer to put the read data into, including
        the Report ID. Set the first byte of @p data[] to the
        Report ID of the report to be read, or set it to zero
        if your device does not use numbered reports.
    @param length The number of bytes to read, including an
        extra byte for the report ID. The buffer can be longer
        than the actual report.

    @returns
        This function returns the number of bytes read plus
        one for the report ID (which is still in the first
        byte), or -1 on error.
*/
int HID_APII_EXPORT HID_APII_CALL hidd_get_feature_report(hidd_device* dev, unsigned char* data,
                                                          size_t length);

/** @brief Get a input report from a HID device.

    Set the first byte of @p data[] to the Report ID of the
    report to be read. Make sure to allow space for this
    extra byte in @p data[]. Upon return, the first byte will
    still contain the Report ID, and the report data will
    start in data[1].

    @ingroup APII
    @param device A device handle returned from hidd_open().
    @param data A buffer to put the read data into, including
        the Report ID. Set the first byte of @p data[] to the
        Report ID of the report to be read, or set it to zero
        if your device does not use numbered reports.
    @param length The number of bytes to read, including an
        extra byte for the report ID. The buffer can be longer
        than the actual report.

    @returns
        This function returns the number of bytes read plus
        one for the report ID (which is still in the first
        byte), or -1 on error.
*/
int HID_APII_EXPORT HID_APII_CALL hidd_get_input_report(hidd_device* dev, unsigned char* data,
                                                        size_t length);

/** @brief Close a HID device.

    This function sets the return value of hidd_error().

    @ingroup APII
    @param dev A device handle returned from hidd_open().
*/
void HID_APII_EXPORT HID_APII_CALL hidd_close(hidd_device* dev);

/** @brief Get The Manufacturer String from a HID device.

    @ingroup APII
    @param dev A device handle returned from hidd_open().
    @param string A wide string buffer to put the data into.
    @param maxlen The length of the buffer in multiples of wchar_t.

    @returns
        This function returns 0 on success and -1 on error.
*/
int HID_APII_EXPORT_CALL hidd_get_manufacturer_string(hidd_device* dev, wchar_t* string,
                                                      size_t maxlen);

/** @brief Get The Product String from a HID device.

    @ingroup APII
    @param dev A device handle returned from hidd_open().
    @param string A wide string buffer to put the data into.
    @param maxlen The length of the buffer in multiples of wchar_t.

    @returns
        This function returns 0 on success and -1 on error.
*/
int HID_APII_EXPORT_CALL hidd_get_product_string(hidd_device* dev, wchar_t* string, size_t maxlen);

/** @brief Get The Serial Number String from a HID device.

    @ingroup APII
    @param dev A device handle returned from hidd_open().
    @param string A wide string buffer to put the data into.
    @param maxlen The length of the buffer in multiples of wchar_t.

    @returns
        This function returns 0 on success and -1 on error.
*/
int HID_APII_EXPORT_CALL hidd_get_serial_number_string(hidd_device* dev, wchar_t* string,
                                                       size_t maxlen);

/** @brief Get a string from a HID device, based on its string index.

    @ingroup APII
    @param dev A device handle returned from hidd_open().
    @param string_index The index of the string to get.
    @param string A wide string buffer to put the data into.
    @param maxlen The length of the buffer in multiples of wchar_t.

    @returns
        This function returns 0 on success and -1 on error.
*/
int HID_APII_EXPORT_CALL hidd_get_indexed_string(hidd_device* dev, int string_index,
                                                 wchar_t* string, size_t maxlen);

/** @brief Get a string describing the last error which occurred.

    Whether a function sets the last error is noted in its
    documentation. These functions will reset the last error
    to NULL before their execution.

    Strings returned from hidd_error() must not be freed by the user!

    This function is thread-safe, and error messages are thread-local.

    @ingroup APII
    @param dev A device handle returned from hidd_open(),
      or NULL to get the last non-device-specific error
      (e.g. for errors in hidd_open() itself).

    @returns
        This function returns a string containing the last error
        which occurred or NULL if none has occurred.
*/
HID_APII_EXPORT const wchar_t* HID_APII_CALL hidd_error(hidd_device* dev);

/** @brief Get a runtime version of the library.

    @ingroup APII

    @returns
        Pointer to statically allocated struct, that contains version.
*/
HID_APII_EXPORT const struct hidd_apii_version* HID_APII_CALL hidd_version(void);

/** @brief Get a runtime version string of the library.

    @ingroup APII

    @returns
        Pointer to statically allocated string, that contains version string.
*/
HID_APII_EXPORT const char* HID_APII_CALL hidd_version_str(void);

#ifdef __cplusplus
}
#endif

#endif
