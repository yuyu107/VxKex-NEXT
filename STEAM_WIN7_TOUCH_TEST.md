# Steam on Windows 7: touch injection API compatibility test

> Experimental change on the `test` branch. This is **not** a release and does **not** implement actual touch injection.

## Background

Some newer SteamUI.dll versions statically import the Windows 8 functions:

- `USER32!InitializeTouchInjection`
- `USER32!InjectTouchInput`

These exports do not exist in the native Windows 7 USER32.dll. In this experiment, `KxUser.dll` exports compatibility stubs for both functions. Each stub returns `FALSE` and sets `GetLastError()` to `ERROR_CALL_NOT_IMPLEMENTED`.

The experiment aims to determine whether Windows 7 with VxKex can load an **unmodified** SteamUI.dll using these additional exports. Success on one machine does not establish full compatibility.

## Modified source files

- `01-Extended DLLs/KxUser/pointer.c` — defines the two API stubs.
- `01-Extended DLLs/KxUser/kxuser.def` — exports the two function names.

## Build

The existing `.github/workflows/build.yml` supports builds on the `test` branch. Open the **Actions** tab, select **Build, Release**, and inspect the build. Download the Debug/Release artifacts for evaluation; do not create a release tag for this experiment.

## Test precautions

1. Back up the original VxKex NEXT `KxUser.dll`, SteamUI.dll, and any Steam bootstrap configuration.
2. Test on a separate Windows 7 SP1 x64 installation or VM before using the main system.
3. Restore **unmodified** SteamUI.dll so the result tests KxUser alone.
4. Keep the known-good compatibility settings for Steam; do not change Steam service or overlay configuration as part of this API test.
5. Verify in Dependencies that both functions resolve, then launch Steam.
6. Record Steam version, OS version, startup result, downloads, game launch, and any service or overlay limitations.
7. If a regression occurs, stop Steam and restore the backed-up `KxUser.dll`.

**Known limitations:** The stubs do not synthesize touch events. Steam Client Service maintenance prompts and in-game overlay failures are outside the scope of this change.

## Validation criteria

A successful compilation is not sufficient. Confirm that an unmodified SteamUI loads and that basic desktop use works on Windows 7 before considering merging to `main`.
