#include "Progression/RunLoadout.h"

FWeaponRow FRunLoadout::DefaultWeapon(EWeaponType Type)
{
	FWeaponRow Row;
	Row.WeaponType = Type;
	switch (Type)
	{
	case EWeaponType::Blade:
		Row.Damage = 15.0f; Row.DamagePerLevel = 8.0f; Row.Interval = 0.4f; Row.Range = 600.0f; Row.LineHalfWidth = 0.0f; Row.MaxLevel = 5;
		break;
	case EWeaponType::Bolt:
		Row.Damage = 20.0f; Row.DamagePerLevel = 10.0f; Row.Interval = 0.9f; Row.Range = 1200.0f; Row.LineHalfWidth = 80.0f; Row.MaxLevel = 5;
		break;
	case EWeaponType::Nova:
		Row.Damage = 25.0f; Row.DamagePerLevel = 12.0f; Row.Interval = 1.6f; Row.Range = 350.0f; Row.LineHalfWidth = 0.0f; Row.MaxLevel = 5;
		break;
	case EWeaponType::Chain:
		Row.Damage = 12.0f; Row.DamagePerLevel = 6.0f; Row.Interval = 1.1f; Row.Range = 700.0f; Row.LineHalfWidth = 0.0f; Row.MaxLevel = 5;
		Row.ChainJumps = 3; Row.ChainRange = 350.0f; Row.bStartsLocked = true;
		break;
	case EWeaponType::Gun:
		// Long reach on purpose: it is the answer to enemies closing in before the hero is strong.
		Row.Damage = 14.0f; Row.DamagePerLevel = 7.0f; Row.Interval = 0.7f; Row.Range = 1500.0f; Row.LineHalfWidth = 0.0f; Row.MaxLevel = 5;
		Row.ProjectileSpeed = 2600.0f; Row.ProjectileCount = 1;
		break;
	case EWeaponType::Orbit:
		// Range is the orbit radius; Interval is how often one enemy can be hurt by the balls again.
		Row.Damage = 10.0f; Row.DamagePerLevel = 5.0f; Row.Interval = 0.5f; Row.Range = 240.0f; Row.LineHalfWidth = 0.0f; Row.MaxLevel = 5;
		Row.ProjectileSpeed = 160.0f; Row.ProjectileCount = 2;
		break;
	default:
		break;
	}
	return Row;
}

FPassiveRow FRunLoadout::DefaultPassive(EPassiveType Type)
{
	FPassiveRow Row;
	Row.PassiveType = Type;
	switch (Type)
	{
	case EPassiveType::Swiftness: Row.MoveSpeedAdd = 0.10f; Row.MaxLevel = 5; break;
	case EPassiveType::Might:     Row.DamageAdd = 0.15f; Row.MaxLevel = 5; break;
	case EPassiveType::Vitality:  Row.MaxHealthAdd = 20.0f; Row.MaxLevel = 5; break;
	case EPassiveType::Greed:     Row.XpGainAdd = 0.25f; Row.MaxLevel = 5; break;
	case EPassiveType::Haste:     Row.AttackSpeedAdd = 0.12f; Row.MaxLevel = 5; Row.bStartsLocked = true; break;
	case EPassiveType::Magnet:    Row.MagnetRadiusAdd = 150.0f; Row.MaxLevel = 5; Row.bStartsLocked = true; break;
	case EPassiveType::Reach:     Row.RangeAdd = 0.12f; Row.MaxLevel = 5; break;
	case EPassiveType::Armor:     Row.DamageReductionAdd = 0.08f; Row.MaxLevel = 5; break;
	case EPassiveType::Regen:     Row.RegenPerSecondAdd = 1.0f; Row.MaxLevel = 5; break;
	case EPassiveType::Crit:      Row.CritChanceAdd = 0.08f; Row.CritMultiplier = 2.0f; Row.MaxLevel = 5; break;
	default: break;
	}
	return Row;
}

const TCHAR* FRunLoadout::WeaponRowName(EWeaponType Type)
{
	switch (Type)
	{
	case EWeaponType::Blade: return TEXT("Blade");
	case EWeaponType::Bolt:  return TEXT("Bolt");
	case EWeaponType::Nova:  return TEXT("Nova");
	case EWeaponType::Chain: return TEXT("Chain");
	case EWeaponType::Gun:   return TEXT("Gun");
	case EWeaponType::Orbit: return TEXT("Orbit");
	default: return TEXT("");
	}
}

const TCHAR* FRunLoadout::PassiveRowName(EPassiveType Type)
{
	switch (Type)
	{
	case EPassiveType::Swiftness: return TEXT("Swiftness");
	case EPassiveType::Might:     return TEXT("Might");
	case EPassiveType::Vitality:  return TEXT("Vitality");
	case EPassiveType::Greed:     return TEXT("Greed");
	case EPassiveType::Haste:     return TEXT("Haste");
	case EPassiveType::Magnet:    return TEXT("Magnet");
	case EPassiveType::Reach:     return TEXT("Reach");
	case EPassiveType::Armor:     return TEXT("Armor");
	case EPassiveType::Regen:     return TEXT("Regen");
	case EPassiveType::Crit:      return TEXT("Crit");
	default: return TEXT("");
	}
}

