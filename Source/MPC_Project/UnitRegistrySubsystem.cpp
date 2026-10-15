// Fill out your copyright notice in the Description page of Project Settings.


#include "UnitRegistrySubsystem.h"
#include "SelectableInterface.h"

void UUnitRegistrySubsystem::RegisterUnit(AActor* UnitToRegister)
{
	if (IsValid(UnitToRegister) && UnitToRegister->Implements<USelectableInterface>())
	{
		SelectableUnits.AddUnique(UnitToRegister);
		UE_LOG(LogTemp, Log, TEXT("[UnitRegistrySubsystem] Unit Registered: %s (Total: %d)"),
			*UnitToRegister->GetName(), SelectableUnits.Num());
	}
}

void UUnitRegistrySubsystem::UnregisterUnit(AActor* UnitToUnregister)
{
	if (UnitToUnregister)
	{
		SelectableUnits.Remove(UnitToUnregister);
		UE_LOG(LogTemp, Log, TEXT("[UnitRegistrySubsystem] Unit Unregistered: %s (Total: %d)"),
			*UnitToUnregister->GetName(), SelectableUnits.Num());
	}
}
