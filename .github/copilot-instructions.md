# C++ code
- Use C++23 standard features (avoid modules).
- Follow the C++ Core Guidelines and modern best practices.
- Use smart pointers and RAII; avoid raw pointers unless absolutely necessary.
- Use `const` and `constexpr` wherever possible.
- Format code with consistent indentation: **use tabs** (1 tab = 4 spaces visually).
- Prefer brace initialization `{}` for variables and objects, except for inline member initializers where `=` is preferred.
- For member variables:
  - Use inline initialization with `=`.
  - End names with a trailing underscore (`score_`).
- Prefer `enum class` over traditional enums.
- Use classic header guards (`#ifndef`, `#define`, `#endif`) with guard names like `APP_GAME_INPUT_H` for `app::game` namespace in file `input.h`.
- Prefer `std::` library facilities over C-style code and avoid macros.
- Write clear, self-documenting code with meaningful names.
- For virtual functions, use one of `override`, `final`, or `virtual`.
- Base class destructors must be public and virtual when objects may be deleted through base pointers; otherwise prefer protected and non-virtual (C.35).
- Use `nullptr` instead of `NULL` or `0`.
- Avoid prefixing class names with "I" (use `Player` instead of `IPlayer`). For concrete implementations, use `PlayerImpl` or a more descriptive name if possible.
- For nested namespaces, use the compact syntax `namespace a::b::c { … }`.
- Prefer signed integers for values, counters, and arithmetic; use unsigned only for bit manipulation, intentional modular arithmetic, or API-required types (ES.101, ES.102).
- Prefer keeping method parameters on the same line; wrap them only when needed for readability.

# Tests
- Give each test a descriptive name that states the observable behavior being verified.
- Structure each test with `// Given`, `// When`, and `// Then` sections.
- Test one behavior per test and prefer one Given/When/Then sequence. Split tests that require multiple independent sequences.
- Put shared setup and cleanup in fixtures or focused helper functions, while keeping scenario-specific setup visible in the test.
- Keep actions in `When` and assertions in `Then`; setup precondition assertions may remain in `Given`.
