|----------------------------------------------------|
|- Enables Dots on a Target                         -|
|----------------------------------------------------|
SUB check_DoTs
/if (${Debug} || ${Debug_Assists}) /echo |- check_DoTs ==>
  /if (${Defined[Command_DoTs2D]} && ${use_CommandDots})  /call Dots_OnCommand
	/if (${Assisting} && ${Defined[Assist_DoTs2D]} && ${use_AssistDots}) /call cast_damageOverTimeSpells "${AssistTarget}" "Assist_DoTs2D"
/if (${Debug} || ${Debug_Assists}) /echo <== check_DoTs -|
/RETURN



|---------------------------------------------------------------------------------------------|
|- Validates targets and adds them to a list of Debuff_Targets. Used for Debuffs_OnCommand.	 -|
|---------------------------------------------------------------------------------------------|
|- Use, '/DoTs' or 'DoTs on ${Target.ID}' in /tell or /bc.							 -|
|---------------------------------------------------------------------------------------------|
#EVENT DoTsOn "<#1#> DoTs on #2#"
#EVENT DoTsOn "#1# tells you, 'DoTs on #2#'"
SUB Event_DoTsOn(line, ChatSender, MobID)
/if (${Debug} || ${Debug_Assists}) /echo |- event_DoTsOn ==>
  /if (${Defined[Command_DoTs2D]}) {
    /if (!${checkEventArgs[${ChatSender},${line},UZR,${MobID}]}) /return
  
    /varset MobID ${c_eventArgData}
    /declare lookupKey string local ${MobID}_${Zone.ID}
    /if (${Spawn[id ${MobID}].Type.Equal[Corpse]} || !${Select[${Spawn[id ${MobID}].Type},${AcceptableTargetTypes}]}) {
      /echo Unacceptable Target ${Spawn[id ${MobID}].Type}
    } else {
      | Check range to the DoTs target.
      /if (${Spawn[id ${MobID}].Distance} > ${MaxAssistDistance}) {
        /echo ${Spawn[${MobID}].CleanName} is too far away
        /docommand ${ChatToggle} ${Spawn[${MobID}].CleanName} is too far away.
      } else {
        | Check if the DoT_Targets array is already defined
        
          /if (${DoT_Target_Map.Contains[${lookupKey}]}) {
            /docommand ${ChatToggle} I am already dotting ${Spawn[id ${MobID}].CleanName}.
          } else {

            /if (${DoT_Target_Map.Add[${lookupKey},${lookupKey}]}) {
              /bc [+g+] -- Dotting [${Spawn[${MobID}].CleanName}] --
            }
          }
          
      }
    }
  }
	:Skip
/if (${Debug} || ${Debug_Assists}) /echo <== event_DoTsOn -|
/RETURN




|------------------------------------------------|
|- Cancels dots on command on all targets. 	-|
|------------------------------------------------|
#EVENT DoTsOff "<#1#> End DoTs"
#EVENT DoTsOff "<#1#> End DoTs on #2#"
SUB event_DoTsOff(line, ChatSender, MobID)
/if (${Debug} || ${Debug_Assists}) /echo |- event_DoTsOff ==>
	
  /if (!${Defined[MobID]}) /declare MobID int local
  /if (!${checkEventArgs[${ChatSender},${line},UZR,${MobID}]}) /return
  /if (${Bool[${c_eventArgData}]}) {
    
     /if (${DoT_Target_Map.Contains[${c_eventArgData}_${Zone.ID}]}) {
        /docommand ${ChatToggle} Ending DoTs on ${Spawn[id ${c_eventArgData}].CleanName}.
         /if (!${DoT_Target_Map.Remove[${c_eventArgData}_${Zone.ID}]}) {
            /echo <event_DoTsOff> issue removing :${c_eventArgData}_${Zone.ID}
         }
    }
 
  } else {
    
     /docommand ${ChatToggle} Ending DoTs on all targets.
      /if (!${DoT_Target_Map.Clear}) { 
          /echo Issue clearing DoT_Target_Map
      }
  }
  
/if (${Debug} || ${Debug_Assists}) /echo <== event_DoTsOff -|
/RETURN


