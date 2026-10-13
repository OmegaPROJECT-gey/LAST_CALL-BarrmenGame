// LAST CALL - editor module entry point.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FLastCallToolsModule : public IModuleInterface
{
public:
	virtual void StartupModule() override {}
	virtual void ShutdownModule() override {}
};
