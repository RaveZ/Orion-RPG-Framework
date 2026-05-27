// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "AttributeInfo.h"
#include "../../OrionLogChannels.h"

FOrionAttributeInfo UAttributeInfo::FindAttributeInfoForTag(const FGameplayTag& AttributeTag, bool bLogNotFound) const
{
	for (const FOrionAttributeInfo& Info : AttributeInformation)
	{
		if (Info.AttributeTag.MatchesTagExact(AttributeTag))
		{
			return Info;
		}
	}
 
	if (bLogNotFound)
	{
		UE_LOG(LogOrion, Error, TEXT("Can't find Info for AttributeTag [%s] on AttributeInfo [%s]."), *AttributeTag.ToString(),*GetNameSafe(this));
	}
 
	return FOrionAttributeInfo();
}
