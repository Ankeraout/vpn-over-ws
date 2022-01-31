#ifndef __INC_TUN_H__
#define __INC_TUN_H__

#include <stddef.h>
#include <stdio.h>

/**
 * @brief Initializes the TUN interface and sets its properties (MTU, IP).
 *
 * @param[in] ip The IP address of the TUN interface.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if no error occurred.
 * @retval Any other value if an error occurred.
 */
int tunInit(const char *ip);

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
int tunWrite(void *buffer, size_t size);

/**
 * @brief Closes the tunnel interface.
 */
void tunClose(void);

void tunLoop(void);

#endif