|----------------------------------------------------------------|
|- Casts Command_DoTs on targets listed in DoT_Targets array.	-|
|----------------------------------------------------------------|
SUB DoTs_OnCommand
/if (${Debug} || ${Debug_Assists}) /echo |- DoTs_OnCommand ==>

  /if (${DoT_Target_Map.Count}<1) {
    /if (${Debug} || ${Debug_Assists}) /echo <== DoT_OnCommand -|
    /return
  }
	/declare targetID int local
	/declare targetZone int local
  /declare keyValue string local
	/declare i int local
  /declare mi mapiterator local
  /vardata mi DoT_Target_Map.First
    
    /while (!${mi.IsEnd}) {
        /varset keyValue ${mi.Key}
        /varset targetID ${keyValue.Arg[1,_]}
        /varset targetZone ${keyValue.Arg[2,_]}

        /if (${Zone.ID} != ${targetZone}) {
            /if (${DoT_Target_Map_Removal.Add[${keyValue},${keyValue}]}) {}
        } else {
       
          | If the DoT_Target has died, remove them from the array.
          /if (!${Bool[${Spawn[id ${targetID}].Type.NotEqual[Corpse]}]}) {
            
            /if (${DoT_Target_Map_Removal.Add[${keyValue},${keyValue}]}) {}
          
          } else {
            |/call cast_longTermSpells "${targetID}" "Command_DoTs2D"
            |Ewiclip MOD: New method for DoTs
            /call cast_damageOverTimeSpells ${targetID} "Command_DoTs2D"
          }
        }
        /if (${ActionTaken}) /break
        /if (${mi.Advance}) {}

    }
    |remove the values from DoT_Target_Map that we couldn't while iterating
    /if (${DoT_Target_Map_Removal.Count}>0) {
      /declare miRemove mapiterator local
      /vardata miRemove DoT_Target_Map_Removal.First
      /while (!${miRemove.IsEnd}) {
        /if (${DoT_Target_Map.Remove[${miRemove.Key}]} ) {}
        /if (${miRemove.Advance}) {}
      }
      |Clear the removal map as we are now done processing
      /if (${DoT_Target_Map_Removal.Clear} ) {}
    }
  
/if (${Debug} || ${Debug_Assists}) /echo <== DoT_OnCommand -|
/RETURN

|--------------------------------------|
| -- Ewiclip MOD: New Method for DoTs -|
|--------------------------------------|

SUB cast_damageOverTimeSpells(int TargetID, string ArrayName)

/declare i int local
/declare iSpellSwap int local 0
/declare spellTimer string local
/declare spellID int local 0
/declare triggerSpellID int local 0
/declare timeleftOnSpell int local 0
/declare spellName string local
/declare checkForName string local

|${${ArrayName}[${i},${iCastName}]}        --> strSpellName
|${${ArrayName}[${i},${iAllowSpellSwap}]} --> 0/1 
|${${ArrayName}[${i},${iNoEarlyRecast}]}  --> 0/1
|${${ArrayName}[${i},${iNoStack}]}        --> 0/1
|${${ArrayName}[${i},${iTriggerSpell}]}   --> 0/strSpellName
|${${ArrayName}[${i},${iIfs}]}            --> /noaprse string
|${${ArrayName}[${i},${iMinMana}]}        --> int 
|${${ArrayName}[${i},${iSpellGem}]}        --> int 

