// LAST CALL - the customer in the world (ARCHITECTURE.md, section 5).
//
// The engine half of F1's "one customer reacts": an actor component the
// customer body carries. It owns patience, shock and the mood float over
// the engine-independent CustomerReactionCore (the math and the reaction
// bands are unit-tested without Unreal); this class only hands the
// world's events - a spill, a served drink, a long queue, a heard story
// - to the core and reports what the customer shows back as delegates:
// the patience widget, the bark and the pose hang off them. Tuning rides
// in from Data/Balance/core.json ("customer"), the same section the
// reaction core's unit test keeps in sync with CustomerTuning; the two
// mood keys are this component's own (ARCHITECTURE 5: mood decays while
// nothing good happens, critical combos lift it).
//
// License: MIT (LICENSE).

#pragma once

#include "CoreMinimal.h"
#include "Bar/FLCPourSolver.h" // ELCComboQuality
#include "Components/ActorComponent.h"
#include "Customers/CustomerReactionCore.h"
#include "ULCCustomerComponent.generated.h"

/** What the customer shows right now (CustomerReactionCore::Reaction). */
UENUM(BlueprintType)
enum class ELCReaction : uint8
{
	Content,  // patience high, nothing wrong
	Neutral,  // waiting, patience middling
	Annoyed,   // patience low, still standing
	Shocked,  // hit by a spill, reeling
	WalkedOut // patience hit zero, or a burst spill - the visit is over
};

/** Reaction band changed (drives the widget, the bark and the pose). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLCReactionChanged, ELCReaction, Reaction);

/** Patience died or a burst spill: this customer leaves for good. Fired once. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLCWalkedOut);

/**
 * The customer body in the bar. Add it to the customer actor (or spawn
 * it from Blueprint); the world pushes events in - OnSpill,
 * OnDrinkServed, SetWaitingInQueue, OnListened - and the component
 * reports the show back through OnReactionChanged / OnWalkedOut. The
 * phase clock (or the spawning system) owns turning the tick on and off
 * with the customer's visit; a walked-out customer ignores everything.
 */
UCLASS(ClassGroup = (LastCall), meta = (BlueprintSpawnableComponent))
class LASTCALL_API ULCCustomerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULCCustomerComponent();

	// UActorComponent: read Data/Balance/core.json before the first tick.
	virtual void InitializeComponent() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	/** Which persona and faction this body is (Data/Customers ids). */
	UFUNCTION(BlueprintCallable, Category = "Customer")
	void Configure(const FString& PersonaId, const FString& FactionId);

	UFUNCTION(BlueprintPure, Category = "Customer")
	FString GetPersonaId() const { return PersonaId; }

	UFUNCTION(BlueprintPure, Category = "Customer")
	FString GetFactionId() const { return FactionId; }

	/** 1.0 fresh, 0.0 gone (what the patience widget fills from). */
	UFUNCTION(BlueprintPure, Category = "Customer")
	float GetPatience() const;

	/** 0..1: lifted by critical combos, drained while nothing good happens. */
	UFUNCTION(BlueprintPure, Category = "Customer")
	float GetMood() const { return static_cast<float>(Mood); }

	UFUNCTION(BlueprintPure, Category = "Customer")
	ELCReaction GetReaction() const { return ShownReaction; }

	/** True once the visit ended - the actor walks out on the delegate. */
	UFUNCTION(BlueprintPure, Category = "Customer")
	bool HasWalkedOut() const;

	/**
	 * The pour landed on them: millilitres burnt as patience, the shock
	 * window starts; a burst spill (balance "burst_walkout_spill_ml")
	 * ends the visit outright.
	 */
	UFUNCTION(BlueprintCallable, Category = "Customer")
	void OnSpill(double Millilitres);

	/**
	 * A drink arrived in front of them: the core moves patience by combo
	 * quality (GAME-DESIGN 3.4) and a critical lifts the mood by the
	 * balance bonus. Wrong drinks never reach here - the pour solver
	 * already punished them, and OrderCore decides what was even served.
	 */
	UFUNCTION(BlueprintCallable, Category = "Customer")
	void OnDrinkServed(ELCComboQuality Quality);

	/** A full queue drains patience at the harsher waiting rate. */
	UFUNCTION(BlueprintCallable, Category = "Customer")
	void SetWaitingInQueue(bool bWaiting) { bWaitingInQueue = bWaiting; }

	/** They got the story out (balance "patience_refill_on_listened"). */
	UFUNCTION(BlueprintCallable, Category = "Customer")
	void OnListened();

	/** Dialogue effects and story beats move the mood directly (clamped 0..1). */
	UFUNCTION(BlueprintCallable, Category = "Customer")
	void AddMood(double Delta);

	/** Reaction band changed (widget, bark, pose). */
	UPROPERTY(BlueprintAssignable, Category = "Customer")
	FOnLCReactionChanged OnReactionChanged;

	/** Patience died or a burst spill: this customer leaves for good. */
	UPROPERTY(BlueprintAssignable, Category = "Customer")
	FOnLCWalkedOut OnWalkedOut;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Customer")
	FString PersonaId;

	UPROPERTY(BlueprintReadOnly, Category = "Customer")
	FString FactionId;

	UPROPERTY(BlueprintReadOnly, Category = "Customer")
	ELCReaction ShownReaction = ELCReaction::Content;

	UPROPERTY(BlueprintReadOnly, Category = "Customer")
	float Mood = 0.5f;

private:
	/** Data/Balance/core.json "customer" -> the core's tuning + the mood keys. */
	void LoadTuning();

	lc::CustomerState State;
	lc::CustomerTuning Tuning;

	double MoodDecayPerSecond = 0.01;  // mood_decay_per_second_unattended
	double ListenedRefill = 0.25;       // patience_refill_on_listened
	bool bWaitingInQueue = false;
	bool bWalkedOutBroadcast = false;
};
