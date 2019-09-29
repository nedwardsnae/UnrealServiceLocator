///////////////////////////////////////////////////////////////////////////
// ServiceDescriptorCustomization.h
///////////////////////////////////////////////////////////////////////////

#pragma once

// Engine
#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"
#include "Widgets/SWidget.h"
#include "Widgets/Views/STableViewBase.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/STreeView.h"

// UnrealServiceLocatorEditor
// ...

///////////////////////////////////////////////////////////////////////////

class FServiceLocatorCustomization : public IPropertyTypeCustomization
{
public:

	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	//////////////////////////////////////////////
	// Overridden Functions - IPropertyTypeCustomization

	virtual void CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, class IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override {}

private:

	struct FServiceLocatorTreeItem;

	using FServiceLocatorTreeItemSharedRef = TSharedRef<FServiceLocatorTreeItem>;
	using FServiceLocatorTreeItemSharedPtr = TSharedPtr<FServiceLocatorTreeItem>;
	using FServiceLocatorTreeItemWeakPtr = TSharedPtr<FServiceLocatorTreeItem>;

	struct FServiceLocatorTreeItem
	{
		TWeakObjectPtr<UClass>						Class;
		FString										DisplayText;
		TArray<FServiceLocatorTreeItemSharedPtr>	Children;
		FServiceLocatorTreeItemWeakPtr				Parent;
		bool										bEditable = false;
	};

	void OnClassCheckStatusChanged(ECheckBoxState NewCheckState, FServiceLocatorTreeItemSharedPtr NodeChanged);
	ECheckBoxState IsClassChecked(FServiceLocatorTreeItemSharedPtr Node) const;

	void BuildEditableDescriptors();
	void RefreshTreeItems();
	void ConcreteTypeChangedHandler();

	FServiceLocatorTreeItemSharedPtr InterfacesTreeItem;
	FServiceLocatorTreeItemSharedPtr ParentClassesTreeItem;

	TArray<struct FServiceDescriptor*> EditableDescriptors;

	TSharedRef<ITableRow> OnGenerateRow(FServiceLocatorTreeItemSharedPtr InItem, const TSharedRef<STableViewBase>& OwnerTable);
	void OnGetChildren(FServiceLocatorTreeItemSharedPtr InItem, TArray<FServiceLocatorTreeItemSharedPtr>& OutChildren);

	TSharedPtr<IPropertyHandle> StructPropertyHandle;

	TArray<FServiceLocatorTreeItemSharedPtr> ServiceLocatorTreeItems;

	/** Container widget holding the tag tree */
	TSharedPtr<SBorder> ClassTreeContainerWidget;

	/** Tree widget showing the gameplay tag library */
	TSharedPtr<STreeView<FServiceLocatorTreeItemSharedPtr>> ClassTreeWidget;

};