#include "../../include/text.h"

static const char *_text[TXT_COUNT] =
{
#define STATIC_TEXT(id, string) string,
#include "../../include/tables/text_table.h"
#undef STATIC_TEXT
};

const char * text(TextID id)
{
    if (id < 0 || id > TXT_COUNT)
    {
	return "<invalid text>";
    }

    return _text[id];
}
