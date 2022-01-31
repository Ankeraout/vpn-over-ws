#ifndef __INC_COMMON_H__
#define __INC_COMMON_H__

#include <stddef.h>

#include <openssl/bio.h>
#include <openssl/ssl.h>

#define UNUSED_PARAMETER(x) ((void)x)

/**
 * @brief Reads n bytes from fd and stores them in buffer.
 *
 * @param[in] fd The file descriptor number.
 * @param[out] buffer The buffer where the read data shall be stored.
 * @param[in] n The number of bytes to read in the buffer.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if the read operation failed because the end of file was reached.
 * @retval -1 if the read operation failed because an error occurred.
 * @retval n if the read operation succeeded.
 */
ssize_t readForce(int fd, void *buffer, size_t n);

/**
 * @brief Reads n bytes from ssl and stores them in buffer.
 *
 * @param[in] ssl The SSL file descriptor.
 * @param[out] buffer The buffer where the read data shall be stored.
 * @param[in] n The number of bytes to read in the buffer.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if the read operation failed because the end of file was reached.
 * @retval -1 if the read operation failed because an error occurred.
 * @retval n if the read operation succeeded.
 */
ssize_t readForceSsl(SSL *ssl, void *buffer, size_t n);

#endif
