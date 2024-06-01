#include <windows.h>
#include "types.h"
#include "win32.c"

inline void PlaceLineEnd(WideString *buffer)
{
    if (buffer->content)
        *(buffer->content + buffer->size) = '\0';
}

void RemoveCarriageReturns(FileContent *fileContent)
{
    char *src = fileContent->content;
    char *dst = fileContent->content;
    int newSize = 0;

    for (int i = 0; i < fileContent->size; i++)
    {
        if (*src != '\r')
        {
            *dst++ = *src;
            newSize++;
        }
        src++;
    }

    fileContent->size = newSize;
    *dst = '\0'; // Null-terminate the modified content
}

void RemoveWCharAtPosition(WideString *wideString, u32 position)
{
    WCHAR *src = wideString->content + position + 1;
    WCHAR *dst = wideString->content + position;

    while (*src != L'\0')
    {
        *dst++ = *src++;
    }
    *dst = L'\0'; // Null-terminate the modified content

    wideString->size--;
}

inline void MoveMyMemory(WCHAR *source, WCHAR *dest, int length)
{
    for (int i = 0; i < length; i++)
    {
        *dest = *source;
        source++;
        dest++;
    }
}

inline void MoveBytesRight(WCHAR *ptr, int length)
{
    for (int i = length - 1; i > 0; i--)
    {
        ptr[i] = ptr[i - 1];
    }
}

void DoubleCapacityIfFull(WideString *buffer)
{
    if (buffer->size * 2 >= buffer->capacity)
    {
        WCHAR *currentStr = buffer->content;
        buffer->capacity = (buffer->capacity == 0) ? 4 : (buffer->capacity * 2);
        buffer->content = VirtualAllocateMemory(buffer->capacity);
        MoveMyMemory(currentStr, buffer->content, buffer->size);
        VirtualFreeMemory(currentStr);
    }
}

void InsertCharAt(WideString *buffer, i32 at, WCHAR ch)
{
    DoubleCapacityIfFull(buffer);

    buffer->size += 1;
    MoveBytesRight(buffer->content + at, buffer->size - at);
    *(buffer->content + at) = ch;
    PlaceLineEnd(buffer);
}
