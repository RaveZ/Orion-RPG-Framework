// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "OrionGameplayTags.h"

/*
GAS
 */
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Damage, "Damage", "Damage Tag")

/*
Effects
*/
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Effects_HitReact, "Effects.HitReact", "Tag granted when Hit Reacting")

/*
Event
*/
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Event_Reward, "Event.Reward", "Event Reward Tag")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Event_AttributeUpgrade, "Event.AttributeUpgrade", "Event AttributeUpgrade Tag")


/*
Primary Attributes
 */
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attributes_Primary_Strength, "Attributes.Primary.Strength", "Increases physical damage")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attributes_Primary_Intelligence, "Attributes.Primary.Intelligence", "Increases magical damage")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attributes_Primary_Resilience, "Attributes.Primary.Resilience", "Increases Armor and Armor Penetration")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attributes_Primary_Vigor, "Attributes.Primary.Vigor", "Increases Health")

/*
Secondary Attributes
 */
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attributes_Secondary_Armor, "Attributes.Secondary.Armor", "Reduces damage taken, improves Block Chance")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attributes_Secondary_ArmorPenetration, "Attributes.Secondary.ArmorPenetration", "Ignores Percentage of enemy Armor, increases Critical Hit Chance")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attributes_Secondary_BlockChance, "Attributes.Secondary.BlockChance", "Chance to cut incoming damage in half")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attributes_Secondary_CriticalHitChance, "Attributes.Secondary.CriticalHitChance", "Chance to double damage plus critical hit bonus")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attributes_Secondary_CriticalHitDamage, "Attributes.Secondary.CriticalHitDamage", "Bonus damage added when a critical hit is scored")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attributes_Secondary_CriticalHitResistance, "Attributes.Secondary.CriticalHitResistance", "Reduces Critical Hit Chance of attacking enemies")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attributes_Secondary_HealthRegeneration, "Attributes.Secondary.HealthRegeneration", "Amount of Health regenerated every 1 second")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attributes_Secondary_ManaRegeneration, "Attributes.Secondary.ManaRegeneration", "Amount of Mana regenerated every 1 second")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attributes_Secondary_MaxHealth, "Attributes.Secondary.MaxHealth", "Maximum amount of Health obtainable")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attributes_Secondary_MaxMana, "Attributes.Secondary.MaxMana", "Maximum amount of Mana obtainable")

/*
Reward
*/
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Attributes_XP, "Attributes.XP", "XP Attribute Tag")


/*
 Input
*/
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Input_BasicAttackAction, "InputTag.BasicAttackAction", "Input Tag for Basic Attack Action")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Input_Ability1, "InputTag.Ability1", "Input Tag Ability 1 ")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Input_Ability2, "InputTag.Ability2", "Input Tag Ability 2 ")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Input_Ability3, "InputTag.Ability3", "Input Tag Ability 3 ")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Input_Ability4, "InputTag.Ability4", "Input Tag Ability 4 ")

/*
AbilitiesS
*/
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Tag_Abilities_Fire_Firebolt, "Abilities.Fire.Firebolt", "Firebolt Ability Tag")

/*
Cooldown
*/
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Tag_Cooldown_Fire_Firebolt, "Cooldown.Fire.Firebolt", "FireBolt Cooldown Tag")