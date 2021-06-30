/*
 * Purpose of the file is to allow unit test framework to test low-level API calls.
 *
 * From CMock github repository (https://github.com/hjagodzinski/C-Mock):
 * > Firstly, all functions you want to mock must be compiled into a dynamic library.
 * > If it includes your project-specific functions you must put them into a dynamic library as well
 *
 * The file is only used in unit test environment.
 */

#include "mfrc522_ll.h"

mfrc522_ll_status
mfrc522_ll_send(u8 addr, size bytes, const u8* payload)
{
    (void)addr;
    (void)bytes;
    (void)payload;
    return mfrc522_ll_status_ok;
}

mfrc522_ll_status
mfrc522_ll_recv(u8 addr, u8* payload)
{
    (void)addr;
    *payload = 0x00;
    return mfrc522_ll_status_ok;
}

void
mfrc522_ll_delay(u32 period)
{
    (void)period;
}
