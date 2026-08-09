/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * Accessibilita JoyStick Interface Firmware
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */

#include <errno.h>
#include <stddef.h>
#include <sys/types.h>

int _close(int file);
off_t _lseek(int file, off_t offset, int whence);
ssize_t _read(int file, void *buffer, size_t length);
ssize_t _write(int file, const void *buffer, size_t length);

/*
 * The firmware does not expose POSIX file descriptors.  Supplying deliberate
 * ENOSYS stubs makes that contract explicit and removes the ambiguous nosys
 * linker diagnostics without silently routing stdio onto a safety-relevant
 * UART or other peripheral.
 */
int _close(int file)
{
    (void)file;
    errno = ENOSYS;
    return -1;
}

off_t _lseek(int file, off_t offset, int whence)
{
    (void)file;
    (void)offset;
    (void)whence;
    errno = ENOSYS;
    return (off_t)-1;
}

ssize_t _read(int file, void *buffer, size_t length)
{
    (void)file;
    (void)buffer;
    (void)length;
    errno = ENOSYS;
    return (ssize_t)-1;
}

ssize_t _write(int file, const void *buffer, size_t length)
{
    (void)file;
    (void)buffer;
    (void)length;
    errno = ENOSYS;
    return (ssize_t)-1;
}
