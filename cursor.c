#include "types.h"

i32 GetCharIndexAfter(WideString *str, i32 afterIndex, WCHAR ch)
{
    for (i32 i = afterIndex + 1; i < str->size; i++)
    {
        if (*(str->content + i) == L'\n')
            return i;
    }
    return -1;
}

i32 GetCurrentOffset(WideString *str, i32 pos)
{
    i32 currentLineOffset = 0;
    for (i32 i = pos - 1; i >= 0; i--)
    {
        if (*(str->content + i) == L'\n')
            break;
        else
            currentLineOffset++;
    }
    return currentLineOffset;
}

i32 GetPositionOnNextLine(WideString *str, u32 currentPosition)
{
    u32 currentLineOffset = GetCurrentOffset(str, currentPosition);
    u32 nextLineStartIndex = GetCharIndexAfter(str, currentPosition - 1, L'\n');

    if (nextLineStartIndex != 0)
    {
        i32 lineAfterNextLine = GetCharIndexAfter(str, nextLineStartIndex, L'\n');
        if (lineAfterNextLine == -1)
            lineAfterNextLine = str->size - 1;

        i32 result = nextLineStartIndex + currentLineOffset + 1;
        if (result > lineAfterNextLine)
            return lineAfterNextLine;
        else
            return result;
    }
    return currentPosition;
}

i32 GetPositionOnPrevLine(WideString *str, u32 currentPosition)
{
}