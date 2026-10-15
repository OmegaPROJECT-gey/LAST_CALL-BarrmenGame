// LAST CALL - the raid director wrapper. See header for the contract.

#include "Bar/ULCRaidDirector.h"

#include "Core/BalanceCore.h"
#include "LastCall.h"
#include "Misc/Paths.h"

namespace
{
	std::string ToUtf8(const FString& Text)
	{
		FTCHARToUTF8 Converter(*Text);
		return std::string(Converter.Get(), Converter.Length());
	}

	FString Utf8ToFString(const std::string& Text)
	{
		return FString(UTF8_TO_TCHAR(Text.c_str()));
	}
} // namespace

void ULCRaidDirector::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// The search's heat and fines come from the balance file; a refusal
	// keeps the compiled defaults the raid unit tests pin to that file.
	const FString BalancePath = FPaths::Combine(
		FPaths::ProjectDir(), TEXT("Data"), TEXT("Balance"), TEXT("core.json"));
	FTCHARToUTF8 PathConverter(*BalancePath);
	lc::BalanceData Balance;
	std::string BalanceError;
	if (lc::load_balance_file(std::string(PathConverter.Get(), PathConverter.Length()),
		Balance, &BalanceError))
	{
		Tuning = Balance.raid;
	}
	else
	{
		UE_LOG(LogLastCall, Warning, TEXT("Balance refused (%s) - raid tuning stays default"),
			UTF8_TO_TCHAR(BalanceError.c_str()));
	}

	// D3's pool decides when raids happen: the morning picks, we answer.
	ULCEventDirector* Events = Collection.InitializeDependency<ULCEventDirector>();
	if (Events)
	{
		Events->OnMorningEvent.AddDynamic(this, &ULCRaidDirector::HandleMorning);
		UE_LOG(LogLastCall, Verbose, TEXT("RaidDirector bound to the morning pool"));
	}
}

FLCRaidResult ULCRaidDirector::RunSearch(const TArray<FString>& Evidence,
	int32 PartnerCapacity, double DistractionScore)
{
	lc::RaidState State;
	State.evidence.reserve(Evidence.Num());
	for (const FString& Item : Evidence)
	{
		State.evidence.push_back(ToUtf8(Item));
	}
	State.partner_capacity = PartnerCapacity > 0 ? PartnerCapacity : 0;

	const lc::RaidResult Outcome =
		lc::resolve_raid(State, DistractionScore, Tuning);

	FLCRaidResult Row;
	Row.DistractionWorked = Outcome.distraction_worked;
	Row.HiddenByPartner = Outcome.hidden_by_partner;
	Row.Found = Outcome.found;
	Row.HeatDelta = Outcome.heat_delta;
	Row.Fine = Outcome.fine;

	// The search happened - today's schedule is spent either way.
	bRaidScheduled = false;
	OnRaidResolved.Broadcast(Row);
	return Row;
}

void ULCRaidDirector::HandleMorning(const FString& EventId, const FLCEventInfo& /*Event*/)
{
	const bool bScheduled = EventId == TEXT("raid");
	if (bScheduled && !bRaidScheduled)
	{
		UE_LOG(LogLastCall, Log, TEXT("The morning named a raid - today's search is on"));
	}
	bRaidScheduled = bScheduled;
	if (bRaidScheduled)
	{
		OnRaidScheduled.Broadcast();
	}
}
