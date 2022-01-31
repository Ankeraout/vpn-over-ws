#include <stddef.h>
#include <stdint.h>

#include <unistd.h>

#include <openssl/bio.h>
#include <openssl/ssl.h>

ssize_t readForce(int fd, void *buffer, size_t n) {
    if(n == 0) {
        return 0;
    }

    size_t bytesRead = 0;

    do {
        ssize_t result = read(fd, ((uint8_t *)buffer) + bytesRead, n);

        if(result < 0) {
            return -1;
        } else if(result == 0) {
            return 0;
        } else {
            bytesRead += (size_t)result;
        }
    } while(bytesRead < n);

    return bytesRead;
}

ssize_t readForceSsl(SSL *ssl, void *buffer, size_t n) {
    if(n == 0) {
        return 0;
    }

    size_t bytesRead = 0;

    do {
        int result = SSL_read(ssl, ((uint8_t *)buffer) + bytesRead, n);

        if(result < 0) {
            return -1;
        } else if(result == 0) {
            return 0;
        } else {
            bytesRead += (size_t)result;
        }
    } while(bytesRead < n);

    return bytesRead;
}
