// LAST CALL - module entry point.
// The phase clock owns the day loop; everything else subscribes to it
// (docs/ARCHITECTURE.md, section 2 "The day loop").

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FLastCallModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

DECLARE_LOG_CATEGORY_EXTERN(LogLastCall, Log, All);
