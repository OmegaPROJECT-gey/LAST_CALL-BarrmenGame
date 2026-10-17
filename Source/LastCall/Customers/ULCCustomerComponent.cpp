// LAST CALL - the customer component implementation. See header for the contract.

#include "Customers/ULCCustomerComponent.h"

#include "Core/JsonCore.h"
#include "LastCall.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace
{

	ELCReaction ToEngineReaction(lc::Reaction Reaction)
	{
		switch (Reaction)
		{
			case lc::Reaction::Content:   return ELCReaction::Content;
			case lc::Reaction::Neutral:   return ELCReaction::Neutral;
			case lc::Reaction::Annoyed:   return ELCReaction::Annoyed;
			case lc::Reaction::Shocked:   return ELCReaction::Shocked;
			case lc::Reaction::WalkedOut: return ELCReaction::WalkedOut;
		}
		return ELCReaction::Neutral;
	}

	lc::ComboQuality FromEngineQuality(ELCComboQuality Quality)
	{
		switch (Quality)
		{
			case ELCComboQuality::Ruined:   return lc::ComboQuality::Ruined;
			case ELCComboQuality::Fine:     return lc::ComboQuality::Fine;
			case ELCComboQuality::Good:     return lc::ComboQuality::Good;
			case ELCComboQuality::Critical: return lc::ComboQuality::Critical;
		}
		return lc::ComboQuality::Ruined;
	}

	double ClampMood(double Value)
	{
		return FMath::Clamp(Value, 0.0, 1.0);
	}

} // namespace

ULCCustomerComponent::ULCCustomerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULCCustomerComponent::InitializeComponent()
{
	Super::InitializeComponent();
	LoadTuning();
}

void ULCCustomerComponent::LoadTuning()
{
	const FString Path = FPaths::Combine(FPaths::ProjectDir(), TEXT("Data"),
		TEXT("Balance"), TEXT("core.json"));
	FString Text;
	if (!FFileHelper::LoadFileToString(Text, *Path))
	{
		UE_LOG(LogLastCall, Warning,
			TEXT("Balance file unreadable (%s) - the customer keeps the reaction core's defaults"),
			*Path);
		return;
	}

	FTCHARToUTF8 Converter(*Text);
	lc::JsonValue Doc;
	std::string Error;
	if (!lc::json_parse(std::string(Converter.Get(), Converter.Length()), Doc, &Error))
	{
		UE_LOG(LogLastCall, Warning,
			TEXT("Balance refused (%s) - the customer keeps the reaction core's defaults"),
			UTF8_TO_TCHAR(Error.c_str()));
		return;
	}

	const lc::JsonValue* Customer = Doc.find("customer");
	if (!Customer || Customer->kind != lc::JsonValue::Kind::Object)
	{
		UE_LOG(LogLastCall, Warning,
			TEXT("Balance has no `customer` section - the customer keeps the reaction core's defaults"));
		return;
	}

	// The ten keys the reaction core's unit test keeps in sync with
	// CustomerTuning; a missing key keeps the core's default, and the
	// mood pair below is this component's own (ARCHITECTURE 5).
	Customer->get_number("patience_drain_per_second_idle", Tuning.drain_idle_per_s);
	Customer->get_number("patience_drain_per_second_waiting", Tuning.drain_waiting_per_s);
	Customer->get_number("patience_refill_on_correct_order", Tuning.refill_correct_order);
	Customer->get_number("patience_penalty_ruined", Tuning.ruined_penalty);
	Customer->get_number("mood_bonus_on_critical_combo", Tuning.bonus_critical);
	Customer->get_number("spill_patience_per_ml", Tuning.spill_patience_per_ml);
	Customer->get_number("burst_walkout_spill_ml", Tuning.burst_walkout_spill_ml);
	Customer->get_number("shock_seconds", Tuning.shock_s);
	Customer->get_number("band_pleased", Tuning.band_pleased);
	Customer->get_number("band_annoyed", Tuning.band_annoyed);

	Customer->get_number("mood_decay_per_second_unattended", MoodDecayPerSecond);
	Customer->get_number("patience_refill_on_listened", ListenedRefill);
}

void ULCCustomerComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (State.walked_out)
	{
		return; // the visit is over; the actor leaves on the delegate
	}

	lc::tick(State, DeltaTime, bWaitingInQueue, Tuning);

	// The mood decays while nothing good happens (balance mood key).
	Mood = static_cast<float>(ClampMood(Mood - MoodDecayPerSecond * DeltaTime));

	const ELCReaction Now = ToEngineReaction(lc::reaction_for(State, Tuning));
	if (Now != ShownReaction)
	{
		ShownReaction = Now;
		OnReactionChanged.Broadcast(Now);
	}

	if (State.walked_out && !bWalkedOutBroadcast)
	{
		bWalkedOutBroadcast = true;
		ShownReaction = ELCReaction::WalkedOut;
		OnWalkedOut.Broadcast();
	}
}

void ULCCustomerComponent::Configure(const FString& InPersonaId, const FString& InFactionId)
{
	PersonaId = InPersonaId;
	FactionId = InFactionId;
}

float ULCCustomerComponent::GetPatience() const
{
	return static_cast<float>(State.patience);
}

bool ULCCustomerComponent::HasWalkedOut() const
{
	return State.walked_out;
}

void ULCCustomerComponent::OnSpill(double Millilitres)
{
	if (State.walked_out)
	{
		return;
	}
	lc::on_spill(State, Millilitres, Tuning);
}

void ULCCustomerComponent::OnDrinkServed(ELCComboQuality Quality)
{
	if (State.walked_out)
	{
		return;
	}
	lc::on_serve(State, FromEngineQuality(Quality), Tuning);
	if (Quality == ELCComboQuality::Critical)
	{
		Mood = static_cast<float>(ClampMood(Mood + Tuning.bonus_critical));
	}
}

void ULCCustomerComponent::OnListened()
{
	if (State.walked_out)
	{
		return;
	}
	State.patience = ClampMood(State.patience + ListenedRefill);
}

void ULCCustomerComponent::AddMood(double Delta)
{
	Mood = static_cast<float>(ClampMood(Mood + Delta));
}
