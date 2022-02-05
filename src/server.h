#ifndef __INC_SERVER_H__
#define __INC_SERVER_H__

/**
 * @brief Initializes the server.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if the operation was successful.
 * @retval Any other value if the operation failed.
 */
int serverInit(void);

/**
 * @brief Executes the server loop.
 *
 * @returns An integer that indicates the result of the server loop.
 * @retval 0 if the server loop executed normally.
 * @retval Any other value if an error occurred.
 */
int serverExecute(void);

/**
 * @brief Quits the server.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if the operation was successful.
 * @retval Any other value if an error occurred.
 */
int serverQuit(void);

#endif
