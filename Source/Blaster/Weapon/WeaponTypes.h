#pragma once

#define TRACE_LENGTH 1000000.f

#define CUSTOM_DEPTH_PURPLE 250
#define CUSTOM_DEPTH_BLUE 251
#define CUSTOM_DEPTH_TAN 252

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    EWT_AssaultRifle UMETA(DisplayNames = "Assault Riffle"),
    EWT_RocketLauncher UMETA(DisplayNames = "Rocket Launcher"),
    EWT_Pistol UMETA(DisplayNames = "Pistol"),
    EWT_Shotgun UMETA(DisplayNames = "Shotgun"),
    EWT_SniperRifle UMETA(DisplayNames = "SniperRifle"),
    EWT_GrenadeLauncher UMETA(DisplayNames = "GrenadeLauncher"),
    EWT_MAX UMETA(DisplayNames = "DefaultMAX")
};