void FRunLoadout::LoadDefaults()
{
	for (uint8 W = 0; W < static_cast<uint8>(EWeaponType::Count); ++W)
	{
		const EWeaponType Type = static_cast<EWeaponType>(W);
		Weapons.Add(Type, DefaultWeapon(Type));
	}
	for (uint8 P = 0; P < static_cast<uint8>(EPassiveType::Count); ++P)
	{
		const EPassiveType Type = static_cast<EPassiveType>(P);
		Passives.Add(Type, DefaultPassive(Type));
	}
}

void FRunLoadout::ResetOwnership()
{
	WeaponLevels.Reset();
	WeaponLevels.Add(EWeaponType::Blade, 1); // the starter weapon
	PassiveStacks.Reset();
}

int32 FRunLoadout::AddWeaponCopy(EWeaponType Type)
{
	int32& Copies = WeaponLevels.FindOrAdd(Type);
	Copies = FMath::Min(Copies + 1, MaxCopies(Weapons.FindChecked(Type).MaxLevel));
	return Copies;
}

int32 FRunLoadout::AddPassiveCopy(EPassiveType Type)
{
	int32& Copies = PassiveStacks.FindOrAdd(Type);
	Copies = FMath::Min(Copies + 1, MaxCopies(Passives.FindChecked(Type).MaxLevel));
	return Copies;
}

void FRunLoadout::SetWeaponCopies(EWeaponType Type, int32 Copies)
{
	WeaponLevels.FindOrAdd(Type) = FMath::Clamp(Copies, 1, MaxCopies(Weapons.FindChecked(Type).MaxLevel));
}

bool FRunLoadout::IsWeaponAvailable(EWeaponType Type, const TArray<FName>& Unlocked) const
{
	return FOptionUnlock::IsAvailable(Weapons.FindChecked(Type).bStartsLocked, WeaponRowName(Type), Unlocked);
}

bool FRunLoadout::IsPassiveAvailable(EPassiveType Type, const TArray<FName>& Unlocked) const
{
	return FOptionUnlock::IsAvailable(Passives.FindChecked(Type).bStartsLocked, PassiveRowName(Type), Unlocked);
}

int32 FRunLoadout::PoolSize(const TArray<FName>& Unlocked) const
{
	int32 Count = 0;
	for (uint8 W = 0; W < static_cast<uint8>(EWeaponType::Count); ++W)
	{
		Count += IsWeaponAvailable(static_cast<EWeaponType>(W), Unlocked) ? 1 : 0;
	}
	for (uint8 P = 0; P < static_cast<uint8>(EPassiveType::Count); ++P)
	{
		Count += IsPassiveAvailable(static_cast<EPassiveType>(P), Unlocked) ? 1 : 0;
	}
	return Count;
}

float FRunLoadout::PassiveTotal(EPassiveType Passive, float PerStack) const
{
	const int32 Stacks = PassiveStacks.FindRef(Passive);
	return static_cast<float>(Stacks) * PerStack * Stars.PassiveMultiplier(Stars.StarsForCopies(Stacks));
}

float FRunLoadout::MoveSpeedMultiplier() const
{
	return 1.0f + PassiveTotal(EPassiveType::Swiftness, Passives.FindChecked(EPassiveType::Swiftness).MoveSpeedAdd);
}

float FRunLoadout::DamageMultiplier() const
{
	return 1.0f + PassiveTotal(EPassiveType::Might, Passives.FindChecked(EPassiveType::Might).DamageAdd);
}

float FRunLoadout::MaxHealthBonus() const
{
	return PassiveTotal(EPassiveType::Vitality, Passives.FindChecked(EPassiveType::Vitality).MaxHealthAdd);
}

float FRunLoadout::XpGainMultiplier() const
{
	return 1.0f + PassiveTotal(EPassiveType::Greed, Passives.FindChecked(EPassiveType::Greed).XpGainAdd);
}

float FRunLoadout::AttackSpeedMultiplier() const
{
	return 1.0f + PassiveTotal(EPassiveType::Haste, Passives.FindChecked(EPassiveType::Haste).AttackSpeedAdd);
}

float FRunLoadout::MagnetRadius() const
{
	return BaseMagnetRadius + PassiveTotal(EPassiveType::Magnet, Passives.FindChecked(EPassiveType::Magnet).MagnetRadiusAdd);
}

float FRunLoadout::RangeMultiplier() const
{
	return 1.0f + PassiveTotal(EPassiveType::Reach, Passives.FindChecked(EPassiveType::Reach).RangeAdd);
}

float FRunLoadout::DamageReduction() const
{
	return FMath::Clamp(PassiveTotal(EPassiveType::Armor, Passives.FindChecked(EPassiveType::Armor).DamageReductionAdd), 0.0f, 0.75f);
}

float FRunLoadout::RegenPerSecond() const
{
	return PassiveTotal(EPassiveType::Regen, Passives.FindChecked(EPassiveType::Regen).RegenPerSecondAdd);
}

float FRunLoadout::CritChance() const
{
	return FMath::Clamp(PassiveTotal(EPassiveType::Crit, Passives.FindChecked(EPassiveType::Crit).CritChanceAdd), 0.0f, 0.9f);
}

float FRunLoadout::CritMultiplier() const
{
	return Passives.FindChecked(EPassiveType::Crit).CritMultiplier;
}
