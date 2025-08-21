#ifndef TEXT_H
#define TEXT_H

typedef enum {
#define STATIC_TEXT(id, string) id,
#include "tables/text_table.h"
#undef STATIC_TEXT
	TXT_COUNT
} TextID;

const char * text(TextID id);

#endif
