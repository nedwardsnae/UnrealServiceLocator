///////////////////////////////////////////////////////////////////////////
// ServiceDescriptorCustomization.cpp
///////////////////////////////////////////////////////////////////////////

// UnrealServiceLocatorEditor
#include "ServiceDescriptorCustomization.h"

// UnrealServiceLocator
#include "ServiceLocatorTypes.h"

// Engine
#include "Algo/Transform.h"
#include "DetailWidgetRow.h"
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

	BuildEditableDescriptors();

	StructPropertyHandle->SetOnPropertyResetToDefault(FSimpleDelegate::CreateSP(this, &FServiceLocatorCustomization::ConcreteTypeChangedHandler));

	TSharedPtr<IPropertyHandle> ConcreteTypeHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FServiceDescriptor, ConcreteType));
	if (!ConcreteTypeHandle.IsValid())
		return;

	ConcreteTypeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FServiceLocatorCustomization::ConcreteTypeChangedHandler));

	HeaderRow
		.NameContent()
		[
			ConcreteTypeHandle->CreatePropertyValueWidget()
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
			ServiceDescriptor->InterfaceTypes.AddUnique(NodeClass);
		}
	}
	else
	{
		for (FServiceDescriptor* ServiceDescriptor : EditableDescriptors)
		{
			ServiceDescriptor->InterfaceTypes.Remove(NodeClass);
		}
	}
}

///////////////////////////////////////////////////////////////////////////

ECheckBoxState FServiceLocatorCustomization::IsClassChecked(FServiceLocatorTreeItemSharedPtr Node) const
{
	UClass* NodeClass = Node->Class.Get();

	TOptional<bool> bIsChecked;
	for (FServiceDescriptor* ServiceDescriptor : EditableDescriptors)
	{
		const bool bContainsClass = ServiceDescriptor->InterfaceTypes.Contains(NodeClass);
		
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

	if (!ParentClassesTreeItem.IsValid())
	{
		ParentClassesTreeItem = MakeShared<FServiceLocatorTreeItem>();
		ParentClassesTreeItem->DisplayText = FText(LOCTEXT("TreeItem_ParentClasses", "Parent Classes")).ToString();
	}
	ParentClassesTreeItem->Children.Reset();

	TOptional<TSet<UClass*>> CommonInterfaceClasses;
	TOptional<TSet<UClass*>> CommonParentClasses;

	TSet<UClass*> InterfaceClasses;
	TSet<UClass*> ParentClasses;

	for (FServiceDescriptor* ServiceDescriptor : EditableDescriptors)
	{
		TSet<UClass*> LocalInterfaceClasses;
		TSet<UClass*> LocalParentClasses;

		if (ServiceDescriptor == nullptr)
			continue;

		for (UClass* ServiceDescriptorInterfaceType : ServiceDescriptor->InterfaceTypes)
		{
			if (ServiceDescriptorInterfaceType == nullptr)
				continue;

			if (ServiceDescriptorInterfaceType->HasAllClassFlags(CLASS_Interface))
			{
				InterfaceClasses.Add(ServiceDescriptorInterfaceType);
			}
			else
			{
				ParentClasses.Add(ServiceDescriptorInterfaceType);
			}
		}

		UClass* ConcreteType = ServiceDescriptor->ConcreteType;
		if (ConcreteType == nullptr)
			continue;

		for (const FImplementedInterface& ImplementedInterface : ConcreteType->Interfaces)
		{
			LocalInterfaceClasses.Add(ImplementedInterface.Class);
		}

		while (ConcreteType != nullptr)
		{
			LocalParentClasses.Add(ConcreteType);
			ConcreteType = ConcreteType->GetSuperClass();
		}

		InterfaceClasses.Append(LocalInterfaceClasses);
		CommonInterfaceClasses = CommonInterfaceClasses.IsSet() ? CommonInterfaceClasses->Intersect(LocalInterfaceClasses) : MoveTemp(LocalInterfaceClasses);
		
		ParentClasses.Append(LocalParentClasses);
		CommonParentClasses = CommonParentClasses.IsSet() ? CommonParentClasses->Intersect(LocalParentClasses) : MoveTemp(LocalParentClasses);
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

	if ((ParentClasses.Num() > 0) && ensure(CommonParentClasses.IsSet()))
	{
		for (UClass* ParentClass : ParentClasses)
		{
			FServiceLocatorTreeItemSharedPtr NewChild = ParentClassesTreeItem->Children.Emplace_GetRef(MakeShared<FServiceLocatorTreeItem>());
			NewChild->Class			= ParentClass;
			NewChild->DisplayText	= ParentClass->GetDisplayNameText().ToString();
			NewChild->Parent		= ParentClassesTreeItem;
			NewChild->bEditable		= CommonParentClasses->Contains(ParentClass);
		}
		ServiceLocatorTreeItems.Add(ParentClassesTreeItem);
	}

	if (ClassTreeWidget.IsValid())
	{
		ClassTreeWidget->SetItemExpansion(InterfacesTreeItem, true);
		ClassTreeWidget->SetItemExpansion(ParentClassesTreeItem, true);
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
