```c++
#include <Luna/Runtime/Variant.hpp>
```

`Variant` is a dynamic typed object that stores data in a schema-less (self-described) manner. `Variant` is used as a general way of representing data for purposes like [[Serialization and Deserialization]].

## Variant type

The type of one `Variant` is represented by `VariantType` enumeration and can be fetched by calling `type` method. LunaSDK supports the following variant types:

1. Null
2. Object
3. Array
4. Number
5. String
6. Boolean
7. BLOB

## Null variant

Variant can be `null`, which represents the absence of value for the variant object. Calling `type` of one null variant returns `VariantType::null`, and calling `valid` of one null variant returns `false`.

## Number variant

Number variant contains one number of integer or floating-point type. The number type of one number variant is represented by `VariantNumberType` enumeration and can be fetched by calling `number_type` method. If the variant object is not a number type, `VariantNumberType::not_number` will be returned.

The number value of the variant can be fetched by calling `unum`, `inum` and `fnum` methods, each of them returns the underlying number in specified format with implicit type conversion when needed. If the variant type is not `VariantType::number`, `0` or `0.0` will be returned.

One variant can be set to number by assigning it with one integer or floating-point value or instance.

## String variant

String variant contains one single string represented by a `Name` object. You can fetch the underlying string of one variant by calling `str()` method, which returns one empty string if the type of the variant is not `VariantType::string`. We also provide `c_str` method to fetch the string buffer quickly, which will return `""` if the variant is not `VariantType::string`.

One variant can be set to string by assigning it a `Name`, a string literal, or a null-terminated `const c8*`. To assign data stored in `String`, pass its `c_str()` value.

## Boolean variant

Boolean variant contains only two kinds of values: `true` and `false`. The Boolean value of one variant can be fetched by calling `boolean` method, which returns `false` if the variant is not `VariantType::boolean`.

One variant can be set to Boolean by assigning it with one `bool` value or instance.

## BLOB Variant

BLOB variant contains one single binary large object. The data, size and alignment of the data can be fetched by calling `blob_data`, `blob_size` and `blob_alignment` methods. Note that `Variant` does optimizations for small blob data, so the blob data is not necessary represented by `Blob`. You may detach the blob data from the variant by calling `blob_detach`, which returns the blob data as a `Blob` object, and the variant will contain one empty blob after this operation.

One variant can be set to BLOB by assigning it a `Blob` value. The data is copied from an lvalue `Blob` and moved from an rvalue `Blob`.

## Array of variants

Array variant contains one array of `Variant` objects, which acts as sub-objects of the current object. Note that `Variant` does optimizations for small array, so the array data is not necessary represented by `Vector<Variant>`.

## Object variants

Object variant contains one unordered set of name-value pairs. Child variants are indexed by `Name` keys. The internal representation is an implementation detail and is not necessarily `HashMap<Name, Variant>`.

For array and object variants, `size` returns the number of child variants and `empty` reports whether that count is zero. The const array subscript returns the shared null sentinel from `Variant::npos()` when the value is not an array or the index is invalid. The mutable array subscript requires a valid array index.

The const object subscript returns `Variant::npos()` when the value is not an object or the key is absent. The mutable object subscript finds or inserts the requested key. If the current variant is null, it is first converted to an object; using the mutable object subscript on any other type violates the API's valid-usage requirements.

## Variant differential

```c++
#include <Luna/VariantUtils/Diff.hpp>
```

The VariantUtils module provides a variant differential library in the `Luna::VariantUtils` namespace. `diff(before, after)` returns a delta variant that represents the changes from `before` to `after`. `patch(before, delta)` applies those changes, while `revert(after, delta)` removes them. These functions are useful for data versioning and undo/redo operations.

## JSON encoding

```c++
#include <Luna/VariantUtils/JSON.hpp>
```

The VariantUtils module provides JSON encoding and decoding for `Variant` objects. `write_json` encodes a `Variant` to JSON text, while `read_json` decodes JSON text to a `Variant`.

The default options preserve LunaSDK's relaxed JSON behavior. This includes comments, trailing commas, non-standard whitespace and escapes, UTF-16 input, Tagged BLOB strings, trailing content, and non-finite floating-point values. Applications consuming data from a strict protocol should pass `JSONReadOptions::strict()` and `JSONWriteOptions::strict()` explicitly:

```cpp
R<Luna::String> normalize_strict_json(const Luna::c8* json_data,
    Luna::usize json_size)
{
    using namespace Luna;
    using namespace Luna::VariantUtils;

    R<Variant> value = read_json(
        json_data, json_size, JSONReadOptions::strict());
    if(!value.valid()) return value.errcode();

    return write_json(value.get(), JSONWriteOptions::strict());
}
```

### Read options

`JSONReadOptions` contains these independently configurable behaviors:

1. `allow_comments`: Accepts `//` and `/* ... */` comments between tokens.
2. `allow_trailing_commas`: Accepts a comma before the closing token of an object or array.
3. `allow_nonstandard_escapes`: Accepts `\0` and `\'` in strings.
4. `allow_nonstandard_whitespace`: Accepts Unicode whitespace in addition to the four JSON whitespace characters.
5. `allow_trailing_content`: Stops after the first root value and ignores remaining content.
6. `decode_blobs`: Converts strings using the LunaSDK `@base64@` or `@base85@` format to BLOB variants.
7. `allow_utf16`: Accepts UTF-16 input with a byte-order mark.
8. `allow_non_finite_numbers`: Allows a number outside the finite `f64` range to become infinity and accepts the non-standard `nan`, `inf`, and `-inf` tokens.

The strict preset disables all of these behaviors. Both presets still reject malformed numbers, unescaped control characters, unterminated strings and comments, missing separators, invalid UTF-8, invalid Unicode surrogate sequences, and duplicate object names. Duplicate names are rejected after escape sequences are decoded, because a `Variant` object cannot represent multiple values under one `Name` key.

The buffer overload treats an explicit `json_size` as the exact number of available bytes. Embedded null bytes are invalid input; use the null-terminated overload, or pass `USIZE_MAX`, when the size should be determined with `strlen`.

JSON number parsing and writing always use JSON's dot decimal notation and are independent of the process locale.

### Write options

`JSONWriteOptions` contains these independently configurable behaviors:

1. `indent`: Adds indentation and line breaks.
2. `encode_blobs`: Encodes a BLOB as a LunaSDK Tagged BLOB string. Writing fails if a BLOB is encountered while this option is disabled.
3. `allow_non_finite_numbers`: Writes the non-standard `nan`, `inf`, and `-inf` tokens. Writing fails when this option is disabled and a non-finite number is encountered.

The strict preset disables all three options, producing compact JSON and rejecting values that do not belong to the JSON data model. JSON writing always validates UTF-8 in string values and object names, independently of these options. The options-based string overload returns `R<String>` so representation errors can be reported. The legacy `write_json(const Variant&, bool indent)` overload remains available with relaxed behavior.
