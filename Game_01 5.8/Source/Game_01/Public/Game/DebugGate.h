// Debug-only features (the K key, the test probes) must not be reachable by a player in a package.
#pragma once

#include "CoreMinimal.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace GameDebug
{
	/**
	 * True when debug keys may work: always in the editor, never in a Shipping build, and in a packaged
	 * Development build only when the game was started with -DevCheats.
	 */
	inline bool CheatsEnabled()
	{
#if UE_BUILD_SHIPPING
		return false;
#elif WITH_EDITOR
		return true;
#else
		static const bool bEnabled = FParse::Param(FCommandLine::Get(), TEXT("DevCheats"));
		return bEnabled;
#endif
	}

	/** True when the command line holds a test-probe flag (-PerfProbe, -OfferProbe and so on). Always false in Shipping. */
	inline bool HasProbeFlag()
	{
#if UE_BUILD_SHIPPING
		return false;
#else
		static const TCHAR* const Flags[] = { TEXT("WeaponShowcase"), TEXT("PauseProbe"), TEXT("EdgeProbe"), TEXT("BossProbe"),
			TEXT("ZoneShot"), TEXT("PerfProbe"), TEXT("OfferProbe"), TEXT("PaceProbe"), TEXT("ReelProbe") };
		for (const TCHAR* Flag : Flags)
		{
			if (FParse::Param(FCommandLine::Get(), Flag))
			{
				return true;
			}
		}
		FString Ignored;
		return FParse::Value(FCommandLine::Get(), TEXT("ReelProbe="), Ignored);
#endif
	}
}
