
#pragma once

#include "GravityMovementComponent.h"
#include "CustomGravityManager.generated.h"


UCLASS()
class CUSTOMGRAVITYPLUGIN_API UCustomGravityManager : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UCustomGravityManager();

	/** Change Global Custom Gravity power. 
	* This change will affect all physics object using a CustomGravityComponent with GravityType set to "Global Custom Gravity". 
	*/
	UFUNCTION(BlueprintCallable, Category = "Global Custom Gravity", meta = (DisplayName = "Set Global Custom Gravity Power"))
		static void SetGlobalCustomGravityPower(float NewGravityPower);

	/** Change Global Custom Gravity direction. 
	* This change will affect all physics object using a CustomGravityComponent with GravityType set to "Global Custom Gravity". 
	*/
	UFUNCTION(BlueprintCallable, Category = "Global Custom Gravity", meta = (DisplayName = "Set Global Custom Gravity Direction"))
		static void SetGlobalCustomGravityDirection(const FVector& NewGravityDirection);

	/** Change Global Custom Gravity force mode. 
	* 	* This change will affect all physics object using a CustomGravityComponent with GravityType set to "Global Custom Gravity". 
	*/
	UFUNCTION(BlueprintCallable, Category = "Global Custom Gravity", meta = (DisplayName = "Set Global Custom Gravity Force Mode"))
		static void SetGlobalCustomGravityForceMode(EForceMode NewForceMode);

	/** Change Global Custom Gravity information. 
	* This change will affect all physics object using a CustomGravityComponent with GravityType set to "Global Custom Gravity". 
	*/
	UFUNCTION(BlueprintCallable, Category = "Global Custom Gravity", meta = (DisplayName = "Set Global Custom Gravity Info"))
		static void SetGlobalCustomGravityInfo(const FGravityInfo& NewGravityInfo);

	/** returns Global Custom Gravity power */
	UFUNCTION(BlueprintPure, Category = "Global Custom Gravity", meta = (DisplayName = "Global Custom Gravity Power"))
		static float GetGlobalCustomGravityPower();

	/** returns Global Custom Gravity direction */
	UFUNCTION(BlueprintPure, Category = "Global Custom Gravity", meta = (DisplayName = "Global Custom Gravity Direction"))
		static FVector GetGlobalCustomGravityDirection();

	/** returns Global Custom Gravity force mode */
	UFUNCTION(BlueprintPure, Category = "Global Custom Gravity", meta = (DisplayName = "Global Custom Gravity ForceMode"))
		static TEnumAsByte<EForceMode>  GetGlobalCustomGravityForceMode();

	/** returns Global Custom Gravity information */
	UFUNCTION(BlueprintPure, Category = "Global Custom Gravity", meta = (DisplayName = "Global Custom Gravity Info"))
		static FGravityInfo  GetGlobalCustomGravityInfo();

	/** Converts a GravityInfo struct value to a string */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToString (GravityInfo)", CompactNodeTitle = "->", BlueprintAutocast), Category = "Utilities|String")
		static FString Conv_GravityInfoToString(FGravityInfo InGravityInfo);

	/** Converts a GravityType enum value to a string */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToString (GravityType)", CompactNodeTitle = "->", BlueprintAutocast), Category = "Utilities|String")
		static FString Conv_GravityTypeToString(EGravityType InGravityType);

	/** Converts a ForceMode enum value to a string */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToString (ForceMode)", CompactNodeTitle = "->", BlueprintAutocast), Category = "Utilities|String")
		static FString Conv_ForceModeToString(EForceMode InForceMode);

protected:

	/** Global Custom Gravity Information. */
	static FGravityInfo GlobalCustomGravityInfo;

};
