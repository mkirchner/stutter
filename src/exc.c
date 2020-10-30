#include "exc.h"
#include "log.h"

#include <assert.h>

static const Value *exc_current = NULL;

void exc_set(const Value *error)
{
    if (exc_is_pending()) {
        LOG_CRITICAL("Cannot raise without handling existing exception: %s", STRING(exc_current));
        assert(0);
    }
    exc_current = error;
}

const Value *exc_get()
{
    return exc_current;
}

void exc_clear()
{
    exc_current = NULL;
}

bool exc_is_pending()
{
    return exc_current != NULL;
}
