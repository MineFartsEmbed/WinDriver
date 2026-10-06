#if !defined (__KERNEL__)
#if defined(__cplusplus)
extern "C" {
#endif

#define WD_AGENT_DEFAULT_TIMEOUT 3
    /**
    * Start the WinDriver Agent that allows validating the license string in
    * the running WinDriver-based process.
    * Call this function only when using the WinDriver Low Level API in
    * WinDriver version 16.40 and above.
    * Call this function right after calling WD_License() has returned
    * WD_MORE_PROCESSING_REQUIRED. 
    * See the documentation of WD_License() for a usage code snippet.
    * 
    *   @param [in] hWD:  Handle to WinDriver's kernel-mode driver as received
    *                          from WD_Open().
    *   @param [in] pcLicense:    WinDriver license registration string.
    *                     If this parameter is a NULL pointer or an empty string,
    *                     the function will attempt to register the demo WinDriver
    *                     evaluation license.
    *                     Therefore, when evaluating WinDriver pass NULL as this
    *                     parameter. After registering your WinDriver toolkit,
    *                     modify the code to pass your
    *                     WinDriver license registration string.
    *   @param [in] dwTimeout:      Maximum time, in seconds, to complete
    *                     the license validation. Increase this parameter under 
    *                     slow connections.
    *                     Pass 0 to use the default timeout value which is 3 seconds.
    * 
    * @return Returns WD_STATUS_SUCCESS (0) on success,
    *  or an appropriate error code otherwise:
    * WD_OPERATION_ALREADY_DONE - If agent was already started for this process.
    * WD_TIME_OUT_EXPIRED - If agent has not succeeded in validating or 
    *                       connecting by the defined timeout.
    */
    DWORD WD_AgentStart(_In_ HANDLE hWD, _In_ const char *pcLicense, _In_ DWORD dwTimeout);
    /**
    * Close the WinDriver Agent that allows validating the license string in
    * the running WinDriver-based process.
    * Call this function only when using the WinDriver Low Level API in
    * WinDriver version 16.40 and above.
    * Call this function right before calling WD_Close().
    *
    * @return Returns WD_STATUS_SUCCESS (0) on success,
    *  or an appropriate error code otherwise:
    * WD_OPERATION_ALREADY_DONE - If agent was already closed for this process.
    * See the documentation of WD_License() for a usage code snippet.
    */
    DWORD WD_AgentClose(void);

#if defined(__cplusplus)
}
#endif
#endif
