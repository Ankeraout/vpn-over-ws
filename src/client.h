#ifndef __INC_CLIENT_H__
#define __INC_CLIENT_H__

/**
 * @brief Connects to the server at the given URL and starts the client thread.
 *
 * @param[in] url The URL to connect to.
 *
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if no error occurred.
 * @retval Any other value if an error occurred.
 */
int clientConnect(const char *url);

int clientWrite(const void *buffer, size_t bufferSize);

#endif
