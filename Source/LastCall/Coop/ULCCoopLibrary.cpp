// LAST CALL - the co-op roles wrapper. See header for the contract.

#include "Coop/ULCCoopLibrary.h"

#include "LastCall.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace
{
	FString RolesFilePath()
	{
		return FPaths::Combine(FPaths::ProjectDir(), TEXT("Data"), TEXT("Coop"),
			TEXT("roles.json"));
	}

	FString Utf8ToFString(const std::string& Text)
	{
		return FString(UTF8_TO_TCHAR(Text.c_str()));
	}

	std::string ToUtf8(const FString& Text)
	{
		FTCHARToUTF8 Converter(*Text);
		return std::string(Converter.Get(), Converter.Length());
	}

	std::vector<std::string> ToUtf8Array(const TArray<FString>& Items)
	{
		std::vector<std::string> Converted;
		Converted.reserve(Items.Num());
		for (const FString& Item : Items)
		{
			Converted.push_back(ToUtf8(Item));
		}
		return Converted;
	}
} // namespace

void ULCCoopLibrary::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadRoles();
}

int32 ULCCoopLibrary::LoadRoles()
{
	const FString Path = RolesFilePath();
	FString Text;
	if (!FFileHelper::LoadFileToString(Text, *Path))
	{
		UE_LOG(LogLastCall, Warning, TEXT("Co-op roles file unreadable: %s"), *Path);
		return 0;
	}

	lc::RoleLibrary Fresh;
	std::string Error;
	if (!Fresh.load(ToUtf8(Text), &Error))
	{
		UE_LOG(LogLastCall, Warning, TEXT("Co-op roles refused: %s"), *Utf8ToFString(Error));
		return 0;
	}

	Library = std::move(Fresh);
	UE_LOG(LogLastCall, Log, TEXT("Co-op roles: %d roles loaded (%d-%d players)"),
		static_cast<int32>(Library.roles().size()), Library.min_players(),
		Library.max_players());
	return static_cast<int32>(Library.roles().size());
}

TArray<FLCRoleInfo> ULCCoopLibrary::GetRoles() const
{
	TArray<FLCRoleInfo> Rows;
	for (const lc::RoleSpec& Role : Library.roles())
	{
		FLCRoleInfo Row;
		Row.Id = Utf8ToFString(Role.id);
		Row.NameEn = Utf8ToFString(Role.name_en);
		Row.NameRu = Utf8ToFString(Role.name_ru);
		Row.JobEn = Utf8ToFString(Role.job_en);
		Row.JobRu = Utf8ToFString(Role.job_ru);
		for (const std::string& Action : Role.actions)
		{
			Row.Actions.Add(Utf8ToFString(Action));
		}
		Row.Crowd = Role.crowd;
		Rows.Add(Row);
	}
	return Rows;
}

TArray<FString> ULCCoopLibrary::GetRoleIds() const
{
	TArray<FString> Ids;
	for (const lc::RoleSpec& Role : Library.roles())
	{
		Ids.Add(Utf8ToFString(Role.id));
	}
	return Ids;
}

bool ULCCoopLibrary::CanRoleDo(const FString& RoleId, const FString& Action) const
{
	return Library.can(ToUtf8(RoleId), ToUtf8(Action));
}

FString ULCCoopLibrary::AssignBlocker(int32 PlayerCount,
	const TArray<FString>& RoleIds) const
{
	return Utf8ToFString(Library.assign_blocker(PlayerCount, ToUtf8Array(RoleIds)));
}

int32 ULCCoopLibrary::CrowdCapacity(const TArray<FString>& RoleIds) const
{
	return Library.crowd_capacity(ToUtf8Array(RoleIds));
}

double ULCCoopLibrary::CrowdLoad(int32 Crowd, const TArray<FString>& RoleIds) const
{
	return Library.crowd_load(Crowd, ToUtf8Array(RoleIds));
}

int32 ULCCoopLibrary::GetMinPlayers() const
{
	return Library.min_players();
}

int32 ULCCoopLibrary::GetMaxPlayers() const
{
	return Library.max_players();
}
