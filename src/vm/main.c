#include <editline/readline.h>
#include <string.h>
#include "common.h"
#include "vm/chunk.h"
#include "vm/vm.h"
#include "vm/object.h"
#include "vm/compiler.h"
#include "gc.h"

int main(int argc, const char *argv[])
{
    // set up garbage collection, use extended setup for bigger mem limits
    // FIXME: this needs args
    gc_start_ext(&gc, &argc, 16384, 16384, 0.2, 0.8, 0.5);

    VM *vm = vm_new();
    while(true) {
        // char *input = readline("stutter> ");
        char *input = readline("\U000003BB> ");
        if (input == NULL) {
            break;
        }
        if (strcmp(input, "") == 0) {
            continue;
        }
        add_history(input);

        Chunk *chunk = chunk_new();
        if (compile(input, chunk) == OK) { // FIXME use read_()
            vm_interpret(vm, chunk);
        }
        free(input);
        vm_reset(vm);
    }
    fprintf(stdout, "\n");
    vm_delete(vm);
    gc_stop(&gc);
    return 0;
}
