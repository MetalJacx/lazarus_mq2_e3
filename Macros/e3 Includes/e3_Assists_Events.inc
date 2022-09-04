|--------------------------------------------------------------------|
|- Contains assisting related functions for e3.mac.					        -|
|- check_IsLookingAt by genbot.mac                                  -|
|- Use, 'Assist on ${Target.ID}' in /tell, /group, or /bc.					-|
|--------------------------------------------------------------------|
#event Assist "<#1#> Assist on #2#"
SUB event_Assist(line, ChatSender, int MobID)

/if (${Debug} || ${Debug_Assists}) /echo |- event_Assist ==>
  /if (!${checkEventArgs[${ChatSender},${line},UZR,${MobID}]}) /return
  /varset MobID ${c_eventArgData}
  | Check that you're not assisting on a corpse, pet or pc
  /if (!${Bool[${Spawn[id ${MobID}].Type.NotEqual[Corpse]}]} || !${Select[${Spawn[id ${MobID}].Type},${AcceptableTargetTypes}]}) {
    /if (${TogglePvP}) /goto :assistAnyway
    /if (${Debug} || ${Debug_Assists}) /echo [${Spawn[id ${MobID}].CleanName}] is not an acceptable TargetType
  } else {
    :assistAnyway
    /if (${Me.Feigning}) {
      |/if (${Select[${Me.Class.ShortName},NEC,SHD,MNK]}) {
      /if (${Select[${Me.Class.ShortName},NEC,SHD]}) {
        /return
      } else {
        /stand
      }
    }
    | Check target distance.
    /if (${Spawn[${MobID}].Distance3D} > ${MaxAssistDistance}) {
      /docommand ${ChatToggle} ${Spawn[${MobID}].CleanName} is too far away.
    } else {
      | Wait to catch up to group, before engage.
      |/echo ${Following}   ${Spawn[${FollowTarget}].Distance}   ${LeashLength}   ${Me.Moving}
      /if (${Following} && ${Spawn[${FollowTarget}].Distance3D} > ${LeashLength} && ${Me.Moving}) {
      |/echo distloop
        /declare Assist_Moving_Timer timer local 100
        :movetoAssist_Loop
        /call Background_Events
        /if (${Spawn[=${FollowTarget}].Distance} > ${LeashLength} && ${Assist_Moving_Timer}) /goto :movetoAssist_Loop
      }
      | Disengage follow.
      /if (${Stick.Active}) /squelch /stick off
/if (${AdvPath.Following}) /squelch /afollow off      | Set Assist Variables.
      /varset Assisting TRUE
      /varset AssistTarget ${MobID}
      /varset AllowControl FALSE
      /varset isEnraged FALSE
      | If I am the assist caller, or I am feigning, don't allow macro control.
      /if (${ChatSender.NotEqual[${Me.CleanName}]}) {
        /varset AllowControl TRUE
        |/docommand ${ChatToggle} Attacking - ${AssistTarget} - ${Spawn[${AssistTarget}].CleanName}
        /if (${Target.ID} != ${AssistTarget} && (!${Bool[${Me.Casting}]} || ${Me.Class.ShortName.Equal[BRD]}) && (${AssistType.Equal[melee]} || ${AssistType.Equal[ranged]})) /call TrueTarget ${AssistTarget}
        /squelch /face fast id ${AssistTarget}
        /if (${Me.Pet.ID}) {
          /if (${Debug} || ${Debug_Assists}) /echo Pet Check: Sending pet in
          /pet attack ${AssistTarget}
        }
        /if (${AssistType.Equal[melee]}) {
          /if (${Debug} || ${Debug_Assists}) /echo melee on
          /if (${Int[${AssistDistanceSetting}]}) {
            /varset AssistStickDistance ${AssistDistanceSetting}
          } else {
            /if (${AssistDistanceSetting.Equal[MaxMelee]} || ${AssistDistanceSetting.Equal[auto]}) /varset AssistStickDistance ${Math.Calc[${Spawn[${Target.ID}].MaxRangeTo}*.75].Int}
          }
          | Check MaxRangeTo, make sure it is not out of bounds.
          /if (${AssistStickDistance} > 25) /varset AssistStickDistance 25
          /call StickToAssistTarget
          /if (${Defined[strikeDisc]}) {
            /call check_rogStrike
          } else {
            /attack on
          }
        } else /if (${AssistType.Equal[ranged]}) {
          /if (${Debug} || ${Debug_Assists}) /echo autofire on
          /squelch /face fast id ${AssistTarget}
          /if (!${Me.AutoFire}) {
            /delay 1s
            /autofire
          } 
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
        } else /if (${AssistType.Equal[off]}) {
          | Face the target for less dmg and stun resist for Ogres
          /squelch /face fast id ${AssistTarget}
        }
      /if (${Debug} || ${Debug_Assists}) /echo Assist=${Assisting} || AssistTarget=${AssistTarget} || AllowControl=${AllowControl}
      }
    }
  }
/if (${Debug} || ${Debug_Assists}) /echo <== event_Assist -|
/RETURN

|--------------------------------------------------------------------|
|- Backs off an AssistTarget, and labels them as a 'PauseTarget'.	  -|
|- You must manually call engage to re-engage a PauseTarget.		    -|
|- Use, '/backoff', or, 'Back off' in /tell, /group, or /bc.		    -|
|--------------------------------------------------------------------|
#event BackOff "<#1#> Back Off#*#"
SUB Event_BackOff(line, ChatSender)
/if (${Debug} || ${Debug_Assists}) /echo |- Event_BackOff ==>
  /if (!${checkEventArgs[${ChatSender},${line},UZR,""]}) /return
|#event BackOff "<#1#> Back Off#2#"
|SUB Event_BackOff(line, ChatSender, args)
|/if (${Debug} || ${Debug_Assists}) /echo |- Event_BackOff ==>

|  /if (!${checkEventArgs[${ChatSender},${line},UZR,${args}]}) /return
  /echo Backing off.
  /if (${c_eventArgChatSender.Equal[${Me}]}) /docommand ${ChatToggle} Backing off.
  /varset PauseTarget ${Target.ID}
  /call AssistOff
/if (${Debug} || ${Debug_Assists}) /echo <== Event_BackOff -|
/RETURN


|------------------------------------------------------------|
|- Turns off attack, when a mob you're attacking enrages.	-|
|------------------------------------------------------------|
#event EnrageOn "#1# has become ENRAGED."
SUB Event_EnrageOn(line, RageOn)
  /if (${Target.ID} == ${Spawn[${RageOn}].ID}) /varset isEnraged TRUE
  /if (${Me.Pet.ID}) {
    /pet back off
    /echo ${Me.Name} is backing off pet for enrage
  } 
/RETURN
|------------------------------------------------|
|- Turns attack back on, after enrage is over.	-|
|------------------------------------------------|
#event EnrageOff "#1# is no longer enraged."
SUB Event_EnrageOff(line, RageOn)
  /if (${Target.ID} == ${Spawn[${RageOn}].ID}) /varset isEnraged FALSE
  /if (${Me.Pet.ID}) /pet attack
/RETURN

|------------------------------------------------|
|- Adjust Melee distance if too far away msg   	-|
|------------------------------------------------|
#EVENT getCloser "Your target is too far away, get closer!"
SUB EVENT_getCloser
	/if (${Assisting} && ${AllowControl}) {
    /doevents flush EVENT_getCloser
		/if (${AssistStickDistance} > 5) /varcalc AssistStickDistance ${AssistStickDistance} - 3
	}
/if (${Debug} || ${Debug_Assists}) /echo EVENT_getCloser -| AssistStickDistance has been set to [${AssistStickDistance}]
/RETURN


|----------------------------------------------------------------|
|- Casts swarm pets from Mages, Necros, Shaman, and Wizards.	-|
|----------------------------------------------------------------|
#event SwarmPets "<#1#> Swarm Pets on #2#"
SUB event_SwarmPets(line, ChatSender, int swarmTarget)
/if (${Debug} || ${Debug_Assists}) /echo |- event_SwarmPets ==>
  /if (!${checkEventArgs[${ChatSender},${line},UZR,${swarmTarget}]}) /return
    /if (${Defined[swarmPets2D]}) {
    /varset use_Swarms TRUE
   }
/if (${Debug} || ${Debug_Assists}) /echo <== event_SwarmPets -|
/RETURN


|----------------------------------------------------|
#EVENT Burns "<#1#> #2# Burn"
#EVENT Burns "#1# tells you, '#2# Burn'"
#EVENT Burns "#1# tells the group, '#2# Burn'"
#EVENT Burns "#1# tell your party, '#2# Burn'"
SUB event_Burns(line, ChatSender, burnType)
/if (${Debug} || ${Debug_Assists}) /echo |- event_Burns ==>
  /if (${line.Find[,]}) /varset line ${line.Left[${Math.Calc[${line.Find[,]}-1]}]} ${line.Right[-${line.Find[,]}]}
  /if (!${checkEventArgs[${ChatSender},${line},UZR,${burnType}]}) /return
  /varset burnType ${c_eventArgData.Upper}
  |/if (!${Assisting}) {
   | /echo You must be attacking something to ${burnType} burn.
    |/return
  |}
  /if (!${Defined[${burnType}Burns2D]}) {
    /return
  } else {
    /if (${c_eventArgChatSender.Equal[${Me}]}) /bc [+g+] ${burnType} burns enabled
    /varset use_${burnType}Burns TRUE
  }
/return


|--------------------------------------------------------------------------------|
#EVENT SpellSet "<#1#> SpellSet #2#"
#EVENT SpellSet "[#1#] SpellSet #2#"
#EVENT SpellSet "#1# tells you, 'SpellSet #2#'"
#EVENT SpellSet "#1# tells the group, 'SpellSet #2#'"
#EVENT SpellSet "#1# tell your party, 'SpellSet #2#'"
SUB event_SpellSet(line, ChatSender, spellArray)
/if (${Debug} || ${Debug_Assists})  /echo |- event_SpellSet ==>
  /if (${line.Find[,]}) /varset line ${line.Left[${Math.Calc[${line.Find[,]}-1]}]} ${line.Right[-${line.Find[,]}]}
  /if (!${checkEventArgs[${ChatSender},${line},UZR,${spellArray}]}) /return
  /varset spellArray ${c_eventArgData}
  /declare i int local
  /if (${Select[${Me.Class.ShortName},BST,CLR,DRU,ENC,MAG,PAL,NEC,RNG,SHD,SHM,WIZ]}) {
    /if (${Select[${spellArray},ON,TRUE,Nuke]}) {
      /varset spellArray Main
    } else /if (${Select[${spellArray},OFF,FALSE]}) {
      /varset use_Nukes FALSE
      /echo \ag Nuking Disabled
    }
    |nukes
    /if (!${Ini[${Character_Ini},Nukes,${spellArray}].Length}) {
      /if (${Debug} || ${Debug_Assists}) /echo \ay [${spellArray}] is not configured in your bot ini [Nukes], keeping existing spell set: [${spellsToUse}]
    } else {
      /call IniToArrayV "${Character_Ini},Nukes,${spellArray}#" MyNukes
      /if (${Defined[MyNukes]}) {
        /if (${MyNukes[1].Equal[OFF]}) {
          /varset use_Nukes FALSE
          /echo \ag Nuking Disabled, ${spellArray} set to OFF
        } else {
          /call BuildSpellArray "MyNukes" "Nukes2D"
          /varset spellsToUse ${spellArray}
          /varset use_Nukes TRUE
          /echo \ag Nukes set to [${spellsToUse}] - ${Nukes2D.Size[1]} spells
          /if (!${Following}) {
            /for i 1 to ${Nukes2D.Size[1]}
 /if (${Nukes2D.Size[1]}==0) /break
              /if (!${Bool[${Me.Gem[${Nukes2D[${i},${iCastName}]}]}]}) {
                /call memorize_spell "${Nukes2D[${i},${iCastName}]}" ${Nukes2D[${i},${iSpellGem}]}
              }
            /next i
          }
        }
      }
    }
    |dots on assist
    /if (${Ini[${Character_Ini},DoTs on Assist].Length}) {
      /if (!${Ini[${Character_Ini},Dots on Assist,${spellArray}].Length}) {
        /if (${Debug} || ${Debug_Assists}) /echo \ay [${spellArray}] is not configured in your bot ini [Dots on Assist], keeping existing spell set: [${spellsToUse}]
      } else {
        /call IniToArrayV "${Character_Ini},Dots on Assist,${spellArray}#" Assist_DoTs
        /if (${Defined[Assist_DoTs]}) {
          /if (${Assist_DoTs[1].Equal[OFF]}) {
            /varset use_AssistDots FALSE
            /echo \ag Assist Dotting Disabled, ${spellArray} set to OFF
          } else {
            /call BuildSpellArray "Assist_DoTs" "Assist_DoTs2D"
            /varset spellsToUse ${spellArray}
            /varset use_AssistDots TRUE
            /echo \ag Dots on Assist set to [${spellsToUse}] - ${Assist_DoTs2D.Size[1]} spells
            /if (!${Following}) {
              /for i 1 to ${Assist_DoTs2D.Size[1]}
 /if (${Assist_DoTs2D.Size[1]}==0) /break
                /if (!${Bool[${Me.Gem[${Assist_DoTs2D[${i},${iCastName}]}]}]}) {
                  /call memorize_spell "${Assist_DoTs2D[${i},${iCastName}]}" ${Assist_DoTs2D[${i},${iSpellGem}]}
                }
              /next i
            }
          }
        }
      }
    }
    |dots on command
    /if (${Ini[${Character_Ini},DoTs on Command].Length}) {
      /if (!${Ini[${Character_Ini},Dots on Command,${spellArray}].Length}) {
        /if (${Debug} || ${Debug_Assists}) /echo \ay [${spellArray}] is not configured in your bot ini [Dots on Command], keeping existing spell set: [${spellsToUse}]
      } else {
        /call IniToArrayV "${Character_Ini},Dots on Command,${spellArray}#" Command_DoTs
        /if (${Defined[Command_DoTs]}) {
          /if (${Command_DoTs[1].Equal[OFF]}) {
            /varset use_CommandDots FALSE
            /echo \ag Command Dotting Disabled, ${spellArray} set to OFF
          } else {
            /call BuildSpellArray "Command_DoTs" "Command_DoTs2D"
            /varset spellsToUse ${spellArray}
            /varset use_CommandDots TRUE
            /echo \ag Dots on Command set to [${spellsToUse}] - ${Command_DoTs2D.Size[1]} spells
            /if (!${Following}) {
              /for i 1 to ${Command_DoTs2D.Size[1]}
 /if (${Command_DoTs2D.Size[1]}==0) /break
              /if (!${Bool[${Me.Gem[${Command_DoTs2D[${i},${iCastName}]}]}]}) {
                /call memorize_spell "${Command_DoTs2D[${i},${iCastName}]}" ${Command_DoTs2D[${i},${iSpellGem}]}
              }
              /next i
            }
          }
        }
      }
    }
  } else /if (${Me.Class.ShortName.Equal[ROG]}) {
    /declare s int local
    /declare cbpi int local
    /declare sbpi int local
    /call IniToArrayV "${Character_Ini},Buffs,Combat Buff#"	CombatBuffs
    /if (${Defined[CombatBuffs]}) {
      /for s 1 to ${CombatBuffs.Size}
 /if (${CombatBuffs.Size}==0) /break
        /if (${Select[${CombatBuffs[${s}]},${poisonFR},${poisonCR},${poisonPR}]}) {
          /varset cbpi ${s}
          /if (${Debug} || ${Debug_Assists}) /echo poison found in ${cbpi} ${CombatBuffs[${cbpi}]}
        }
      /next s
    }
    /call IniToArrayV "${Character_Ini},Buffs,Self Buff#" SelfBuffs
    /if (${Defined[SelfBuffs]}) {
      /for s 1 to ${SelfBuffs.Size}
 /if (${SelfBuffs.Size}==0) /break
        /if (${Select[${SelfBuffs[${s}]},${poisonFR},${poisonCR},${poisonPR}]}) {
          /varset sbpi ${s}
          /if (${Debug} || ${Debug_Assists}) /echo poison found in ${sbpi} ${SelfBuffs[${sbpi}]}
        }
      /next s
    }
    /if (${spellArray.Find[poison]}) {
      /if (${FindItemCount[=${poisonPR}]}) {
        /if (${cbpi}) /varset CombatBuffs[${cbpi}] ${poisonPR}/${Me.Name}
        /if (${sbpi}) /varset SelfBuffs[${sbpi}] ${poisonPR}
        /squelch ${Me.Buff[${FindItem[=${poisonFR}].Spell}].Remove}
        /squelch ${Me.Buff[${FindItem[=${poisonCR}].Spell}].Remove}
        /echo \ag Poison set to: ${poisonPR}
      } else {
        /echo \ay I do not have any ${poisonPR}, poison unchanged
      }
    } else /if (${spellArray.Find[fire]}) {
      /if (${FindItemCount[=${poisonFR}]}) {
        /if (${cbpi}) /varset CombatBuffs[${cbpi}] ${poisonFR}/${Me.Name}
        /if (${sbpi}) /varset SelfBuffs[${sbpi}] ${poisonFR}
        /squelch ${Me.Buff[${FindItem[=${poisonPR}].Spell}].Remove}
        /squelch ${Me.Buff[${FindItem[=${poisonCR}].Spell}].Remove}
        /echo \ag Poison set to: ${poisonFR}
      } else {
        /echo \ay I do not have any ${poisonFR}, poison unchanged
      }
    } else /if (${spellArray.Find[cold]}) {
      /if (${FindItemCount[=${poisonCR}]}) {
        /if (${cbpi}) /varset CombatBuffs[${cbpi}]  ${poisonCR}/${Me.Name}
        /if (${sbpi}) /varset SelfBuffs[${sbpi}]    ${poisonCR}
        /squelch ${Me.Buff[${FindItem[=${poisonPR}].Spell}].Remove}
        /squelch ${Me.Buff[${FindItem[=${poisonFR}].Spell}].Remove}
        /echo \ag Poison set to: ${poisonCR}
      } else {
        /echo \ay I do not have any ${poisonCR}, poison unchanged
      }
    } else {
      /squelch ${Me.Buff[${FindItem[=${poisonPR}].Spell}].Remove}
      /squelch ${Me.Buff[${FindItem[=${poisonFR}].Spell}].Remove}
      /squelch ${Me.Buff[${FindItem[=${poisonCR}].Spell}].Remove}
    }
    /if (${Defined[CombatBuffs]}) /call BuildSpellArray "CombatBuffs"	"CombatBuffs2D"
    /if (${Defined[SelfBuffs]})   /call BuildSpellArray "SelfBuffs"	  "SelfBuffs2D"
  }
/if (${Debug} || ${Debug_Assists})  /echo <== event_SpellSet -|
/RETURN

|--------------------------------------------------------------------------------|
#event CombatMode "<#1#> CombatMode #2#"
sub event_CombatMode(line, ChatSender, combatMode)
  /if (!${checkEventArgs[${ChatSender},${line},UZR,${combatMode}]}) /return
  /varset combatMode ${c_eventArgData}
  /if (${combatMode.Equal[melee]}) {
    /varset AssistType Melee
    /delay 2
    /swap melee
    /delay 2
  }
  /if (${combatMode.Equal[ranged]}) {
    /varset AssistType Ranged
    /delay 2
    /swap ranged
    /delay 2
  }
  /if (${combatMode.Equal[off]}) {
    /varset AssistType off
  }
  /if (${Debug} || ${Debug_Assists})  /echo |-- Leaving Combat Mode ---|
/return


#event OffAssistIgnore "[#1#(msg)] OffAssistIgnore #2#"
sub event_OffAssistIgnore(line, ChatSender, IgnoreTarget)

  /if (!${Defined[offAssistIgnore_${IgnoreTarget}]}) {
    /echo Adding offAssistIgnore_${IgnoreTarget}
    /declare offAssistIgnore_${IgnoreTarget} int outer ${IgnoreTarget}
  }
/return

#event OffAssistIgnoreRemove "[#1#(msg)] OffAssistIgnoreRemove #2#"
sub event_OffAssistIgnoreRemove(line, ChatSender, IgnoreTarget)

  /if (${Defined[offAssistIgnore_${IgnoreTarget}]}) {
    /echo Deleting offAssistIgnore_${IgnoreTarget}
    /deletevar offAssistIgnore_${IgnoreTarget}
  }
/return

