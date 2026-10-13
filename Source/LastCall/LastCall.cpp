#include "LastCall.h"

DEFINE_LOG_CATEGORY(LogLastCall);

void FLastCallModule::StartupModule()
{
	UE_LOG(LogLastCall, Log, TEXT("LAST CALL runtime module started"));
}

void FLastCallModule::ShutdownModule()
{
	UE_LOG(LogLastCall, Log, TEXT("LAST CALL runtime module stopped"));
}

IMPLEMENT_PRIMARY_GAME_MODULE(FLastCallModule, LastCall, "LastCall");
