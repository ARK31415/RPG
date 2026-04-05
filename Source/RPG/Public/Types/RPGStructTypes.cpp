#include "RPGStructTypes.h"
#include "AbilitySystem/Abilities/RPGPlayerGameplayAbility.h"

bool FRPGPlayerAbilitySet::IsValid() const
{
	return InputTag.IsValid() && AbilityToGrant;
}
