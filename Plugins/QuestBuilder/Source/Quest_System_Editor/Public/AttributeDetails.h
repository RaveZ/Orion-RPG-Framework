// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Layout/Visibility.h"
#include "Widgets/SWidget.h"
#include "Widgets/Views/STableViewBase.h"
#include "Widgets/Views/STableRow.h"
#include "IDetailCustomization.h"
#include "IPropertyTypeCustomization.h"
#include "PropertyEditorModule.h"

class IDetailLayoutBuilder;
class IPropertyHandle;
class IPropertyUtilities;
class SComboButton;
class SSearchBox;

struct FDataRegistryId;

class QUEST_SYSTEM_EDITOR_API FQuestPropertyDetails : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	FQuestPropertyDetails()
	{
	}

protected:

	virtual void CustomizeHeader( TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils ) override;
	virtual void CustomizeChildren( TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils ) override;              
	//Quest Handle Accessors
	void OnQuestIdChanged();
	void OnQuestIdPreChange();
	void OnNodeIdChanged();
	void OnNodeIdPreChange();

	FText GetQuestIDComboBoxContentText() const;
	void GetQuestIDNames(TArray<FName>& OutRows) const;


	TWeakPtr<IPropertyUtilities> PropertyUtilities;

	TSharedPtr<IPropertyHandle> QuestIdProperty;
	TSharedPtr<IPropertyHandle> NodeIdProperty;
	
	bool bSourceRefreshQueued;
};
