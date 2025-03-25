// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "../Data/AbilityInfo.h"
#include "../../OrionLogChannels.h"

FOrionAbilityInfo UAbilityInfo::FindAbilityInfoForTag(const FGameplayTag& AbilityTag, bool bLogNotFound) const
{
    for (const FOrionAbilityInfo& Info : AbilityInformation)
    {
        if (Info.AbilityTag == AbilityTag)
        {
            return Info;
        }
    }

    if (bLogNotFound)
    {
        UE_LOG(LogOrion, Error, TEXT("Can't find info for AbilityTag [%s] on AbilityInfo [%s]"), *AbilityTag.ToString(), *GetNameSafe(this));
    }

    return FOrionAbilityInfo();
}