#include "buildcfg.h"
#include "kxbasep.h"
#include <KexStrSafe.h>

/*
 * Roblox version-check diagnostics: record the actual values returned by the
 * existing wrappers without changing their behavior or the caller's last-error.
 */
STATIC volatile LONG RobloxBaseTraceBusy = 0;
STATIC volatile LONG RobloxBaseTraceEvents = 0;

STATIC VOID KxBaseTraceRobloxVersion(
    IN PCWSTR Api,
    IN PCVOID Caller,
    IN BOOL Success,
    IN ULONG Major,
    IN ULONG Minor,
    IN ULONG Build,
    IN DWORD TypeMask)
{
    ULONG OldLastError;
    PLDR_DATA_TABLE_ENTRY Entry;
    NTSTATUS Status;

    OldLastError = GetLastError();

    if (!KexData || !KexData->LogHandle ||
        !AshExeBaseNameIs(L"RobloxPlayerBeta.exe")) {
        SetLastError(OldLastError);
        return;
    }

    if (InterlockedCompareExchange(&RobloxBaseTraceBusy, 1, 0) != 0) {
        SetLastError(OldLastError);
        return;
    }

    if (InterlockedIncrement(&RobloxBaseTraceEvents) <= 128) {
        Entry = NULL;
        Status = LdrFindEntryForAddress(Caller, &Entry);
        if (NT_SUCCESS(Status) && Entry) {
            KexLogInformationEvent(
                L"[RobloxVersionTrace] %s caller=%p module=%wZ base=%p success=%lu returned=%lu.%lu.%lu mask=0x%08lx lastError=%lu",
                Api, Caller, &Entry->BaseDllName, Entry->DllBase,
                (ULONG) Success, Major, Minor, Build, TypeMask, OldLastError);
        } else {
            KexLogInformationEvent(
                L"[RobloxVersionTrace] %s caller=%p module=<unknown> success=%lu returned=%lu.%lu.%lu mask=0x%08lx lastError=%lu",
                Api, Caller, (ULONG) Success, Major, Minor, Build, TypeMask, OldLastError);
        }
    }

    InterlockedExchange(&RobloxBaseTraceBusy, 0);
    SetLastError(OldLastError);
}

KXBASEAPI DWORD WINAPI Ext_GetVersion(
	VOID)
{
	PCVOID Caller = ReturnAddress();
	DWORD Result = GetVersion();
	WORD Loword = LOWORD(Result);
	INT	Major = LOBYTE(Loword);
	INT Minor = HIBYTE(Loword);

	//
	// APPSPECIFICHACK: Java's "awt.dll" detects the version of Windows, and if it's
	// older than Windows 8, the touch functionality will be disabled.
	//
	unless(KexData->IfeoParameters.DisableAppSpecific) {
		if (AshModuleBaseNameIs(ReturnAddress(), L"awt.dll") &&
			Major == 6 && Minor < 2) {
			Minor = 2;
			Result = (HIWORD(Result) << 16) | MAKEWORD(Major, Minor);
		}
	}
	KxBaseTraceRobloxVersion(L"GetVersion", Caller, TRUE,
		LOBYTE(LOWORD(Result)), HIBYTE(LOWORD(Result)), HIWORD(Result), 0);
	return Result;
}

//
// Iertutil.dll contains a version check which will end up causing errors (failure
// to open files with ShellExecute including with shell context menus) if the reported
// OS version is higher than 6.1.
//
// This function un-spoofs the OS version for Windows DLLs, and preserves normal behavior
// for all other situations.
//

