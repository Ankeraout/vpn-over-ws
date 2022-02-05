#ifndef __INC_CLIENT_H__
#define __INC_CLIENT_H__

/**
 * @brief Initializes the client.
 *
 * @param[in] p_url The URL to connect to.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if the operation was successful.
 * @retval Any other value if the operation failed.
 */
int clientInit(const char *p_url);

/**
 * @brief Executes the client loop.
 *
 * @returns An integer that indicates the result of the client loop.
 * @retval 0 if the client loop executed normally.
 * @retval Any other value if an error occurred.
 */
int clientExecute(void);

/**
 * @brief Quits the client.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if the operation was successful.
 * @retval Any other value if an error occurred.
 */
int clientQuit(void);

#endif
