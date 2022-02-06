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
 * @brief Starts the server.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if the operation was successful.
 * @retval Any other value if the operation failed.
 */
int serverStart(void);

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

/**
 * @brief Sends a packet to the server. This function is called when a packet is
 *        received on the TUN interface in server mode.
 *
 * @param[in] p_buffer The buffer that contains the packet data.
 * @param[in] p_packetSize The size of the received packet.
 */
void serverSend(const void *p_buffer, size_t p_packetSize);

#endif
