#ifndef __PARSER_H__
#define __PARSER_H__

#include <stdio.h>
#include "value.h"

enum ParseResult {
    PARSER_FAIL,
    PARSER_SUCCESS
};
typedef enum ParseResult ParseResult;

ParseResult parser_parse(FILE *stream, Value **ast);

#endif /* !__PARSER_H__ */
