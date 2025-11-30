# power-supply
### Directories
- `/lib`: Third-party software libraries.
- `/scripts`: Helper Python and Linker scripts.
- `/src`: C sources for the embedded software.

### Acknowledgements
- Gregory Ling (https://github.com/glingy) for his Python stack analyzer, BMP detection script, precise ASM busy wait functions, and MIN/MAX/CLAMP macros
  - Stack analyzer modified by me to work with GCC
  - Small stack analyzer bugfixes by me
  - ASM busy wait function bugfixes by me
- Atmel for CMSIS headers, SVDs, linker script, and startup script. Used under Apache 2.0 License.
  - Linker script and startup script modified by me
- picolibc project for their compact and efficient libc, libg, and libm implementations. Used under BSD license.