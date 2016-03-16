Noise
=====

Building & Testing
------------------

```
git clone https://github.com/ervanalb/noise
cd nzlib
make
cd ..
make

LD_LIBRARY_PATH=.:nzlib ./noise_test_unison

cd ui
LD_LIBRARY_PATH=.. python3 ui.py
```

Typesystem
----------

- `typedef void nz_obj;` -- `nz_obj` represents an object/value that is part of the typesystem. You don't get type checking in C, though :(

See [src/types.md](./src/types.md).

Blocks & Graphs
---------------

- `nz_obj * NZ_PULL(struct nz_block self, int port_index, nz_obj * result);` -- returns NULL on failure, `result` on success.
    - The caller owns `result` and needs to ensure that there is enough memory allocated to hold the result.
    - Types are checked at connect-time, not pull-time. There is no type checking in the C code (sadly...)

- `struct nz_context` keeps track of the available typeclasses & blockclasses. These can be loaded at runtime from a shared library. Currently there is only the standard library, `nzlib`.


Conventions
-----------

Little conventions that are used that make the code easier to follow. Break them when appropriate.

- If using noise as a library, you only need to `#include "libnoise.h"`.
- All extern symbols are prefixed with ``nz_`` to facilitate including as a library.
- An `rsprintf` function is used to allocate memory for a formatted string (like `asprintf`, with a `strdup`-like API)
- Error handling
    - If a function can error, it will return an ``nz_rc``, which is `typedef`'d to `int`.
    - On success, it will return `0` / `NZ_SUCCESS`. On failure it will return a (positive) error code
        - Error codes defined in `noise.h`
    - (Functions which *test* something and return a boolean will return ``0`` for false, and ``1`` for true!)
- Carefully document when pointer ownership transfers! It should be very obvious from signature/name.
- OOP-y data:
    - Preference for ``nz_rc xxx_init(struct xxx *, ...)`` and ``void xxx_term(struct xxx *)`` functions, which initialize & deinitialize an already-allocated struct.
    - For opaque structs, use ``struct xxx * xxx_create(...)`` and ``void xxx_destroy(struct xxx *)`` functions.
    - structs have members in the form ``struct nz_xxx { int xxx_yyy; ... }`` to make code infinitely more greppable 
- Dealing with pointers to structs is nice, but so is having them on the stack. ``struct xxx x[1];`` solves this, making ``x`` be accessible as (almost) a ``struct xxx *``.
