///////////////////////////////////////////////////////////////////////////
// ServiceDescriptorCustomization.cpp
///////////////////////////////////////////////////////////////////////////

// UnrealServiceLocatorEditor
#include "ServiceDescriptorCustomization.h"

// UnrealServiceLocator
#include "ServiceLocatorTypes.h"

// Engine
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailPropertyRow.h"
#include "Algo/Transform.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/SNullWidget.h"

///////////////////////////////////////////////////////////////////////////
#define LOCTEXT_NAMESPACE "ServiceDescriptorCustomization"
///////////////////////////////////////////////////////////////////////////

TSharedRef<IPropertyTypeCustomization> FServiceLocatorCustomization::MakeInstance()
{
	return MakeShared<FServiceLocatorCustomization>();
}

///////////////////////////////////////////////////////////////////////////

void FServiceLocatorCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	StructPropertyHandle = InStructPropertyHandle;
	StructPropertyHandle->SetOnPropertyResetToDefault(FSimpleDelegate::CreateSP(this, &FServiceLocatorCustomization::ConcreteTypeChangedHandler));

	BuildEditableDescriptors();

	TSharedPtr<IPropertyHandle> ServiceTypeHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FServiceDescriptor, ServiceType));
	if (ensure(ServiceTypeHandle.IsValid()))
	{
		ServiceTypeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FServiceLocatorCustomization::ConcreteTypeChangedHandler));

		HeaderRow
			.NameContent()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.0f, 0.0f, 4.0f, 0.0f)
				[
					StructPropertyHandle->CreatePropertyNameWidget()
				]
				+ SHorizontalBox::Slot()
				[
					ServiceTypeHandle->CreatePropertyValueWidget()
				]
			];
	}
}

///////////////////////////////////////////////////////////////////////////

void FServiceLocatorCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	TSharedPtr<IPropertyHandle> MappedTypesHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FServiceDescriptor, MappedTypes));
	if (ensure(MappedTypesHandle.IsValid()))
	{
		MappedTypesHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FServiceLocatorCustomization::ConcreteTypeChangedHandler));

		ChildBuilder.AddProperty(MappedTypesHandle.ToSharedRef())
			.CustomWidget()
			.NameContent()
			[
				MappedTypesHandle->CreatePropertyNameWidget()
			]
			.ValueContent()
			.MaxDesiredWidth(512)
			[
				SAssignNew(ClassTreeContainerWidget, SBorder)
				.Padding(FMargin(4.f))
				[
					SAssignNew(ClassTreeWidget, STreeView<FServiceLocatorTreeItemSharedPtr>)
					.TreeItemsSource(&ServiceLocatorTreeItems)
					.OnGenerateRow(this, &FServiceLocatorCustomization::OnGenerateRow)
					.OnGetChildren(this, &FServiceLocatorCustomization::OnGetChildren)
					//.OnExpansionChanged( this, &FServiceLocatorCustomization::OnExpansionChanged)
					.SelectionMode(ESelectionMode::Multi)
				]
			];

		RefreshTreeItems();
	}

	TSharedPtr<IPropertyHandle> LocateBehaviourHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FServiceDescriptor, LocateBehaviour));
	if (ensure(LocateBehaviourHandle.IsValid()))
	{
		ChildBuilder.AddProperty(LocateBehaviourHandle.ToSharedRef());
	}

	TSharedPtr<IPropertyHandle> DebugOnlyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FServiceDescriptor, bDebugOnly));
	if (ensure(DebugOnlyHandle.IsValid()))
	{
		ChildBuilder.AddProperty(DebugOnlyHandle.ToSharedRef());
	}
}

///////////////////////////////////////////////////////////////////////////

void FServiceLocatorCustomization::OnClassCheckStatusChanged(ECheckBoxState NewCheckState, FServiceLocatorTreeItemSharedPtr NodeChanged)
{
	UClass* NodeClass = NodeChanged->Class.Get();

	// Don't do anything
	if (NewCheckState == ECheckBoxState::Undetermined)
		return;

	if (NewCheckState == ECheckBoxState::Checked)
	{
		for (FServiceDescriptor* ServiceDescriptor : EditableDescriptors)
		{
			ServiceDescriptor->MappedTypes.AddUnique(NodeClass);
		}
	}
	else
	{
		for (FServiceDescriptor* ServiceDescriptor : EditableDescriptors)
		{
			ServiceDescriptor->MappedTypes.Remove(NodeClass);
		}
	}

	RefreshTreeItems();
}

