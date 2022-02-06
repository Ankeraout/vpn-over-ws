#ifndef __INC_TUN_H__
#define __INC_TUN_H__

#include <stddef.h>

/**
 * @brief Initializes the TUN interface and sets its properties (MTU, IP).
 *
 * @param[in] ip The IP address of the TUN interface.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if no error occurred.
 * @retval Any other value if an error occurred.
 */
int tunInit(const char *p_ip);

/**
 * @brief Writes a packet to the tunnel.
 *
 * @param[in] buffer The buffer that contains the packet to send.
 * @param[in] size The size of the packet to send.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if no error occurred.
 * @retval Any other value if an error occurred.
 */
int tunWrite(void *p_buffer, size_t p_size);

/**
 * @brief Closes the tunnel interface and stops the main loop.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if no error occurred.
 * @retval Any other value if an error occurred.
 */
int tunClose(void);

/**
 * @brief TUN reader thread main loop.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if no error occurred.
 * @retval Any other value if an error occurred.
 */
void tunLoop(void);

/**
 * @brief Reads a packet (blocking).
 *
 * @param[out] p_buffer The buffer where the packet data shall be stored.
 *
 * @returns The size of the packet.
 * @retval 0 if there are no more packets to read.
 * @retval -1 if an error occurred.
 */
ssize_t tunRead(void *p_buffer);

#endif
