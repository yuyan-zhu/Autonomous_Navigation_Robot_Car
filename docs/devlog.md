# Dev Log

## 2026-11-01

**Done**
- Installed VS Code + Raspberry Pi Pico extension; blink + USB serial (`hello N`) working
- Set up the project directory layout; moved `main.c` into `firmware/src/`
- Created GitHub repo and pushed the first commit

**Problem → Cause → Fix**
- Compile failed after deleting `build/` ("No such file or directory")
  → the Compile button only runs ninja; it doesn't recreate the build folder
  → ran "Raspberry Pi Pico: Configure CMake", then compiled again
- After moving `main.c` into `src/`, the build would break
  → CMakeLists.txt still pointed to the old path
  → changed to `add_executable(firmware src/main.c)` and added `include/` to include dirs
- Commits were not linked to my GitHub account (gray avatar)
  → I typed the email with Chinese curly quotes “ ”, so they became part of the email
  → reset `user.email` without quotes, then `git commit --amend --reset-author` and `git push --force`

**Learned**
- Always use the English keyboard in the terminal; Chinese quotes look similar but are different characters
- Git basics: add → commit → push

**Next**
- Step 2: hardware inventory (pins, encoder type, measurements) → `docs/hardware.md`