///////////////////////////////////////////////////////////////////////////

ECheckBoxState FServiceLocatorCustomization::IsClassChecked(FServiceLocatorTreeItemSharedPtr Node) const
{
	UClass* NodeClass = Node->Class.Get();

	TOptional<bool> bIsChecked;
	for (FServiceDescriptor* ServiceDescriptor : EditableDescriptors)
	{
		const bool bContainsClass = ServiceDescriptor->MappedTypes.Contains(NodeClass);
		
		if (!bIsChecked.IsSet())
		{
			bIsChecked = bContainsClass;
			continue;
		}

		if (bIsChecked.GetValue() == bContainsClass)
			continue;

		return ECheckBoxState::Undetermined;
	}

	return bIsChecked.IsSet() ? (bIsChecked.GetValue() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked) : ECheckBoxState::Undetermined;
}

///////////////////////////////////////////////////////////////////////////

void FServiceLocatorCustomization::BuildEditableDescriptors()
{
	EditableDescriptors.Empty();

	if (StructPropertyHandle.IsValid())
	{
		TArray<void*> RawStructData;
		StructPropertyHandle->AccessRawData(RawStructData);

		for (void* RawStructDataElem : RawStructData)
		{
			EditableDescriptors.Add((FServiceDescriptor*)RawStructDataElem);
		}
	}
}

///////////////////////////////////////////////////////////////////////////

