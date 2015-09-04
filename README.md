Noise
=====

Conventions
-----------

Little conventions that are used that make the code easier to follow. Break them when appropriate.

- If using noise as a library, you only need to `#include "noise.h"`.
- All extern symbols are prefixed with ``nz_`` to facilitate including as a library.
- An `rsprintf` function is used to allocate memory for a formatted string (like `asprintf`, with a different API)
- Error handling
    - If a function can error, it either returns a pointer or an ``int``
    - On error, it returns either ``NULL`` or ``-1`` respectively, such that ``errno`` is set appropriately
        - System & lib calls (e.g. ``calloc``) will set errno, no need to re-set
    - (Functions which *test* something and return a boolean will return ``0`` for false, and ``1`` for true!)
- `const` pointers when possible, as in `const char * mystr`, but not like `char * const mystr` or `const char * const mystr`.
- Carefully document when pointer ownership transfers! It should be very obvious from signature/name.
- OOP-y data:
    - Preference for ``int xxx_init(struct xxx *, ...)`` and ``void xxx_term(struct xxx *)`` functions, which initialize & deinitialize an already-allocated struct.
    - For opaque structs, use ``struct xxx * xxx_create(...)`` and ``void xxx_destroy(struct xxx *)`` functions.
    - structs have members in the form ``struct nz_xxx { int xxx_yyy; ... }`` to make code infinitely more greppable 
- Dealing with pointers to structs is nice, but so is having them on the stack. ``struct xxx x[1];`` solves this, making ``x`` be accessible as (almost) a ``struct xxx *``. Save on ``&``'s!
