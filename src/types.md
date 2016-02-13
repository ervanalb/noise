TODO make this better

# Objects

nz_obj * is a void pointer. It points to the actual data of a noise object. Its contents vary based on
what kind of noise object it is pointing to. It contains no meta-data about the object, such as its
type or methods to operate on it. This makes it very lightweight. For example, for a noise "double",
the nz_obj * can be safely cast to a double* and examined or mutated.

Every noise object has a type. nz_type * is also a void pointer. It points to any additional data
that is necessary for describing a type that is not already handled by the typeclass. For instance,
in a noise "array" type, the nz_type * points to a struct nz_array_type which has the array size and
type information for the array elements. For more basic types such as a noise "double", the typeclass
fully specifies the behavior, and this pointer is not used.

Every noise object also has a typeclass. Typeclasses are defined const at compile-time, and contain
most of the metadata regarding typing. For instance, they contain many function pointers used for
working with types and objects, mostly concerning their creation and destruction.

## Typeclasses

type_id is a const string declared at compile-time that uniquely identifies the typeclass.
It is registered with the underlying noise subsystem, and helps noise know what types exist
and ensure there are no collisions. It is used when instantiating types from strings.

type_create is a function which creates a type from a typeclass. It populates type with
any necessary type data extracted from a string argument. For simple types such as "double",
there is no extra type metadata so it will throw an error if string is not NULL, and upon
success, will put NULL in type_p. For more complicated types such as "array", it will parse
the size and element type out of string, and put a more machine-readable representation of
it in type_p.
If type_create returns NZ_SUCCESS, then it has allocated and populated the type*. The
type should not be mutated after creation. type_destroy must be called to free any memory
associated with the type after it has served its purpose. If type_create returns an error,
type* is undefined and no memory needs to be freed later.

The next four methods operate on types. They take the type* data pointer as their first
argument.

type_destroy frees any memory allocated for a type following a successful call to type_create.
type_p should not be used after calling type_destroy. Do not call type_destroy with a
NULL pointer. If type_create did not complete successful, do not call type_destroy on
the type_p, as it contains an undefined value.

type_is_equal checks if two type_p's describe the same type. It must only be used on
types that are known to be of the same typeclass. To check equality of both typeclass
and type, use the global function nz_types_are_equal. It delegates to this function
after checking that the typeclasses are equal. For simple types that are fully defined
by their typeclass, such as "double", this function should always return true.

type_str returns a string describing the data contained in type_p. If type_p is not
used, this function should put NULL in *string. It should not include the typeclass
name. It should be both human and machine readable. Namely, the result of type_str
should be able to be passed back into type_create to construct an equivalent type
(one for which type_is_equal returns true). On success, it mallocs a char[] which
must be subsequently freed. On failure, no free() is necessary and *string contains
an undefined value.

type_create_obj instantiates an object of the given type. It puts the result in obj_p.
Objects differ slightly from types in that they are always constructed with no
arguments, are mutable, and end up in a default state. To fill them with useful data,
you can use type_init_obj or type_copy_obj.
A successful call to type_create_obj allocates memory which must be freed using
type_destroy_obj. An unsuccessful call to type_create_obj does not allocate memory, and
puts an undefined value into obj_p.

The next four methods operate on objects. They take the type_p associated with the
object as well as the obj_p data pointer.

type_init_obj populates an object with data parsed out of a string. The obj* must
have been previously allocated using type_obj_create, and must not be NULL. Likewise,
the string pointer must not be NULL and the string must be null-terminated. A failed
execution of type_init_obj should not alter the object data.

type_copy_obj populates an object with the data from another object. The source obj*
must be of the same typeclass and type. The result is a deep copy, and can be mutated
in any way without affecting the object it was copied from. For simple types such as
"double", this may be a single call to memcpy(dst, src, sizeof(double)). But, for more
advanced types, more complicated memory gymnastics or calls to child type_obj_copy may
be necessary. This function should be written with an eye for speed, as it is often
used at pull-time. If possible, it should not allocate or free any memory. In general,
this function will not fail, but if it does, it should not alter the underlying object
data.

type_destroy_obj frees all memory associated with an object. It should be called
following a successful run of type_create_obj. It should not be called with a NULL
obj_p or an invalid obj_p from an unsuccessful call to type_create_obj.

type_str_obj converts the value of the object into a string. This string should
be human-readable, but also machine readable. Passing this string into type_init_obj
should result in an equivalent object. The string should not contain any typeclass
or type information. On success, this function mallocs a char[] which must be later
freed. On failure, free() is not necessary and *string contains an undefined value.

Strings returned from type_str_obj should be properly encapsulated or escaped.
The same should apply to strings passed in to type_init_obj.

Creating a noise type involves implementing all of these functions, in addition
to any custom interaction you may want. But don't worry! There are helper macros for
common patterns (see below.)
