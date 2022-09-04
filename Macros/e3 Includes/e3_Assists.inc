|--------------------------------------------------------------------------------|
|START SETUP AREA
|--------------------------------------------------------------------------------|
Sub assist_Setup
/call iniToVarV "${advSettings_Ini},Debug,Debug Assists (On/Off)" Debug_Assists bool outer
/if (${Debug} || ${Debug_Assists}) /echo |- assist_Setup ==>

  |map lookup for targets to DoT
  /declare DoT_Target_Map map outer
  |map lookup for entries to remove from dot lookup
  /declare DoT_Target_Map_Removal map outer
  |map lookup for targets to Debuff
  /declare Debuff_Target_Map map outer
  |map lookup for targets to remove from debuff lookup
  /declare Debuff_Target_Map_Removal map outer

	/declare TogglePvP bool outer FALSE
	/declare Assisting bool outer FALSE
	/declare AllowControl bool outer FALSE
  /declare AssistTarget int outer 0
	/declare AssistStickDistance int outer
	/declare PauseTarget string outer
|	/declare AutoAggro bool outer TRUE
	/declare use_Nukes bool outer TRUE
	/declare use_TargetAE bool outer FALSE
	/declare use_PBAE bool outer FALSE
	/declare use_AssistDots bool outer TRUE
  /declare use_CommandDots bool outer TRUE

  /declare use_FULLBurns bool outer FALSE
  /declare use_LONGBurns bool outer FALSE
  /declare use_QUICKBurns bool outer FALSE
  /declare use_EPICBurns bool outer FALSE
  /declare use_Swarms bool outer FALSE

  /declare resistCount string outer
	/declare isEnraged bool outer FALSE
	/declare revertRngToMelee bool outer FALSE
	/declare pauseRanged bool outer FALSE
  /declare aeTarget int outer
  /declare offAssistMaxAggro int outer 0
  /if (!${Defined[spellsToUse]})   /declare spellsToUse string global Main

| ------------------------------------- Import Macro_Ini Settings.
	/call iniToVarV "${genSettings_Ini},Assists,Max Engage Distance"     MaxAssistDistance int outer
	/call iniToVarV "${genSettings_Ini},Assists,AE Threat Range"         AE_Radius int outer
	/call iniToVarV "${genSettings_Ini},Assists,Acceptable Target Types" AcceptableTargetTypes string outer
	/call iniToVarV "${genSettings_Ini},Assists,Auto-Assist (On/Off)"    auto_AssistOn bool outer
  /call iniToVarV "${genSettings_Ini},Assists,Long Term Debuff Recast(s)" longTermRecastTime int outer
  /call iniToVarV "${genSettings_Ini},Assists,Short Term Debuff Recast(s)" shortTermRecastTime int outer