KXBASEAPI BOOL WINAPI Ext_GetVersionExA(
	IN OUT	POSVERSIONINFOA	VersionInfo)
{
	BOOL Success;
	PCVOID Caller = ReturnAddress();

	Success = GetVersionExA(VersionInfo);

	if (Success && (IsVMwareInstallationLauncher || AshModuleIsWindowsModule(ReturnAddress()))) {
		VersionInfo->dwMajorVersion = OriginalMajorVersion ? OriginalMajorVersion : 6;
		VersionInfo->dwMinorVersion = OriginalMinorVersion ? OriginalMinorVersion : 1;
		VersionInfo->dwBuildNumber = OriginalBuildNumber ? LOWORD(OriginalBuildNumber) : 7601;
	} else if (KexData->IfeoParameters.WinVerSpoof != WinVerSpoofNone) {
		switch (KexData->IfeoParameters.WinVerSpoof) {
		case WinVerSpoofWin7:
			VersionInfo->dwMajorVersion		= 6;
			VersionInfo->dwMinorVersion		= 1;
			VersionInfo->dwBuildNumber		= 7601;
			
			StringCchCopyA(
				VersionInfo->szCSDVersion,
				ARRAYSIZE(VersionInfo->szCSDVersion),
				"Service Pack 1");

			break;
		case WinVerSpoofWin8:
			VersionInfo->dwMajorVersion		= 6;
			VersionInfo->dwMinorVersion		= 2;
			VersionInfo->dwBuildNumber		= 9200;
			break;
		case WinVerSpoofWin8Point1:
			VersionInfo->dwMajorVersion		= 6;
			VersionInfo->dwMinorVersion		= 3;
			VersionInfo->dwBuildNumber		= 9600;
			break;
		case WinVerSpoofWin10:
			VersionInfo->dwMajorVersion		= 10;
			VersionInfo->dwMinorVersion		= 0;
			VersionInfo->dwBuildNumber		= 19045;
			break;
		case WinVerSpoofWin11:
		default:
			VersionInfo->dwMajorVersion		= 10;
			VersionInfo->dwMinorVersion		= 0;
			VersionInfo->dwBuildNumber		= 26200;
			break;
		}
	}

	KxBaseTraceRobloxVersion(L"GetVersionExA", Caller, Success,
		Success ? VersionInfo->dwMajorVersion : 0,
		Success ? VersionInfo->dwMinorVersion : 0,
		Success ? VersionInfo->dwBuildNumber : 0, 0);
	return Success;
}

KXBASEAPI BOOL WINAPI Ext_GetVersionExW(
	IN OUT	POSVERSIONINFOW	VersionInfo)
{
	BOOL Success;
	PCVOID Caller = ReturnAddress();

	Success = GetVersionExW(VersionInfo);

	if (Success && (IsVMwareInstallationLauncher || AshModuleIsWindowsModule(ReturnAddress()))) {
		VersionInfo->dwMajorVersion = OriginalMajorVersion ? OriginalMajorVersion : 6;
		VersionInfo->dwMinorVersion = OriginalMinorVersion ? OriginalMinorVersion : 1;
		VersionInfo->dwBuildNumber = OriginalBuildNumber ? LOWORD(OriginalBuildNumber) : 7601;
	} else if (KexData->IfeoParameters.WinVerSpoof != WinVerSpoofNone) {
		switch (KexData->IfeoParameters.WinVerSpoof) {
		case WinVerSpoofWin7:
			VersionInfo->dwMajorVersion		= 6;
			VersionInfo->dwMinorVersion		= 1;
			VersionInfo->dwBuildNumber		= 7601;
			
			StringCchCopy(
				VersionInfo->szCSDVersion,
				ARRAYSIZE(VersionInfo->szCSDVersion),
				L"Service Pack 1");

			break;
		case WinVerSpoofWin8:
			VersionInfo->dwMajorVersion		= 6;
			VersionInfo->dwMinorVersion		= 2;
			VersionInfo->dwBuildNumber		= 9200;
			break;
		case WinVerSpoofWin8Point1:
			VersionInfo->dwMajorVersion		= 6;
			VersionInfo->dwMinorVersion		= 3;
			VersionInfo->dwBuildNumber		= 9600;
			break;
		case WinVerSpoofWin10:
			VersionInfo->dwMajorVersion		= 10;
			VersionInfo->dwMinorVersion		= 0;
			VersionInfo->dwBuildNumber		= 19045;
			break;
		case WinVerSpoofWin11:
		default:
			VersionInfo->dwMajorVersion		= 10;
			VersionInfo->dwMinorVersion		= 0;
			VersionInfo->dwBuildNumber		= 26200;
			break;
		}
	}

	KxBaseTraceRobloxVersion(L"GetVersionExW", Caller, Success,
		Success ? VersionInfo->dwMajorVersion : 0,
		Success ? VersionInfo->dwMinorVersion : 0,
		Success ? VersionInfo->dwBuildNumber : 0, 0);
	return Success;
}

KXBASEAPI BOOL WINAPI Ext_VerifyVersionInfoW(
	IN	LPOSVERSIONINFOEXW	lpVersionInformation,
	IN	DWORD				dwTypeMask,
	IN	DWORDLONG			dwlConditionMask)
{
	BOOL Success;
	PCVOID Caller = ReturnAddress();

	if (AshExeBaseNameIs(L"RobloxStudioBeta.exe")) {
		Success = TRUE;
	} else {
		Success = VerifyVersionInfoW(lpVersionInformation, dwTypeMask, dwlConditionMask);
	}

	KxBaseTraceRobloxVersion(L"VerifyVersionInfoW", Caller, Success,
		lpVersionInformation ? lpVersionInformation->dwMajorVersion : 0,
		lpVersionInformation ? lpVersionInformation->dwMinorVersion : 0,
		lpVersionInformation ? lpVersionInformation->dwBuildNumber : 0,
		dwTypeMask);
	return Success;
}