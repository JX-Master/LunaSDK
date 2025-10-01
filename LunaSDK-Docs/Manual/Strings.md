Strings are sequences of characters represented by `c8`, `c16` and `c32`, terminated by a null terminator (`\0`). LunaSDK provides various string types and libraries, they will be discussed in this section.

## String types

```c++
#include <Luna/Runtime/String.hpp> // For String, String16 and String32.
#include <Luna/Runtime/Name.hpp> // For Name.
```

LunaSDK provides two kinds of string types: `String` and `Name`.

The `String` type is a sequence of `c8` characters ended with `\0`. We designed `String` as a replacement of `std::string` in LunaSDK, so most methods used for `std::string` should work find with our `String` type. Besides the `String` type, we also have `String16` and `String32` as replacements for `std::u16string` and `std::u32string` that holds character sequences of `c16` and `c32` types.

The `Name` type represents one immutable `c8` string that is usually used as an identifier. We implemented a global name registry in LunaSDK so that every unique name will have only one data copy in the registry, and all `Name` objects with the same string data refers to that copy, thus can be compared for equality quickly. The name string data is reference counted, and will be freed when the last `Name` object that refers to the data is destructed. Strings stored in `Name` cannot be changed, if the user assigns `Name` with another string, the `Name` object will refer to another string data entry, remaining the original string entry unchanged.

`Name` and `String` can be converted to each other implicitly. There is no enforced encoding format for string types, but most text processing APIs in LunaSDK expects UTF-8 encoded strings for `String` and `Name` types.

## String utility library

```c++
#include <Luna/Runtime/StringUtils.hpp>
```

The string utility library provides functions for processing characters and strings. LunaSDK imports the following string and character processing functions from C standard library that can be used directly in LunaSDK:

1. `strncpy`
2. `strcat`
3. `strncat`
4. `strxfrm`
5. `strncmp`
6. `strcoll`
7. `strchr`
8. `strrchr`
9. `strspn`
10. `strpbrk`
11. `strstr`
12. `strtok`
13. `isalnum`
14. `isalpha`
15. `islower`
16. `isupper`
17. `tolower`
18. `toupper`
19. `isdigit`
20. `isxdigit`
21. `iscntrl`
22. `isgraph`
23. `isspace`
24. `isblank`
25. `isprint`
26. `ispunct`

`strlen`, `strcpy` and `strcmp` are compatible to C standard library, but are extended by LunaSDK so they handles all character types. `strcmp_prefix` checks whether one string is the prefix string of another string, and returns `0` if is. `strtoi64`, `strtou64` and `strtof64` interprets one number value presented the by string, and returns the value.

## Unicode encoding library

```c++
#include <Luna/Runtime/UTF8.hpp>
```

[Unicode](https://home.unicode.org/) is a text encoding standard that is widely used in modern computers, programs and websites. LunaSDK comes with a built-in Unicode library for processing strings encoded in commonly-used Unicode formats, including UTF-8, UTF-16 (LE and GE) and UTF-32.

LunaSDK uses 32-bit character type (`c32`) to represent one Unicode character, the value of the character object represents the codepoint of the character in Unicode character table. One Unicode character can be encoded to 1 `c32` character in UTF-32, 1 to 2 `c16` characters in UTF-16, and 1 to 6 `c8` characters in UTF-8.

> By definition, one Unicode character represented by `c32` differs from one Unicode character encoded using UTF-32 (the first bit of one UTF-32 character must be 0, so only 2^32 Unicode codepoints can be represented in UTF-32). But in practice, all existing Unicode characters can be converted to their UTF-32 representation without any modification, so we do not differ one Unicode character from one UTF-32 character in this manual.

`utf8_charspan` and `utf16_charspan` take one Unicode character, and return the number of `c8`  or `c16` characters required to represent that character in UTF-8 or UTF-16 encoding. `utf8_charlen` and `utf16_charlen` take the first `c8`  or `c16`  character of one UTF-8 or UTF-16 encoded Unicode character, and return the number of bytes used for that character. These functions can be used to measure the size of one UTF-32 character in UTF-8 and UTF-16 encoding.

`utf8_strlen` and `utf16_strlen` calculate the number of Unicode characters contained by a UTF-8 or UTF-16 encoded string, `utf8_index` and `utf16_index` return the index of the first `c8` or `c16` character of the `n`th Unicode character in a UTF-8 or UTF-16 ebcided string. These functions can be used to calculate the length of Unicode-encoded strings.

`utf8_encode_char` and `utf16_encode_char` encode one Unicode character into multiple `c8` or `c16` characters using UTF-8 or UTF-16 encoding, write the encoded characters to the user-provided buffer, and return the number of characters written.  `utf8_decode_char` and `utf16_decode_char`, on the other side, read multiple `c8` or `c16` characters from the user-provided buffer, and returns the Unicode character represented by these characters.

`utf16_to_utf8` converts a UTF-8 encoded string to a UTF-16 encoded string, and `utf8_to_utf16` converts a UTF-16 encoded string to a UTF-8 encoded string. Both functions write result strings in a user-provided buffer, `utf16_to_utf8_len` and `utf8_to_utf16_len` can be used to calculate the minimum size (measured in number of `c8` or `c16` characters, not including the null terminator) required for the buffer to hold the result string.

## Base64 encoding library

```c++
#include <Luna/Runtime/Base64.hpp>
```

[Base64](https://en.wikipedia.org/wiki/Base64) is an encoding format that represents arbitrary binary data using 64 printable characters, plus one character (`=`) for paddings. It is useful to store binary data in a text-based file. LunaSDK comes with a built-in Base64 library for encoding and decoding binary data using Base64.

`base64_encode` encodes the binary data in the user-provided source buffer to a Base64 encoded string, and writes the string to the user-provided destination buffer. To determine the size of the destination buffer required, call `base64_get_encoded_size` with the size of the row binary data.

`base64_decode` decodes the Base64 string in the user-provided source buffer to original binary data, and writes the binary data to the user-provided destination buffer. To determine the size of the destination buffer required, call `base64_get_decoded_size` with the size of the Base64 string, excluding the null terminator.