void FServiceLocatorCustomization::RefreshTreeItems()
{
	ServiceLocatorTreeItems.Reset();
	
	if (!InterfacesTreeItem.IsValid())
	{
		InterfacesTreeItem = MakeShared<FServiceLocatorTreeItem>();
		InterfacesTreeItem->DisplayText = FText(LOCTEXT("TreeItem_Interfaces", "Implemented Interfaces")).ToString();
	}
	InterfacesTreeItem->Children.Reset();

	if (!ConcreteClassesTreeItem.IsValid())
	{
		ConcreteClassesTreeItem = MakeShared<FServiceLocatorTreeItem>();
		ConcreteClassesTreeItem->DisplayText = FText(LOCTEXT("TreeItem_ConcreteClasses", "Concrete Classes")).ToString();
	}
	ConcreteClassesTreeItem->Children.Reset();

	// If we're customising multiple service descriptors, these are the common sets of classes to all descriptors
	TOptional<TSet<UClass*>> CommonInterfaceClasses;
	TOptional<TSet<UClass*>> CommonConcreteClasses;

	// These are the classes we're going to display, in some sort of order.
	TArray<UClass*> InterfaceClasses;
	TArray<UClass*> ConcreteClasses;

	for (FServiceDescriptor* ServiceDescriptor : EditableDescriptors)
	{
		TArray<UClass*> LocalInterfaceClasses;
		TArray<UClass*> LocalConcreteClasses;

		if (ServiceDescriptor == nullptr)
			continue;

		// The service type for this descriptor could be null
		UClass* ServiceType = ServiceDescriptor->ServiceType;
		if (ServiceType != nullptr)
		{
			for (const FImplementedInterface& ImplementedInterface : ServiceType->Interfaces)
			{
				LocalInterfaceClasses.AddUnique(ImplementedInterface.Class);
			}

			while (ServiceType != nullptr)
			{
				LocalConcreteClasses.AddUnique(ServiceType);
				ServiceType = ServiceType->GetSuperClass();
			}
		}

		for (UClass* MappedType : ServiceDescriptor->MappedTypes)
		{
			if (MappedType == nullptr)
				continue;

			if (MappedType->HasAllClassFlags(CLASS_Interface))
			{
				LocalInterfaceClasses.AddUnique(MappedType);
			}
			else
			{
				LocalConcreteClasses.AddUnique(MappedType);
			}
		}

		auto AppendUnique = [](TArray<UClass*>& InOut, const TArray<UClass*>& In)
		{
			InOut.Reserve(InOut.Num() + In.Num());
			for (UClass* InElem : In)
				InOut.AddUnique(InElem);
		};

		AppendUnique(InterfaceClasses, LocalInterfaceClasses);
		TSet<UClass*> LocalInterfaceClassesSet(MoveTemp(LocalInterfaceClasses));
		CommonInterfaceClasses = CommonInterfaceClasses.IsSet() ? CommonInterfaceClasses->Intersect(LocalInterfaceClassesSet) : MoveTemp(LocalInterfaceClassesSet);
		
		AppendUnique(ConcreteClasses, LocalConcreteClasses);
		TSet<UClass*> LocalParentClassesSet(MoveTemp(LocalConcreteClasses));
		CommonConcreteClasses = CommonConcreteClasses.IsSet() ? CommonConcreteClasses->Intersect(LocalParentClassesSet) : MoveTemp(LocalParentClassesSet);
	}

	if ((InterfaceClasses.Num() > 0) && ensure(CommonInterfaceClasses.IsSet()))
	{
		for (UClass* InterfaceClass : InterfaceClasses)
		{
			FServiceLocatorTreeItemSharedPtr NewChild = InterfacesTreeItem->Children.Emplace_GetRef(MakeShared<FServiceLocatorTreeItem>());
			NewChild->Class			= InterfaceClass;
			NewChild->DisplayText	= InterfaceClass->GetDisplayNameText().ToString();
			NewChild->Parent		= InterfacesTreeItem;
			NewChild->bEditable		= CommonInterfaceClasses->Contains(InterfaceClass);
		}
		ServiceLocatorTreeItems.Add(InterfacesTreeItem);
	}

	if ((ConcreteClasses.Num() > 0) && ensure(CommonConcreteClasses.IsSet()))
	{
		for (UClass* ConcreteClass : ConcreteClasses)
		{
			FServiceLocatorTreeItemSharedPtr NewChild = ConcreteClassesTreeItem->Children.Emplace_GetRef(MakeShared<FServiceLocatorTreeItem>());
			NewChild->Class			= ConcreteClass;
			NewChild->DisplayText	= ConcreteClass->GetDisplayNameText().ToString();
			NewChild->Parent		= ConcreteClassesTreeItem;
			NewChild->bEditable		= CommonConcreteClasses->Contains(ConcreteClass);
		}
		ServiceLocatorTreeItems.Add(ConcreteClassesTreeItem);
	}

	if (ClassTreeWidget.IsValid())
	{
		ClassTreeWidget->SetItemExpansion(InterfacesTreeItem, true);
		ClassTreeWidget->SetItemExpansion(ConcreteClassesTreeItem, true);
		ClassTreeWidget->RequestTreeRefresh();
	}
}

///////////////////////////////////////////////////////////////////////////

void FServiceLocatorCustomization::ConcreteTypeChangedHandler()
{
	RefreshTreeItems();
}

///////////////////////////////////////////////////////////////////////////

TSharedRef<ITableRow> FServiceLocatorCustomization::OnGenerateRow(FServiceLocatorTreeItemSharedPtr InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow< FServiceLocatorTreeItemSharedPtr>, OwnerTable)
		//.Style(FEditorStyle::Get(), "GameplayTagTreeView")
		[
			SNew( SHorizontalBox )
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Left)
			[
				SNew(SCheckBox)
				.OnCheckStateChanged(this, &FServiceLocatorCustomization::OnClassCheckStatusChanged, InItem)
				.IsChecked(this, &FServiceLocatorCustomization::IsClassChecked, InItem)
				//.ToolTipText(TooltipText)
				.Visibility(InItem->Parent.IsValid() ? EVisibility::Visible : EVisibility::Collapsed)
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.HAlign(HAlign_Left)
			[
				SNew(STextBlock)
				.Text(FText::FromString(InItem->DisplayText))
			]
		];
}

///////////////////////////////////////////////////////////////////////////

void FServiceLocatorCustomization::OnGetChildren(FServiceLocatorTreeItemSharedPtr InItem, TArray<FServiceLocatorTreeItemSharedPtr>& OutChildren)
{
	OutChildren = InItem->Children;
}

///////////////////////////////////////////////////////////////////////////
#undef LOCTEXT_NAMESPACE
///////////////////////////////////////////////////////////////////////////