| ------------------------------------- Import Character_Ini Settings.
	/if (${Ini[${Character_Ini},Assist Settings,Assist Type (Melee/Ranged/Off)].Length})  /call iniToVarV "${Character_Ini},Assist Settings,Assist Type (Melee/Ranged/Off)" AssistType string outer
  /if (${Ini[${Character_Ini},Assist Settings,Taunt(On/Off)].Length})                   /call iniToVarV "${Character_Ini},Assist Settings,Taunt(On/Off)" doTaunt bool outer
  /if (${Ini[${Character_Ini},Assist Settings,SmartTaunt(On/Off)].Length})              /call iniToVarV "${Character_Ini},Assist Settings,SmartTaunt(On/Off)" smartTaunt bool outer
	/if (${Ini[${Character_Ini},Assist Settings,Melee Stick Point].Length})               /call iniToVarV "${Character_Ini},Assist Settings,Melee Stick Point" AssistStickPoint string outer
	/if (${Ini[${Character_Ini},Assist Settings,Melee Distance].Length})                  /call iniToVarV "${Character_Ini},Assist Settings,Melee Distance" AssistDistanceSetting string outer
	/if (${Ini[${Character_Ini},Assist Settings,Ranged Distance].Length})                 /call iniToVarV "${Character_Ini},Assist Settings,Ranged Distance" RangedStickDistance string outer
	/if (${Ini[${Character_Ini},Assist Settings,Auto-Assist Engage Percent].Length})      /call iniToVarV "${Character_Ini},Assist Settings,Auto-Assist Engage Percent" EngagePct int outer
	/if (!${Defined[EngagePct]}) {
		/if (${Debug} || ${Debug_Assists}) /echo ERROR: @[EngagePct] - EngagePct is invalid, defaulting to 98%
		/declare EngagePct int outer 98
	} else /if (${Defined[EngagePct]} && !${EngagePct}) {
		/if (${Debug} || ${Debug_Assists}) /echo ERROR: @[EngagePct] - EngagePct is invalid, defaulting to 98%
		/varset EngagePct 98
	}

  | Epic Definitions for EpicBurn -exclude defensive or buffs
  /declare EpicName string outer
  /if (${FindItemCount[=Prismatic Dragon Blade]})                /varset EpicName Prismatic Dragon Blade
  /if (${FindItemCount[=Blade of Vesagran]})                     /varset EpicName Blade of Vesagran
  /if (${FindItemCount[=Raging Taelosian Alloy Axe]})            /varset EpicName Raging Taelosian Alloy Axe
  /if (${FindItemCount[=Vengeful Taelosian Blood Axe]})          /varset EpicName Vengeful Taelosian Blood Axe
  /if (${FindItemCount[=Staff of Living Brambles]})              /varset EpicName Staff of Living Brambles
  /if (${FindItemCount[=Staff of Everliving Brambles]})          /varset EpicName Staff of Everliving Brambles
  /if (${FindItemCount[=Fistwraps of Celestial Discipline]})     /varset EpicName Fistwraps of Celestial Discipline
  /if (${FindItemCount[=Transcended Fistwraps of Immortality]})  /varset EpicName Transcended Fistwraps of Immortality
  /if (${FindItemCount[=Redemption]})                            /varset EpicName Redemption
  /if (${FindItemCount[=Nightbane, Sword of the Valiant]})       /varset EpicName Nightbane, Sword of the Valiant
  /if (${FindItemCount[=Heartwood Blade]})                       /varset EpicName Heartwood Blade
  /if (${FindItemCount[=Heartwood Infused Bow]})                 /varset EpicName Heartwood Infused Bow
  /if (${FindItemCount[=Aurora, the Heartwood Blade]})           /varset EpicName Aurora, the Heartwood Blade
  /if (${FindItemCount[=Aurora, the Heartwood Bow]})             /varset EpicName Aurora, the Heartwood Bow
  /if (${FindItemCount[=Fatestealer]})                           /varset EpicName Fatestealer
  /if (${FindItemCount[=Nightshade, Blade of Entropy]})          /varset EpicName Nightshade, Blade of Entropy
  /if (${FindItemCount[=Innoruuk's Voice]})                      /varset EpicName Innoruuk's Voice
  /if (${FindItemCount[=Innoruuk's Dark Blessing]})              /varset EpicName Innoruuk's Dark Blessing
  /if (${FindItemCount[=Crafted Talisman of Fates]})             /varset EpicName Crafted Talisman of Fates
  /if (${FindItemCount[=Blessed Spiritstaff of the Heyokah]})    /varset EpicName Blessed Spiritstaff of the Heyokah
  /if (${FindItemCount[=Staff of Prismatic Power]})              /varset EpicName Staff of Prismatic Power
  /if (${FindItemCount[=Staff of Phenomenal Power]})             /varset EpicName Staff of Phenomenal Power
  /if (${FindItemCount[=Soulwhisper]})                           /varset EpicName Soulwhisper
  /if (${FindItemCount[=Deathwhisper]})                          /varset EpicName Deathwhisper
  /if (${EpicName.Length})     /call BuildArray EPICBurns "${EpicName}"

  /declare AnguishChest string outer
  /if (${FindItemCount[=Bladewhisper Chain Vest of Journeys]})                /varset AnguishChest Bladewhisper Chain Vest of Journeys
  /if (${FindItemCount[=Farseeker's Plate Chestguard of Harmony]})            /varset AnguishChest Farseeker's Plate Chestguard of Harmony
  /if (${FindItemCount[=Wrathbringer's Chain Chestguard of the Vindicator]})  /varset AnguishChest Wrathbringer's Chain Chestguard of the Vindicator
  /if (${FindItemCount[=Savagesoul Jerkin of the Wilds]})                     /varset AnguishChest Savagesoul Jerkin of the Wilds
  /if (${FindItemCount[=Glyphwielder's Tunic of the Summoner]})               /varset AnguishChest Glyphwielder's Tunic of the Summoner
  /if (${FindItemCount[=Whispering Tunic of Shadows]})                        /varset AnguishChest Whispering Tunic of Shadows
  /if (${FindItemCount[=Ritualchanter's Tunic of the Ancestors]})             /varset AnguishChest Ritualchanter's Tunic of the Ancestors
  /if (${AnguishChest.Length}) /call BuildArray EPICBurns "${AnguishChest}"

  |/echo ---------Assist calling spellarrays ----------
  /call assist_SpellArrays

  | Setup a spammable spell for spell procs such as deep sleep and languid bite
  /if (!${Defined[spammableSpell]}) {
    /call iniToVarV "${Character_Ini},Misc,Spammable Spell" spammableSpell string outer
    /if (${Bool[${spammableSpell}]}) {
      /echo I can now spam ${spammableSpell}, please use a /CheckFor to stop once the proc spell is on
    }
  }

  /if (${Debug} || ${Debug_Assists}) /echo <== assist_Setup -|
/return

|------------------------------------------------------------------------------------------------
|------------------------------------------------------------------------------------------------
SUB assist_SpellArrays
  /call IniToArrayV "${Character_Ini},Melee Abilities,Ability#"         MyAbilities
  /call IniToArrayV "${Character_Ini},Nukes,${spellsToUse}#"            MyNukes
  /call IniToArrayV "${Character_Ini},DoTs on Assist,${spellsToUse}#"   Assist_DoTs
  /call IniToArrayV "${Character_Ini},DoTs on Command,${spellsToUse}#"  Command_DoTs
  /call IniToArrayV "${Character_Ini},Debuffs,Debuff on Assist#"        Assist_Debuffs
  /call IniToArrayV "${Character_Ini},Debuffs,Debuff on Command#"       Command_Debuffs
  /call IniToArrayV "${Character_Ini},TargetAE,TargetAE#"               TargetAE_Spells
  /call IniToArrayV "${Character_Ini},PBAE,PBAE#"                       PBAE_Spells
  /call IniToArrayV "${Character_Ini},Burn,Quick Burn#"                 QUICKBurns
  /call IniToArrayV "${Character_Ini},Burn,Long Burn#"                  LONGBurns
  /call IniToArrayV "${Character_Ini},Burn,Full Burn#"                  FULLBurns
  /call IniToArrayV "${Character_Ini},Off Assist Spells,${spellsToUse}#"            OffAssistSpells

/if (${Me.Class.ShortName.Equal[MAG]}) {
	/if (!${Defined[summonMoltenOrb]}) /declare summonMoltenOrb bool outer FALSE
  /if (${Bool[${Me.AltAbility[Servant of Ro]}]})        /call BuildArray "swarmPets" "Servant of Ro"
  /if (${Bool[${Me.AltAbility[Host of the Elements]}]}) /call BuildArray "swarmPets" "Host of the Elements"

  /if (${Defined[MyNukes]}) {
    /declare i int local
    /for i 1 to ${MyNukes.Size}
 /if (${MyNukes.Size}==0) /break
      /if (${MyNukes[${i}].Find[Lava Orb]}) /varset summonMoltenOrb TRUE
    /next i
  }

  } else /if (${Me.Class.ShortName.Equal[NEC]}) {
    /if (${Bool[${Me.AltAbility[Swarm of Decay]}]}) /call BuildArray "swarmPets" "Swarm of Decay"
	  /if (${Bool[${Me.AltAbility[Rise of Bones]}]})  /call BuildArray "swarmPets" "Rise of Bones"
    /if (${Bool[${FindItem[Graverobber's Icon]}]}) /call BuildArray "swarmPets" "Graverobber's Icon"
    /if (${Bool[${FindItem[Soulwhisper]}]}) /call BuildArray "swarmPets" "Soulwhisper"
    /if (${Bool[${FindItem[Deathwhisper]}]}) /call BuildArray "swarmPets" "Deathwhisper"
    /if (${Bool[${Me.AltAbility[Wake the Dead]}]})  /call BuildArray "swarmPets" "Wake the Dead"
  
  } else /if (${Me.Class.ShortName.Equal[SHM]}) {
    /if (${Bool[${Me.AltAbility[Spirit Call]}]})    /call BuildArray "swarmPets" "Spirit Call"
    /if (${Bool[${FindItem[Shattered Gnoll Slayer]}]})    /call BuildArray "swarmPets" "Shattered Gnoll Slayer"
  
  } else /if (${Me.Class.ShortName.Equal[WIZ]}) {
    /if (${Bool[${Me.AltAbility[Call of Xuzl]}]})   /call BuildArray "swarmPets" "Call of Xuzl"
  
  } else /if (${Me.Class.ShortName.Equal[BRD]}) {
    /if (${Bool[${Me.AltAbility[Song of Stone]}]})   /call BuildArray "swarmPets" "Song of Stone"
     /if (${Bool[${FindItem[Tarnished Skeleton Key]}]}) /call BuildArray "swarmPets" "Tarnished Skeleton Key"
  
  } else /if (${Me.Class.ShortName.Equal[CLR]}) {
    /if (${Bool[${Me.AltAbility[Celestial Hammer]}]}) /call BuildArray "swarmPets" "Celestial Hammer"
    /if (${Bool[${FindItem[Graverobber's Icon]}]}) /call BuildArray "swarmPets" "Graverobber's Icon"
  
  } else /if (${Me.Class.ShortName.Equal[WAR]}) {
    /if (${Bool[${FindItem[Battered Smuggler's Barrel]}]}) /call BuildArray "swarmPets" "Battered Smuggler's Barrel"
 
  } else /if (${Me.Class.ShortName.Equal[BER]}) {
    /if (${Bool[${FindItem[Battered Smuggler's Barrel]}]}) /call BuildArray "swarmPets" "Battered Smuggler's Barrel"
 
  } else /if (${Me.Class.ShortName.Equal[ENC]}) {
 
    /if (${Bool[${Me.AltAbility[Doppelganger]}]}) /call BuildArray "swarmPets" "Doppelganger"
    /if (${Bool[${Me.AltAbility[Phantasmal Opponent]}]}) /call BuildArray "swarmPets" "Phantasmal Opponent"
    /if (${Bool[${FindItem[Tarnished Skeleton Key]}]}) /call BuildArray "swarmPets" "Tarnished Skeleton Key"
 
  } else /if (${Me.Class.ShortName.Equal[PAL]}) {
    /if (${Bool[${Me.AltAbility[Projection of Piety]}]}) /call BuildArray "swarmPets" "Projection of Piety"
  
  } else /if (${Me.Class.ShortName.Equal[DRU]}) {
    /if (${Bool[${Me.AltAbility[Spirits of Nature]}]}) /call BuildArray "swarmPets" "Spirits of Nature"
    /if (${Bool[${Me.AltAbility[Nature's Guardian]}]}) /call BuildArray "swarmPets" "Nature's Guardian"
    /if (${Bool[${FindItem[Shattered Gnoll Slayer]}]}) /call BuildArray "swarmPets" "Shattered Gnoll Slayer"
  }

  /if (${Defined[MyAbilities]})		    /call BuildSpellArray "MyAbilities"	      "Abilities2D"
  /if (${Defined[MyNukes]})		        /call BuildSpellArray "MyNukes"	          "Nukes2D"
  /if (${Defined[PBAE_Spells]})		    /call BuildSpellArray "PBAE_Spells"	      "PBAE_Spells2D"
  /if (${Defined[TargetAE_Spells]})		/call BuildSpellArray "TargetAE_Spells" 	"TargetAE_Spells2D"
  /if (${Defined[Assist_Debuffs]})		/call BuildSpellArray "Assist_Debuffs" 		"Assist_Debuffs2D"
  /if (${Defined[Command_Debuffs]})		/call BuildSpellArray "Command_Debuffs" 	"Command_Debuffs2D"
  /if (${Defined[Assist_DoTs]})	      /call BuildSpellArray "Assist_DoTs" 	    "Assist_DoTs2D"
  /if (${Defined[Command_DoTs]})	    /call BuildSpellArray "Command_DoTs" 	    "Command_DoTs2D"
  /if (${Defined[EPICBurns]})         /call BuildSpellArray "EPICBurns" 	      "EPICBurns2D"
  /if (${Defined[QUICKBurns]})	      /call BuildSpellArray "QUICKBurns" 	      "QUICKBurns2D"
  /if (${Defined[LONGBurns]})	        /call BuildSpellArray "LONGBurns" 	      "LONGBurns2D"
  /if (${Defined[FULLBurns]})	        /call BuildSpellArray "FULLBurns" 	      "FULLBurns2D"
  /if (${Defined[swarmPets]})	        /call BuildSpellArray "swarmPets" 	      "swarmPets2D"
  /if (${Defined[OffAssistSpells]})	  /call BuildSpellArray "OffAssistSpells"   "OffAssist_Spells2D"
/RETURN

|--------------------------------------------------------------------------------|
SUB assist_MacroSettings
/if (${Debug}) /echo |- assist_MacroSettings ==>
	/call WriteToIni "${advSettings_Ini},Debug,Debug Assists (On/Off)" Off
	/call WriteToIni "${genSettings_Ini},Assists,Auto-Assist (On/Off)" Off
	/call WriteToIni "${genSettings_Ini},Assists,Max Engage Distance" 250
	/call WriteToIni "${genSettings_Ini},Assists,AE Threat Range" 100
	/call WriteToIni "${genSettings_Ini},Assists,Acceptable Target Types" NPC,Pet
  /call WriteToIni "${genSettings_Ini},Assists,Long Term Debuff Recast(s)" 30
  /call WriteToIni "${genSettings_Ini},Assists,Short Term Debuff Recast(s)" 5
/if (${Debug}) /echo <== assist_MacroSettings -|
/RETURN
|--------------------------------------------------------------------------------|
SUB assist_CharacterSettings
/if (${Debug}) /echo |- assist_CharacterSettings ==>
	| Add Assist_Stick entries.
	/call WriteToIni "${Character_Ini},Assist Settings,Assist Type (Melee/Ranged/Off)" ${If[${Select[${Me.Class.ShortName},BRD,BER,BST,MNK,PAL,RNG,ROG,SHD,WAR]},Melee,Off]}
  | Default Tanks to Front
  /if (${Select[${Me.Class.ShortName},PAL,SHD,WAR]}) {
    /call WriteToIni "${Character_Ini},Assist Settings,Melee Stick Point" Front
    /call WriteToIni "${Character_Ini},Assist Settings,Taunt(On/Off)" Off
    /call WriteToIni "${Character_Ini},Assist Settings,SmartTaunt(On/Off)" On
  } else {
    /call WriteToIni "${Character_Ini},Assist Settings,Melee Stick Point" Behind
  }
	/call WriteToIni "${Character_Ini},Assist Settings,Melee Distance" MaxMelee
	/call WriteToIni "${Character_Ini},Assist Settings,Ranged Distance" 100
  /call WriteToIni "${Character_Ini},Assist Settings,Auto-Assist Engage Percent" 98
	| If I'm a melee class, add Melee Abilities entries.
	/if (${Select[${Me.Class.ShortName},BRD,BER,BST,MNK,PAL,RNG,ROG,SHD,WAR,CLR]}) {
		/if (!${Ini[${Character_Ini},Melee Abilities].Length}) /call WriteToIni "${Character_Ini},Melee Abilities,Ability"
	}
	| If I'm a casting class, add Nuke entries.
	/if (${Select[${Me.Class.ShortName},BST,CLR,DRU,ENC,MAG,PAL,NEC,RNG,SHD,SHM,WIZ]}) {
		/if (!${Ini[${Character_Ini},Nukes].Length}) {
      /call WriteToIni "${Character_Ini},Nukes,Main"
      /if (${Select[${Me.Class.ShortName},DRU,RNG,MAG,WIZ]}) {
        /call WriteToIni "${Character_Ini},Nukes,BigFire"
        /call WriteToIni "${Character_Ini},Nukes,FastFire"
        /call WriteToIni "${Character_Ini},Nukes,LureFire"
      }
      /if (${Select[${Me.Class.ShortName},RNG,WIZ,SHM]}) {
        /call WriteToIni "${Character_Ini},Nukes,BigCold"
        /call WriteToIni "${Character_Ini},Nukes,FastCold"
        /call WriteToIni "${Character_Ini},Nukes,LureCold"
      }
      /if (${Select[${Me.Class.ShortName},MAG,WIZ]}) {
        /call WriteToIni "${Character_Ini},Nukes,BigMagic"
        /call WriteToIni "${Character_Ini},Nukes,FastMagic"
        /call WriteToIni "${Character_Ini},Nukes,LureMagic"
      }
    }
	}

	| If I'm a casting class, add DoT entries.
  /if (${Ini[${Character_Ini},DoTs].Length}) {
    /call WriteToIni "${Character_Ini},DoTs,The [Dots] section is deprecated, move dots to [DoTs on Command] and [DoTs on Assist]"
    /bc [+r+] The [Dots] section is deprecated, move dots to [DoTs on Command] and [DoTs on Assist]
  }

	/if (${Select[${Me.Class.ShortName},BST,DRU,ENC,NEC,RNG,SHD,SHM]}) {
    /if (!${Ini[${Character_Ini},DoTs on Assist].Length}) {
      /call WriteToIni "${Character_Ini},DoTs on Assist,Main"
      /if (${Select[${Me.Class.ShortName},DRU,ENC,NEC,SHD,SHM,RNG]}) /call WriteToIni "${Character_Ini},DoTs on Assist,Magic"
      /if (${Select[${Me.Class.ShortName},NEC,SHD,SHM,BST]}) /call WriteToIni "${Character_Ini},DoTs on Assist,Poison"
      /if (${Select[${Me.Class.ShortName},NEC,SHD,SHM,BST]}) /call WriteToIni "${Character_Ini},DoTs on Assist,Disease"
      /if (${Select[${Me.Class.ShortName},DRU,NEC]}) /call WriteToIni "${Character_Ini},DoTs on Assist,Fire"
    }
    /if (!${Ini[${Character_Ini},DoTs on Command].Length}) {
      /call WriteToIni "${Character_Ini},DoTs on Command,Main"
      /if (${Select[${Me.Class.ShortName},DRU,ENC,NEC,SHD,SHM,RNG]}) /call WriteToIni "${Character_Ini},DoTs on Command,Magic"
      /if (${Select[${Me.Class.ShortName},NEC,SHD,SHM,BST]}) /call WriteToIni "${Character_Ini},DoTs on Command,Poison"
      /if (${Select[${Me.Class.ShortName},NEC,SHD,SHM,BST]}) /call WriteToIni "${Character_Ini},DoTs on Command,Disease"
      /if (${Select[${Me.Class.ShortName},DRU,NEC]}) /call WriteToIni "${Character_Ini},DoTs on Command,Fire"
    }
	}
	| If I'm a casting class, add Debuff entries.
	/if (${Select[${Me.Class.ShortName},BST,DRU,ENC,MAG,NEC,SHM,RNG]}) {
		/if (!${Ini[${Character_Ini},Debuffs].Length}) {
			/call WriteToIni "${Character_Ini},Debuffs,Debuff on Assist"
			/call WriteToIni "${Character_Ini},Debuffs,Debuff on Command"
		}
	}
	| If I'm a casting class, add AE entries.
	/if (${Select[${Me.Class.ShortName},CLR,ENC,DRU,MAG,SHM,WIZ,RNG]}) {
		/if (!${Ini[${Character_Ini},TargetAE].Length}) /call WriteToIni "${Character_Ini},TargetAE,TargetAE"
    /if (!${Ini[${Character_Ini},PBAE].Length}) /call WriteToIni "${Character_Ini},PBAE,PBAE"
	}
	| Burn entries.
	/if (!${Ini[${Character_Ini},Burn].Length}) {
		/call WriteToIni "${Character_Ini},Burn,Quick Burn"
    /call WriteToIni "${Character_Ini},Burn,Long Burn"
		/call WriteToIni "${Character_Ini},Burn,Full Burn"
	}
/if (${Debug}) /echo <== assist_CharacterSettings -|
/RETURN

|--------------------------------------------------------------------------------|
Sub assist_Aliases
/if (${Debug}) /echo |- assist_Aliases ==>
	/noparse /squelch /alias /AssistOn /bc Assist on ${Target.ID}
	/noparse /squelch /alias /AssistMe /bc Assist on ${Target.ID}
	/squelch /alias /BackOff /bc Back off
	/noparse /squelch /alias /debuff /bc Debuffs on ${Target.ID}
	/noparse /squelch /alias /debuffson /bc Debuffs on ${Target.ID}
	/squelch /alias /debuffsoff /bc End Debuffs
	/noparse /squelch /alias /dot /bc DoTs on ${Target.ID}
	/noparse /squelch /alias /dotson /bc DoTs on ${Target.ID}
	/squelch /alias /dotsoff /bc End DoTs
	/squelch /alias /targetaeon /bc targetae on
	/squelch /alias /targetaeoff /bc targetae off
	/squelch /alias /pbaeon /bc pbae on
  /squelch /alias /pbaeoff /bc pbae off
  /squelch /alias /aeon /bc allae on
  /squelch /alias /aeoff /bc allae off
  /squelch /alias /pvpOff /bc //varset TogglePvP FALSE
  /squelch /alias /pvpOn /bc //varset TogglePvP TRUE
  /noparse /squelch /alias /SwarmPets /bc Swarm pets on ${Target.ID}
	/squelch /alias /EpicBurns /bc Epic burn
	/squelch /alias /QuickBurns /bc Quick burn
  /squelch /alias /LongBurns /bc Long burn
	/squelch /alias /FullBurns /bc Full burn
	/squelch /alias /taunton /varset doTaunt 1
  /squelch /alias /tauntoff /varset doTaunt 0
	/squelch /alias /AssistType /bc //varset Assist Type
	/squelch /alias /ns /bc SpellSet
  /squelch /alias /ss /bc SpellSet
	/squelch /alias /combatmode /bc combatmode
	/squelch /alias /cm /bc combatmode

  /squelch /alias /Lesson /bc VetAA Lesson of the Devoted
  /squelch /alias /Infusion /bc VetAA Infusion of the Faithful
  /squelch /alias /Staunch /bc VetAA Staunch Recovery
  /squelch /alias /Servant /bc VetAA Steadfast Servant
  /squelch /alias /Intensity /bc VetAA Intensity of the Resolute
  /squelch /alias /Armor /bc VetAA Armor of Experience
  /squelch /alias /Expedient /bc VetAA Expedient Recovery
  |/squelch /alias /Throne /bc VetAA Throne of Heroes
  /squelch /alias /Jester /bc VetAA Chaotic Jester

/if (${Debug}) /echo <== assist_Aliases -|
/return

|--------------------------------------------------------------------------------|
Sub assist_Background_Events
 
  /if (${auto_AssistOn})    /call check_AutoAssist
  /if (${tankKillEngaged})  /call tankKill
  /doevents Assist
  /doevents BackOff
  /if (${Assisting}) {
    /call check_AssistStatus
    /varset combatTimer ${combatDelay}
  } else {
    /if (${Defined[timerArray]}) /call check_macroTimers
  }
  /doevents Burns
  /doevents AEs
  /doevents EnrageOn
  /doevents EnrageOff
  /doevents DebuffsOn
  /doevents DebuffsAllOn
  /doevents DebuffsOff
  /doevents DoTsOn
  /doevents DoTsOff
  /doevents SwarmPets
  /doevents SpellSet
  /doevents SetTank
  /doevents CombatMode
  /doevents debuffsOnAll
  /doevents OffAssistIgnore
  /doevents OffAssistIgnoreRemove
/return

|--------------------------------------------------------------------------------|
|END SETUP AREA
|--------------------------------------------------------------------------------|


|----------------------------------------------------------------|
|- Turns off Assist_Functions, and resets assisting variables.	-|
|----------------------------------------------------------------|
SUB AssistOff
/if (${Debug} || ${Debug_Assists}) /echo |- AssistOff -| ==>
  /if (${Me.Casting.ID} && !${Spell[${Me.Casting}].SpellType.Find[beneficial]} && ${Me.Class.ShortName.NotEqual[BRD]}) /call interrupt
  /if (${Me.Combat})    /attack off
  /if (${Me.AutoFire})  /autofire off
  /if (${Me.Pet.ID}) /squelch /pet back off

  /delay 5 !${Me.Combat} && !${Me.AutoFire}

 	/varset Assisting FALSE
	/varset AssistTarget 0
	/varset AllowControl FALSE
  /varset isEnraged FALSE

  /if (${Stick.Status.Equal[ON]})   /squelch /stick off
  |/if (${Defined[Debuff_Targets]})  /deletevar Debuff_Targets
  |/if (${Defined[DoT_Targets]})     /deletevar DoT_Targets
  /if (${use_FULLBurns})           /varset use_FULLBurns FALSE
  /if (${use_LONGBurns})           /varset use_LONGBurns FALSE
  /if (${use_QUICKBurns})          /varset use_QUICKBurns FALSE
	/if (${use_EPICBurns})           /varset use_EPICBurns FALSE
  /if (${use_Swarms})              /varset use_Swarms FALSE

  /if (${Defined[${resistCount}]}) /deletevar ${resistCount}
  /call check_ResistCounters
  /if (${Following}) /call AcquireFollow

/if (${Debug} || ${Debug_Assists}) /echo |- AssistOff -| <== [${Assisting}] || AssistTarget: [${AssistTarget}]
/RETURN


|----------------------------------------|
|- Engages /stick on an AssistTarget.	-|
|----------------------------------------|
SUB StickToAssistTarget
  |fix in case assist distance is ever 0, set it to max melee
  /if (${AssistStickDistance}<1) {
    /varset AssistStickDistance ${Math.Calc[${Spawn[id ${AssistTarget}].MaxRangeTo}*.75].Int}
  }
/if (${Debug} || ${Debug_Assists}) /echo |- StickToAssistTarget ==> assistPoint ${AssistStickPoint} dist ${AssistStickDistance}
		/if (${AssistStickPoint.Equal[behind]}) {
			/if (${Debug} || ${Debug_Assists}) /echo Attempting to stick behind [${AssistStickDistance}].
			/squelch /stick snaproll uw
			/delay 20 ${Stick.Behind} && ${Stick.Stopped}
			/squelch /stick hold moveback behind ${AssistStickDistance} uw
    } else /if (${AssistStickPoint.Equal[front]}) {
      /if (${Debug} || ${Debug_Assists}) /echo Attempting to stick front [${AssistStickDistance}].
      /squelch /stick hold front ${AssistStickDistance} uw
      /delay 20 ${Stick.Stopped}
		} else /if (${AssistStickPoint.Equal[behindonce]}) {
			/if (${Debug} || ${Debug_Assists}) /echo Attempting to stick behindonce [${AssistStickDistance}].
			/squelch /stick snaproll uw
      /delay 20 ${Stick.Behind} && ${Stick.Stopped}
			/squelch /stick hold moveback behindonce ${AssistStickDistance} uw
		} else /if (${AssistStickPoint.Equal[pin]}) {
			/if (${Debug} || ${Debug_Assists}) /echo Attempting to stick pin [${AssistStickDistance}].
			/squelch /stick snaproll uw
			/delay 20 ${Stick.Stopped}
			/squelch /stick hold moveback pin ${AssistStickDistance} uw
		} else /if (${AssistStickPoint.Equal[!front]}) {
			/if (${Debug} || ${Debug_Assists}) /echo Attempting to stick !front [${AssistStickDistance}].
			/squelch /stick snaproll uw
      /delay 20 ${Stick.Behind} && ${Stick.Stopped}
			/squelch stick hold moveback !front ${AssistStickDistance} uw
		} else {
			/if (${Debug} || ${Debug_Assists}) /echo [${AssistStickPoint}] is not a valid stick point.  Defaulting to stick behind.
			/squelch /stick snaproll uw
      /delay 20 ${Stick.Behind} && ${Stick.Stopped}
			/squelch /stick hold moveback behind ${AssistStickDistance} uw
		}
/if (${Debug} || ${Debug_Assists}) /echo <== StickToAssistTarget -|	
/RETURN


|----------------------------------------------------|
|- Checks range and combat status while assisting.	-|
|----------------------------------------------------|
SUB check_AssistStatus
/if (${Debug} || ${Debug_Assists}) {
	/echo |- check_AssistStatus ==> AssistTarget = [${AssistTarget}] || AllowControl = [${AllowControl}] || Distance check = [${Spawn[id ${AssistTarget}].Distance} < ${MaxAssistDistance}] || TargetType = [${Spawn[id ${AssistTarget}].Type}]
}

  /if (!${Bool[${Spawn[id ${AssistTarget}].Type.NotEqual[Corpse]}]}) {
    /if (${Debug} || ${Debug_Assists}) /echo Target doesnt exist or is dead, AssistOff
    /call AssistOff
    /return
  }
  |only if macro controlled and assisting, stand up from FD
  /if (${Me.Feigning}) {
    /if (${AllowControl} || !${Select[${Me.Class.ShortName},NEC,SHD,MNK]}) /stand
  }
  | If the character is macro controlled
  /if (${AllowControl} && ((${AssistType.Equal[melee]} || ${AssistType.Equal[ranged]} || ${AssistType.Equal[Autofire]} ))) {
    | Check Target: this only forces retarget for melee/autofire bots and lets casters/priests do their thing
    /if (${Target.ID} != ${AssistTarget} && (!${Bool[${Me.Casting}]} || ${Me.Class.ShortName.Equal[BRD]}) && (${AssistType.Equal[melee]} || ${AssistType.Equal[ranged]}|| ${AssistType.Equal[Autofire]} )) /call TrueTarget ${AssistTarget}
    |----- Melee functions-------------------------------------------------------------------------------------------------------------|
      /if (${AssistType.Equal[Melee]} || ${pauseRanged}) {
        /if (${isEnraged}) {
          /squelch /face fast
          /if ((${Math.Abs[${Me.Heading.Degrees}-${Target.Heading.Degrees}]} > 85 && !${Stick.Behind}) || ${AssistStickPoint.Equal[front]}) {
            /if (${Debug} || ${Debug_Assists}) /echo I am not behind the target. Stopping attack.
            /attack off
            /return
          }
        }
        | If I am target of target and not sticking to front(tank) attack/stick off
        | If I am < 50% life turn off attack (unless tank/stick front)
        /if (${Me.TargetOfTarget.CleanName.Equal[${Me.CleanName}]} && ${AssistStickPoint.NotEqual[front]}) {
          /if (${Me.PctHPs} < 80 && !${Bool[${Select[${Me.TargetOfTarget.Class.ShortName},WAR,PAL,SHD]}]}) /attack off
          /stick off
        } else {
          |turn off autofire if on
          /if (${Me.AutoFire}) {
            /autofire
            /delay 1s
          } 
          | If the mob is not targeting me
          /if (!${Me.Combat}) /attack on
          /if (!${Stick.Active} || (${Target.Distance} > ${AssistStickDistance}) || (${AssistStickPoint.Find[behind]} && !${Stick.Behind})) {
            /call StickToAssistTarget
            /doevents getCloser
          }
          /if (${pauseRanged}) /varset pauseRanged FALSE
        }
    |----- Ranged functions-------------------------------------------------------------------------------------------------------------||
    } else /if (${AssistType.Equal[Ranged]} && !${pauseRanged}) {
      /squelch /face fast id ${AssistTarget}
      |dont play chase
      /if (${Me.TargetOfTarget.CleanName.Equal[${Me.CleanName}]}) {
        /squelch /autofire off
        /squelch /attack on
        /varset pauseRanged TRUE
        /squelch /stick 8
      } else {
        /if (${RangedStickDistance.Equal[Clamp]}) {
          /if (${Target.Distance} >=30 && ${Target.Distance} <=200) {
            /squelch /stick hold moveback ${Target.Distance}
          } else {
            /if (${Target.Distance} > 200) /squelch /stick hold moveback 195
            /if (${Target.Distance} < 30) /squelch /stick hold moveback 35
          }
        } else {
          /squelch /stick hold moveback ${RangedStickDistance}
        }
        /if (!${Me.AutoFire}) {
            /delay 1s
            /autofire
      } 
      }
    } else /if (${AssistType.Equal[Autofire]} && !${pauseRanged}) {
      /squelch /face fast id ${AssistTarget}
      /if (${Target.Distance} > 200) {
        /squelch /stick moveback 195
      } 
      |keep out of AE rampage range
      |/if (${Target.Distance} < 35) {
       | /squelch /stick moveback 35
      |} 
      /if (!${Me.AutoFire}) {
            /delay 1s
            /autofire
      } 
    }
  }
  /if ((!${Me.Casting.ID} || ${Me.Class.ShortName.Equal[BRD]}) && (${Me.Combat} || ${Me.CombatState.Equal[Combat]} ||  ${AssistTarget} >0 ) && ${Target.ID} == ${AssistTarget} && ${Defined[Abilities2D]}) /call CombatAbilities
/if (${Debug} || ${Debug_Assists}) /echo <== check_AssistStatus -|
/RETURN

|------------------------------------------------|
|- Uses combat abilities, AAs, and disciplines.	-|
|- e3_casting is not used, for performance, as these are short recast low impact only
|------------------------------------------------|
SUB CombatAbilities

  
  /if (${Debug} || ${Debug_Assists}) /echo |- CombatAbilities ==>
  /declare i int local
  | Prereq/priorities
  /if (${Defined[doTaunt]}) {
    /if (${doTaunt} && ${Me.AbilityReady[Taunt]} && ${Me.PctAggro}< 100) /doability Taunt
  }
  /if (${Defined[smartTaunt]}) {
    /if (${smartTaunt}) {
      /if (${Bool[${Me.TargetOfTarget}]} && !${Select[${Me.TargetOfTarget.Class.ShortName},WAR,PAL,SHD]}) {
        /if (${Select[${Spawn[id ${Target.ID}].Type},${AcceptableTargetTypes}]}) {
          /if (${Me.AbilityReady[Taunt]}) {
            /bc Taunting ${Target.CleanName}: ${Me.TargetOfTarget.Class.ShortName}-${Me.TargetOfTarget} has aggro
            /echo [${Time}] Taunting ${Target.CleanName}: ${Me.TargetOfTarget.Class.ShortName}-${Me.TargetOfTarget} has aggro
            /doability Taunt
          |} else /if (${Me.CombatAbilityReady[Mock]}) {
          |  /bc Mock on ${Target.CleanName}: ${Me.TargetOfTarget.Class.ShortName}-${Me.TargetOfTarget} has aggro
          |  /doability Mock
          | } else /if (${Me.CombatAbilityReady[Bazu Bellow]}) {
          |  /bc Bazu Bellow on ${Target.CleanName}: ${Me.TargetOfTarget.Class.ShortName}-${Me.TargetOfTarget} has aggro
          |   /doability "Bazu Bellow"
          } else /if (${Me.AltAbilityReady[Divine Stun]}) {
            /bc Divine Stun on ${Target.CleanName}: ${Me.TargetOfTarget.Class.ShortName}-${Me.TargetOfTarget} has aggro
            /call castSimpleSpell "Divine Stun" ${Target.ID}

          } else /if (${Me.SpellReady[Terror of Discord]}) {
            /bc Terror of Discord on ${Target.CleanName}: ${Me.TargetOfTarget.Class.ShortName}-${Me.TargetOfTarget} has aggro
            /call castSimpleSpell "Terror of Discord" ${Target.ID}
          }
        }
      }
    }
  }
  /if (${Defined[AutoEvade]}) {
    /if (${AutoEvade}) /call AutoEvade
  }
 
  /declare castIfNotName string local
  /declare castIfName string local

  /for i 1 to ${Abilities2D.Size[1]}
 /if (${Abilities2D.Size[1]}==0) /break

      |skips if contains or not contains
      /varset castIfName	${Abilities2D[${i},${iCastIf}]}
      /if (${Bool[${castIfName}]} && !${castIfName.Equal[-1]} ) {
            /if (!${Bool[${Target.Buff[${castIfName}]}]}) {
              |/echo \ag [${Time}] ${Target.CleanName} is not debuffed with ${checkForDebuff} , skipping
              /goto :skipAbility
            }   
      }

      /varset castIfNotName	${Abilities2D[${i},${iCheckFor}]}
      /if (!${castIfNotName.Equal[-1]} ) {
            /if (${Bool[${Target.Buff[${castIfNotName}]}]}) {
              |/echo \ag [${Time}] ${Target.CleanName} is not debuffed with ${checkForDebuff} , skipping
              /goto :skipAbility
            }   
      }
      |end contains not contains 

    |skip the ability if current aggro is less than defined PctAggro
    /if (${Abilities2D[${i},${iPctAggro}]} > 0 && (${Me.PctAggro} < ${Abilities2D[${i},${iPctAggro}]})) {
      /if (${Debug} || ${Debug_Assists}) /echo skipping ${Abilities2D[${i},${iCastName}]}, current aggro ${Me.PctAggro} defined ${Abilities2D[${i},${iPctAggro}]} named ${Target.Named}
      /goto :skipAbility
    }
    /if (${Abilities2D[${i},${iCastType}].Equal[Ability]}) {

      /if (${Abilities2D[${i},${iCastName}].Equal[Bash]}) {
        /if (!${Select[${Me.Inventory[Offhand].Type},Shield]} && !${Me.AltAbility[2 Hand Bash]}) /goto :skipAbility
        |can't do anything if stunned.
        /if (${Me.Stunned}) /goto :skipAbility
        /if (${Target.Distance} >14) /goto :skipAbility
        
      }
      /if (${Me.AbilityReady[${Abilities2D[${i},${iCastName}]}]}) {
        
        /call castSimpleSpell "${Abilities2D[${i},${iCastName}]}" 0
      }
      |- AA, only those with 0 cast time can go here
    } else /if (${Abilities2D[${i},${iCastType}].Equal[AA]}) {
      |/echo Checking i: ${i}  with cast name ${Abilities2D[${i},${iCastName}]} with value:${Abilities2D[${i},${iIfs}]}
      /if (${Me.AltAbilityReady[${Abilities2D[${i},${iCastName}]}]} && ${Abilities2D[${i},${iIfs}]}) {
        /call castSimpleSpell "${Abilities2D[${i},${iCastName}]}" 0
      }
      |- Discipline, no disc with a duration can go here
    } else /if (${Abilities2D[${i},${iCastType}].Equal[Disc]}) {
    |/echo car ${Me.CombatAbilityReady[${abilityName}]} ${Me.CombatAbility[${abilityName}]} ${abilityName} ${Me.CombatAbilityReady[${abilityName}]} ${Me.CombatAbilityReady[Backstab]}
      /if (${Me.CombatAbilityReady[${Abilities2D[${i},${iCastName}]}]} && !${Me.ActiveDisc.Name.Find[${Abilities2D[${i},${iCastName}]}]}) {
        |/echo ${Abilities2D[${i},${iCastName}]} myend ${Me.PctEndurance} >= minend ${Abilities2D[${i},${iMinEnd}]} and endcost ${Spell[${Abilities2D[${i},${iCastName}]}].EnduranceCost} <= ${Me.Endurance}
        /if (${Me.PctEndurance} >= ${Abilities2D[${i},${iMinEnd}]} && ${Spell[${Abilities2D[${i},${iCastName}]}].EnduranceCost} <= ${Me.Endurance} && ${Abilities2D[${i},${iIfs}]}) {
          |/echo ready ${abilityName} ${Me.CombatAbilityReady[${abilityName}]}
          /if (!(${Me.ActiveDisc.ID} && ${Abilities2D[${i},${iCastType}].Equal[Disc]} && ${Spell[${Abilities2D[${i},${iCastName}]}].TargetType.Equal[Self]})) {
              /call castSimpleSpell "${Abilities2D[${i},${iCastName}]}" 0
          }
        }
      }
    }
  :skipAbility
  /next i

  /if (${Debug} || ${Debug_Assists}) /echo <== CombatAbilities -|
/RETURN


|------------------------------------------------------------|
|- dont use this, its dumb and expensive
|------------------------------------------------------------|
SUB check_AutoAssist
/if (${Debug} || ${Debug_Assists}) /echo |- check_AutoAssist ==>
	/if (!${Assisting}) {
		/if ((${Target.ID} && ${Me.Combat}) || (${Target.ID} && ${Spell[${Me.Casting.ID}].SpellType.Equal[Detrimental]})) {
			/if (${Target.PctHPs} < ${EngagePct} && ${Select[${Spawn[id ${Target.ID}].Type},${AcceptableTargetTypes}]}) {
			  /if ((!${Bool[${PauseTarget}]} || ${Target.ID} != ${PauseTarget}) && (!${Bool[${Me.Pet.ID}]} || ${Target.ID} != ${Me.Pet.ID})) {
          /if (${Debug} || ${Debug_Assists}) /echo I am attacking, calling Assist
          /assistme
        }
			}
    }
	}
/if (${Debug} || ${Debug_Assists}) /echo <== check_AutoAssist -|
/RETURN


|--------------------------------------------------------------------------------|
| Author: Rekka, original idea SirHops
| Casts spells on the non-assist target (but only while assist is active)
| this is useful for casting dots / debuffs on the non-assist targets 
| when you know the main assist target will be dead soon
|--------------------------------------------------------------------------------|
SUB check_OffAssistSpells
  | If we're not currently attacking something, don't do anything
  /if (!${Assisting}) /return
  /if (${Bool[${Raid}]}) /return
  | loop through the xtargets checking if any match our criteria
  | break on the first one that matches
  /declare pctAggro int local 80
  /declare minHp int local 10
  /declare highestHp int local 0
  /declare mobId int local 0
  /declare i int local
 
  /for i 1 to 13
    |check off assist exemptions
    /if (!${Me.XTarget[${i}].TargetType.Equal[Auto Hater]}) /continue
    
    /varset mobId ${Me.XTarget[${i}].ID}
    
    /if (!${Bool[${mobId}]}) /continue
    /if (${mobId}<1) /continue
    /if (${mobId}==${AssistTarget}) /continue
      |should we ignore this mob type?
    /if (${Defined[offAssistIgnore_${mobId}]}) /continue
   
    |using spawn to limit access to xtarget object to try and avoid xtarget bug (unknown if it will help)
    /if (!${Spawn[npc id ${mobId}].Aggressive}) /continue
    /if (${Spawn[npc id ${mobId}].PctHPs}<=${minHp}) /continue
    /if (!${Spawn[npc id ${mobId}].LineOfSight}) /continue
    /if (${Spawn[npc id ${mobId}].Distance}>60) /continue
    /if (${Spawn[npc id ${mobId}].Type.Equal[Corpse]}) /continue
     

    /call cast_longTermSpells "${mobId}" "OffAssist_Spells2D"
    /if (${ActionTaken}) {
      /return
    }
       
  /next i

/RETURN

|----------------------------------|
|- Swap to Ranged for Target AE	  -|
|----------------------------------|
SUB swapToRangedSet(bool toggleRanged)
  /echo ${toggleRanged} toggleRanged
  /if (${toggleRanged} && ${AssistType.Equal[melee]}) {
    /varset AssistType Ranged
    /varset revertRngToMelee TRUE
    /call WriteToIni "${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Pending Exchange" "${Me.Inventory[Ranged].Name}/Ranged" 1
    /delay 2
    /swap ranged
    /delay 2
  } else {
    /varset AssistType Melee
    /varset revertRngToMelee FALSE
    /swap melee
    /delay 2
    /call WriteToIni "${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Pending Exchange" "NONE"
    /delay 2
  }
/RETURN


Sub useSwarms
    
    /if (${Defined[swarmPets2D]}) {
    /if (!${Assisting}) /return
    /declare swarmTarget int Local
    /varset swarmTarget ${AssistTarget}
		/if (${SpawnCount[id ${swarmTarget}]} && ${Spawn[${swarmTarget}].Type.NotEqual[Corpse]} && ${Select[${Spawn[${swarmTarget}].Type},${AcceptableTargetTypes}]}) {
			/declare i int local
			/for i 1 to ${swarmPets2D.Size[1]}
 /if (${swarmPets2D.Size[1]}==0) /break
        /call check_Ready "swarmPets2D" ${i}
        /if (${c_Ready} && ${swarmPets2D[${i},${iIfs}]}) {
          /if (${check_Distance[${swarmTarget},${swarmPets2D[1,${iMyRange}]}]}) {
            /g SWARM-${swarmPets2D[${i},${iSpellName}]}
				    /call e3_Cast ${swarmTarget} "swarmPets2D" "${i}" 1
          }
        }
			/next i
		}
	}

/Return
|----------------------------------------------------|
SUB check_Burns
  /if (${Assisting}) {
    /if (${use_FULLBurns})  /call useBurns "FULLBurns2D"
    /if (${use_LONGBurns})  /call useBurns "LONGBurns2D"
    /if (${use_QUICKBurns}) /call useBurns "QUICKBurns2D"
    /if (${use_EPICBurns})  /call useBurns "EPICBurns2D"
    /if (${use_Swarms}) /call useSwarms
  }
/RETURN



|--------------------------------------------------------------------------------|
|- Casts spells, abilities, and burns through disciplines on a given targetID.	-|
|--------------------------------------------------------------------------------|
SUB useBurns(ArrayName)
/if (${Debug} || ${Debug_Assists}) /echo |- useBurns Array=${ArrayName} ==>

	/declare i int local
  /declare isHealer bool Local ${Select[${Me.Class.ShortName},CLR,DRU,SHM]}
	/for i 1 to ${${ArrayName}.Size[1]}
 /if (${${ArrayName}.Size[1]}==0) /break

    /call check_Ready "${ArrayName}" ${i}
    /if (${c_Ready} && ${${ArrayName}[${i},${iIfs}]}) {

      /if (${check_Distance[${AssistTarget},${${ArrayName}[${i},${iMyRange}]}]}) {
       
        /if (${Me.ActiveDisc.ID} && ${${ArrayName}[${i},${iCastName}].Equal[Savage Spirit]}) /goto :skipCurrent
        /if (${Me.ActiveDisc.ID} && ${${ArrayName}[${i},${iCastType}].Equal[Disc]} && ${Spell[${${ArrayName}[${i},${iCastName}]}].TargetType.Equal[Self]}) {
           | /echo Currently in disc and duration > 0. Skiping ${${ArrayName}[${i},${iCastName}]}. Duration: ${Bool[${${ArrayName}[${i},${iDuration}]}]} 
           |/echo Burns-Skip: ${i} ${${ArrayName}[${i},${iCastName}]}
         
           /goto :skipCurrent

        } 
        /call e3_Cast ${AssistTarget} "${ArrayName}" "${i}" 0
       
        /if (${Select[${castReturn},CAST_SUCCESS]}) {
          /g ${ArrayName}-${${ArrayName}[${i},${iCastName}]}
        } else {
          |/g ${castReturn}: ${ArrayName}-${${ArrayName}[${i},${iCastName}]}
        }
       
      }
    } else {
      /if (${Debug} || ${Debug_Assists}) /echo useBurns ${ArrayName}: NotReady: ${i} ${${ArrayName}[${i},${iCastName}]}
    }
    :skipCurrent
    /if (${isHealer} && !${${ArrayName}[${i},${iNoInterrupt}]} && !${ArrayName.Equal[EPICBurns2D]}) {
        |interupt burning if we have someone to heal
        /call check_HealCasting_DuringBurns
        /if (!${c_SubToRun}) {
          	/echo [${Time}]: useBurns: ${ArrayName}: healer needs to heal jumping out of burn.
            /break
        }
    } 

  /next i

/if (${Debug} || ${Debug_Assists}) /echo <== useBurns -|
/RETURN

|--------------------------------------------------------------------------------|
SUB check_lifeTap
  /if (${Debug}) /echo |- check_LifeTap ==>
  /if (${Assisting} && ${Defined[lifeTaps2D]}) {
    /declare s int local
    /for s 1 to ${lifeTaps2D.Size[1]}
 /if (${lifeTaps2D.Size[1]}==0) /break
    /call check_Ready "lifeTaps2D" ${s}
    /if (${c_Ready} && ${lifeTaps2D[${s},${iIfs}]}) {
      /if (${check_Mana["lifeTaps2D",${s}]}) {
        /if (${Me.PctHPs} <= ${lifeTaps2D[${s},${iHealPct}]}) {
          /if (${check_Distance[${AssistTarget},${lifeTaps2D[${s},${iMyRange}]}]}) {
            /call e3_Cast ${AssistTarget} "lifeTaps2D" ${s}
          }
        }
      }
    }
    /if (${ActionTaken}) /break
 /next s
  }
  /if (${Debug}) /echo <== check_LifeTap -|
/RETURN


|----------------------------------------------------------------------|
|- Ewiclip MOD: Function for returning duration of my spell on the NPC
|- Return Seconds remaining for a buff on your target
|----------------------------------------------------------------------|
Sub TimeLeftOnMySpell(SpellName)
  |${Target.BuffDuration[${${ArrayName}[${i},${iCastName}]} ]} -- Divide by 1000 to get seconds remaining on DoTName
  |${Target.Buff[${${ArrayName}[${i},${iCastName}]} ].Caster}  -- Who casted this DoT?
  |${Target.Buff[DoTName].ID) -- SpellID 
  |${FindItem[DoTName].Spell.ID} -- Returns ID of spell on item correctly
  
  /declare SpellID string local 0
  /declare ItemName string local
  /declare BuffDuration int local 0

  /if (${Bool[${FindItem[=${SpellName}]}]}) {
    /varset SpellID ${FindItem[${SpellName}].Spell.ID}
    /varset ItemName SpellName
    /varset SpellName ${FindItem[${SpellName}].Spell}
  } else {
    /varset SpellID ${Spell[${SpellName}].ID}
  }
  
  /declare iBuff int local 1
  /for iBuff 1 to 100
    /if ((${SpellName.Equal[${Target.Buff[${iBuff}]}]}) && ${SpellID.Equal[${Target.Buff[${iBuff}].ID}]} && ${Target.Buff[${iBuff}].Caster.Equal[${Me}]}) {
      /varset BuffDuration ${Math.Calc[${Target.BuffDuration[${iBuff}]} / 1000]}
      /break
    }
  /next iBuff
/RETURN ${BuffDuration}