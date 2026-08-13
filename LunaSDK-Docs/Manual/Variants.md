```
#include <Luna/Runtime/Variant.hpp>
```

`Variant` is a dynamic typed object that stores data in a schema-less (self-described) manner. `Variant` is used as a general way of representing data for purposes like [[Serialization and Deserialization]].

## Variant type

The type of one `Variant` is represented by `VariantType` enumeration and can be fetched by calling `type` method. LunaSDK supports the following variant types:

1. Null
2. Number
3. String
4. Boolean
5. BLOB
6. Pointer
7. Array of variants
8. Associated array of variants

## Null variant

Variant can be `null`, which represents the absence of value for the variant object. Calling `type` of one null variant returns `VariantType::null`, and calling `valid` of one null variant returns `false`.

## Number variant

Number variant contains one number of integer or floating-point type. The number type of one number variant is represented by `VariantNumberType` enumeration and can be fetched by calling `number_type` method. If the variant object is not a number type, `NumberType::not_number` will be returned.

The number value of the variant can be fetched by calling `unum`, `inum` and `fnum` methods, each of them returns the underlying number in specified format with implicit type conversion when needed. If the variant type is not `VariantType::number`, `0` or `0.0` will be returned.

One variant can be set to number by assigning it with one integer or floating-point value or instance.

## String variant

String variant contains one single string represented by a `Name` object. You can fetch the underlying string of one variant by calling `str()` method, which returns one empty string if the type of the variant is not `VariantType::string`. We also provide `c_str` method to fetch the string buffer quickly, which will return `""` if the variant is not `VariantType::string`.

One variant can be set to string by assigning it with one `Name` instance, one `String` instance, one string literal, or one zero-terminated `c8*` pointer instance.

## Boolean variant

Boolean variant contains only two kinds of values: `true` and `false`. The Boolean value of one variant can be fetched by calling `boolean` method, which returns `false` if the variant is not `VariantType::boolean`.

One variant can be set to Boolean by assigning it with one `bool` value or instance.

## BLOB Variant

BLOB variant contains one single binary large object. The data, size and alignment of the data can be fetched by calling `blob_data`, `blob_size` and `blob_alignment` methods. Note that `Variant` does optimizations for small blob data, so the blob data is not necessary represented by `Blob`. You may detach the blob data from the variant by calling `blob_detach`, which returns the blob data as a `Blob` object, and the variant will contain one empty blob after this operation.

One variant can be set to pointer by assigning it with one `Blob` value or instance.

## Pointer Variant

Pointer variant contains one type-less user pointer. The pointer is stored as-is and can be fetched by calling `pointer` method, which returns `nullptr` if the variant is not `VariantType::pointer`.

One variant can be set to pointer by assigning it with one pointer value or instance.

## Array of variants

Array variant contains one array of `Variant` objects, which acts as sub-objects of the current object. Note that `Variant` does optimizations for small array, so the array data is not necessary represented by `Vector<Variant>`.

## Associated array of variants

Associated array variant contains one set of `Variant` objects, which acts as sub-objects of the current object. Unlike array variants, objects in associated array variant are indexed by `Name` objects, and does not have a particular order. Note that `Variant` does optimizations for small array, so the array data is not necessary represented by `HashMap<Name, Variant>`.

For both array variants and associated array variants, `size` method returns the number of sub-objects in the array, and `empty` method returns `true` if `size()` returns `0`. The user can use subscript syntaxes (`[]`) to fetch elements in array variants (`[N]`) and associated array variants (`["Name"]`), if the specified element does not exist, one null variant will be returned. Using subscript syntaxes for variants with incorrect types always return null objects.

## Variant differential

```c++
#include <Luna/Runtime/VariantDiff.hpp>
```

LunaSDK comes with one variant differential library that computes and patches variant differences. `diff_variant` calculates the difference between `before` and `after` variant objects, and returns the difference as another variant object called `diff` object. `patch_variant_diff` applies `diff` object to `before` variant object to reproduce `after` object, and `reverse_variant_diff` removes the `diff` object from `after` object to reproduce `before` object. These functions are useful for implementing data versioning and undo/redo operations.

## JSON encoding

```c++
#include <Luna/VariantUtils/JSON.hpp>
```

The VariantUtils module provides JSON encoding and decoding for `Variant` objects. `write_json` encodes a `Variant` to JSON text, while `read_json` decodes JSON text to a `Variant`.

The default options preserve LunaSDK's relaxed JSON behavior. This includes comments, trailing commas, non-standard whitespace and escapes, UTF-16 input, Tagged BLOB strings, trailing content, and non-finite floating-point values. Applications consuming data from a strict protocol should pass `JSONReadOptions::strict()` and `JSONWriteOptions::strict()` explicitly:

```cpp
using namespace Luna;
using namespace Luna::VariantUtils;

JSONReadOptions read_options = JSONReadOptions::strict();
R<Variant> value = read_json(json_data, json_size, read_options);

JSONWriteOptions write_options = JSONWriteOptions::strict();
R<String> json = write_json(value.get(), write_options);
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
8. `allow_non_finite_numbers`: Allows a number outside the finite `f64` range to become infinity.

The strict preset disables all of these behaviors. Both presets still reject malformed numbers, unescaped control characters, unterminated strings and comments, missing separators, and invalid Unicode surrogate sequences.

### Write options

`JSONWriteOptions` contains these independently configurable behaviors:

1. `indent`: Adds indentation and line breaks.
2. `encode_blobs`: Encodes a BLOB as a LunaSDK Tagged BLOB string. Writing fails if a BLOB is encountered while this option is disabled.
3. `allow_non_finite_numbers`: Writes platform `nan` and `inf` tokens. Writing fails when this option is disabled and a non-finite number is encountered.

The strict preset disables all three options, producing compact JSON and rejecting values that do not belong to the JSON data model. The options-based string overload returns `R<String>` so representation errors can be reported. The legacy `write_json(const Variant&, bool indent)` overload remains available with relaxed behavior.
