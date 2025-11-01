# C++ Standard Requirements

## Project Standard
- **This project uses: C++20**

## Dependency C++ Standard Requirements

### Managed via Conan

| Library | Version | Minimum C++ Standard | Compatible with C++20? |
|---------|---------|----------------------|------------------------|
| **cpprestsdk** | 2.10.19 | C++11 | ✅ Yes |
| **catch2** | 3.5.3 | C++14 | ✅ Yes |
| **fmt** | 11.1.4 | C++11 | ✅ Yes |

### Bundled Dependencies

| Library | Version | Minimum C++ Standard | Notes |
|---------|---------|----------------------|-------|
| **fmt** (bundled) | Old | C++11 | ⚠️ Not C++20 compatible - use Conan version |
| **RxCpp** | (bundled) | C++11 | ✅ Header-only, works with C++20 |

## Summary

All dependencies are compatible with **C++20**:
- Minimum requirement across all: **C++14** (Catch2)
- Project uses: **C++20** ✅
- All libraries support C++20

## Notes

- The bundled `fmt` library is old and incompatible with C++20 (uses deprecated `std::result_of`)
- Always use Conan's fmt (11.1.4+) which is fully C++20 compatible
- If Conan is not available, the build will fall back to bundled fmt and may fail with C++20