/for i 1 to ${${ArrayName}.Size[1]}
 /if (${${ArrayName}.Size[1]}==0) /break

    /varset spellName ${${ArrayName}[${i},${iCastName}]} 
    |/echo SpellName is :${spellName}

    /doevents DoTsOff
    |make sure we have the right target    
    |make sure we don't need to do any healing
    /if (${mainLoop_IsHealer}) {
       /call check_HealCasting_DuringDetrimental
        /if (!${c_SubToRun}) {
            /varset ActionTaken TRUE
            /return
        }
    }

    |make sure our target is not a corpse
    /if (!${Bool[${Spawn[id ${TargetID}].Type.NotEqual[Corpse]}]}) {
      /return
    }
    
     /if (${Bool[${FindItem[=${spellName}]}]}) {
        /varset spellID ${FindItem[${spellName}].Spell.ID}
        /varset spellName ${FindItem[${spellName}].Spell}
    } else {
      
      /varset spellID ${Spell[${spellName}].ID}
      |/echo SpellID is:${spellID}
    }
    |setups for resist counters and spellRetry timers
    
    /varset spellTimer spellTimer_${TargetID}_${spellID}
		/varset resistCount resist_${TargetID}_${spellID}
    |Is there a spell timer configured for this spell and target? if so, skip this   
    /if (${Defined[${spellTimer}]}) {
		  /if (${${spellTimer}}) {
        |/echo skipping due to spellTimer_${TargetID}_${spellID} with time: ${${spellTimer}}
        /continue
      }
    }
    | -- Rules that tell us to skip the DoT we're on and evaluate off the next DoT.
    |Skip if /MinMana|%% is set and my mana percentage is below the defined amount in the parameter
    /if (!${${ArrayName}[${i},${iMinMana}]} > ${Me.PctMana}) {
      /continue
    }
    |Skip of defiend IF isn't met
    /if (!${${ArrayName}[${i},${iIfs}]}) {
      /continue
    }
    |Change the target here, as we need the correct target if using TimeLeftOnMySpell
    /if (${Target.ID} != ${TargetID}) {
      |/if (${Debug} || ${Debug_Assists})  /echo changing target to ${TargetID}
      |/echo <cast_Dots> changing target to ${TargetID} from targetid: ${Target.ID}
      /call TrueTarget ${TargetID}
      /delay 5 ${Target.BuffsPopulated}
    }
    
    /varset checkForName	${${ArrayName}[${i},${iCheckFor}]}
    /if (!${checkForName.Equal[-1]} ) {
          /if (${Bool[${Target.Buff[${checkForName}]}]}) {
            /continue
          }   
    }

    /varset timeleftOnSpell ${TimeLeftOnMySpell[${${ArrayName}[${i},${iCastName}]}]} 
    |Skip if this spell is already on the NPC from me and has more than 4 seconds left on it. (This is spell casts)
    |and no trigger spell is specified
    /if (${timeleftOnSpell} > 8 && !${Bool[${${ArrayName}[${i},${iTriggerSpell}]}]}) {
      /continue
    }

    |verify the spell we want to use is actually ready
    :recheckCooldown
    /if (!(${Me.ItemReady[${${ArrayName}[${i},${iCastName}]}]} || ${Me.SpellReady[${${ArrayName}[${i},${iCastName}]}]} || ${Me.AltAbilityReady[${${ArrayName}[${i},${iCastName}]}]})) {

       |we are here because our spells are on cooldown, 
       /if (${Me.SpellInCooldown}) {
          /delay 1
          /goto :recheckCooldown
       }

       /continue
    }
    |Skip if /NoEarlyRecast is set and the mob has this spell from me
    /if (${timeleftOnSpell} > 0 && ${${ArrayName}[${i},${iNoEarlyRecast}]}) {
      /continue
    }
    |Skip if /NoStack is set and ANYONE (including myself) have already casted this on the NPC
    /if (${${ArrayName}[${i},${iNoStack}]} && (${Bool[${Target.Buff[${FindItem[=${${ArrayName}[${i},${iCastName}]}].Spell}]}]} || ${Bool[${Target.Buff[=${${ArrayName}[${i},${iCastName}]}]}]})) {
      /continue
    }
    |Skip if /TriggerSpell|SpellName is set and both the spell and TriggerSpell are already on the NPC from me
    /if (${Bool[${${ArrayName}[${i},${iTriggerSpell}]}]} && ${TimeLeftOnMySpell[${${ArrayName}[${i},${iTriggerSpell}]}]} > 4 && ${timeleftOnSpell} > 4) {
      /continue
    }
    |Skip if this spell is already on the NPC from me and has more than 4 seconds left on it. (This is item casts)
    /if (${Bool[${FindItem[=${${ArrayName}[${i},${iCastName}]}]}]} && ${TimeLeftOnMySpell[${FindItem[=${${ArrayName}[${i},${iCastName}]}].Spell}]} > 4) {
      /continue
    }
    
 

    /if (!${Window[CastingWindow].Open}) {

     
      |set the checks for during the cast for interrupts if a healer type
      /if (${mainLoop_IsHealer}) {
        /varset ${ArrayName}[${i},${iSubToRun}] check_HealCasting_DuringDetrimental
      }
      |Cast the spell!
      /call e3_Cast ${TargetID} ${ArrayName} ${i}
      /if (${castReturn.Equal[CAST_INTERRUPTED]} && ${ActionTaken}) {
           |spell was interrupted due to a heal check kickout
           /return
      }
      
      |Resist checks
      /if (${Cast.Stored.Name.Equal[${${ArrayName}[${i},${iSpellName}]}]} && ${castReturn.Equal[CAST_RESIST]}) {
       
        /if (!${Defined[${resistCount}]}) {
          |this will declare a resist counter as an int outer 1, so we can also keep track of it to delete it later.
          /call register_ResistCounter ${resistCount}
        
        } else {
          /varcalc ${resistCount} ${${resistCount}}+1
          /if (${Debug} || ${Debug_Assists})  /echo incrementing resist counter to:${${resistCount}} for:${resistCount}
          /if (${${resistCount}} >= ${${ArrayName}[${i},${iMaxTries}]}) {
            /varset ${resistCount} 0
            /call createTimer "${spellTimer}" "1m"
          }
        }
      | If the spell did not take hold.
      } else /if (${Cast.Stored.Name.Equal[${${ArrayName}[${i},${iSpellName}]}]} && ${castReturn.Equal[CAST_TAKEHOLD]} && !${Cast.Stored.Name.Equal[${spammableSpell}]}) {
        /if (${Defined[${resistCount}]}) {
           /varset ${resistCount} 0
        }
        |/echo creating spell timer for 3 min before retry
        /call createTimer "${spellTimer}" "3m"
        | If the target was immune.
      } else /if (${Cast.Stored.Name.Equal[${${ArrayName}[${i},${iSpellName}]}]} && ${castReturn.Equal[CAST_IMMUNE]} && !${Cast.Stored.Name.Equal[${spammableSpell}]}) {
        /if (${Defined[${resistCount}]}) {
           /varset ${resistCount} 0
        }
        /call createTimer "${spellTimer}" "5m"
      } else {
         /varset timeleftOnSpell ${TimeLeftOnMySpell[${${ArrayName}[${i},${iCastName}]}]} 
         /if ( ${timeleftOnSpell}<7) {
          /varcalc timeleftOnSpell ${${ArrayName}[${i},${iDuration}]}*6
         } else {
            /varcalc timeleftOnSpell ${timeleftOnSpell}-8
         }
         |/echo Creating timer ${spellTimer} for ${timeleftOnSpell}s
         /call createTimer "${spellTimer}" ${timeleftOnSpell}s
      }


      |Spell Swap Code Specifically for /AllowSpellSwap
      |These spells will De-Mem and Re-Mem a new spell immediately after cast. /Gem|# is ignored for these spells. 
      /if (${Bool[${Me.SpellInCooldown}]} && ${${ArrayName}[${i},${iAllowSpellSwap}]} && ${Bool[${Me.Book[${${ArrayName}[${i},${iCastName}]}]}]}) {
        /for iSpellSwap 1 to ${${ArrayName}.Size[1]}
 /if (${${ArrayName}.Size[1]}==0) /break
          /if (${TimeLeftOnMySpell[${${ArrayName}[${iSpellSwap},${iCastName}]}]} > 18 || ${Bool[${Me.Gem[${${ArrayName}[${iSpellSwap},${iCastName}]}]}]} || !${Bool[${Me.Book[${${ArrayName}[${iSpellSwap},${iCastName}]}]}]} || !${${ArrayName}[${iSpellSwap},${iAllowSpellSwap}]}) {
            /continue
          }
              |After /MemSpell the book will automatically close. However, while in combat this command sometimes fails to start the first time we send it.
              |We put this into a loop to ensure "it as least tries" -- If the spell really does fail to mem it will exit on it's own and try again next cycle.
              |/MemSpell {int WhereToMem} {String SpellToMem}
              /while (!${Window[SpellBookWnd].Open}) {
                /if (${GetEmptySpellGem[]} > 0) {
                  /MemSpell ${GetEmptySpellGem[]} "${${ArrayName}[${iSpellSwap},${iCastName}]}" 
                } else {
                  /MemSpell ${Me.Gem[${${ArrayName}[${i},${iCastName}]}]} "${${ArrayName}[${iSpellSwap},${iCastName}]}" 
                }
              }
              /break
         /next iSpellSwap
      } else {
        |Code to mem any spell in any empty gem with /Gem|# as the preferred location. This only applies to spells that DO NOT have /AllowSpellSwap defined.
        |/AllowSpellSwap spells are mem-shuffled with the code directly above this. And we ignore /Gem|# for those. 
        |In E3, if no /Gem|# is defined it automatically assigns the ${DefaultGem} to that specific spell's ${iSpellGem} variable.
        /if (${Bool[${Me.SpellInCooldown}]} && ${Bool[${Me.Book[${${ArrayName}[${i},${iCastName}]}]}]} && !${Bool[${${ArrayName}[${i},${iSpellSwap}]}]}) {
          /for iSpellSwap 1 to ${${ArrayName}.Size[1]}
 /if (${${ArrayName}.Size[1]}==0) /break
            /if (!${Bool[${Me.Gem[${${ArrayName}[${iSpellSwap},${iCastName}]}]}]} && ${Bool[${Me.Book[${${ArrayName}[${iSpellSwap},${iCastName}]}]}]}) {
              |After /MemSpell the book will automatically close. However, while in combat this command sometimes fails to start the first time we send it.
              |We put this into a loop to ensure "it as least tries" -- If the spell really does fail to mem it will exit on it's own and try again next cycle.
              |/MemSpell {int WhereToMem} {String SpellToMem}
              /while (!${Window[SpellBookWnd].Open}) {
                |If I join this IF with an else /if it causes a hard client crash. --- /break prevents it from sending 2 different MemSpells
                /if (${${ArrayName}[${iSpellSwap},${iSpellGem}]} != ${DefaultGem}) {
                  /MemSpell ${${ArrayName}[${iSpellSwap},${iSpellGem}]} "${${ArrayName}[${iSpellSwap},${iCastName}]}"
                  /break
                }
                /if (${GetEmptySpellGem[]} > 0) {
                  /MemSpell ${GetEmptySpellGem[]} "${${ArrayName}[${iSpellSwap},${iCastName}]}" 
                } else {
                  /MemSpell ${DefaultGem} "${${ArrayName}[${iSpellSwap},${iCastName}]}" 
                }
              }
              /break
            }
          /next iSpellSwap
        }
      }
      |End Spell Swap Code

      |These checks are the same as above--but for the NEXT spell in the DoT array. This is so we don't QuickCast a spell that wouldn't be casted already.

      |verify that the next thing is an AA or Item, so we can ignore the GCD
      /if (${i} < ${${ArrayName}.Size[1]} && ${Bool[${Me.Book[${${ArrayName}[${i},${iCastName}]}]}]} && !${Bool[${Me.Book[${${ArrayName}[${Math.Calc[${i}+1]},${iCastName}]}]}]}) {
        
        |Incrementing loop up 1 so we can check the next spell in sequence. 
        /varcalc i ${i}+1
        |Skip if /MinMana|%% is set and my mana percentage is below the defined amount in the parameter
        /if (!${${ArrayName}[${i},${iMinMana}]} > ${Me.PctMana}) {
          /continue
        }

        |Skip if defiend IF isn't met
        /if (!${${ArrayName}[${i},${iIfs}]}) {
          /return
        } 

        |Skip if /NoEarlyRecast is set and the mob has this spell from me
        /if (${TimeLeftOnMySpell[${${ArrayName}[${i},${iCastName}]}]} > 0 && ${${ArrayName}[${i},${iNoEarlyRecast}]}) {
          /return
        }

        |Skip if /NoStack is set and ANYONE (including myself) have already casted this on the NPC
        /if (${${ArrayName}[${i},${iNoStack}]} && (${Bool[${Target.Buff[${FindItem[=${${ArrayName}[${i},${iCastName}]}].Spell}]}]} || ${Bool[${Target.Buff[=${${ArrayName}[${i},${iCastName}]}]}]})) {
          /return
        }

        |Skip if /TriggerSpell|SpellName is set and both the spell and TriggerSpell are already on the NPC from me
        /if (${Bool[${${ArrayName}[${i},${iTriggerSpell}]}]} && ${TimeLeftOnMySpell[${${ArrayName}[${i},${iTriggerSpell}]}]} > 4 && ${TimeLeftOnMySpell[${${ArrayName}[${i},${iCastName}]}]} > 4) {
          /return
        }

        |Skip if this spell is already on the NPC from me and has more than 4 seconds let on it. (This is item casts and not spells)
        /if (${Bool[${FindItem[=${${ArrayName}[${i},${iCastName}]}]}]} && ${TimeLeftOnMySpell[${FindItem[=${${ArrayName}[${i},${iCastName}]}].Spell}]} > 4) {
          /return
        }

        |Skip if this spell is already on the NPC from me and has more than 4 seconds let on it. (This is spell casts and not items)
        /if (${TimeLeftOnMySpell[${${ArrayName}[${i},${iCastName}]}]} > 4 && !${Bool[${${ArrayName}[${i},${iTriggerSpell}]}]}) {
          /return
        }
        /echo QuickCast:: ${${ArrayName}[${i},${iCastName}]}
        /call e3_Cast ${TargetID} ${ArrayName} ${i} True
        
      } else {
        /return
      }
    }

/next i

/RETURN
