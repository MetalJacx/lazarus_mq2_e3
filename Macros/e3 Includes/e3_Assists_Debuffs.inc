|----------------------------------------------------|
|- Enables Debuffing on a Target                    -|
|----------------------------------------------------|
SUB check_Debuffs
  /if (${Defined[Command_Debuffs2D]}) {
    /call Debuffs_OnCommand
  } 
 	/if (${Assisting} && ${Defined[Assist_Debuffs2D]}) /call cast_longTermSpells "${AssistTarget}" "Assist_Debuffs2D"
/if (${Debug} || ${Debug_Assists}) /echo <== check_Debuffs -|
/RETURN

|---------------------------------------------------------------------------------------------|
|- Validates targets and adds them to a list of Debuff_Targets. Used for Debuffs_OnCommand.	 -|
|---------------------------------------------------------------------------------------------|
|- Use, '/Debuff' or 'Debuffs on ${Target.ID}' in /tell, /group, or /bc.					 -|
|---------------------------------------------------------------------------------------------|
#EVENT DebuffsOn "<#1#> Debuffs on #2#"
#EVENT DebuffsOn "#1# tells you, 'Debuffs on #2#'"
SUB Event_DebuffsOn(line, ChatSender, MobID)
/if (${Debug} || ${Debug_Assists}) /echo |- event_DebuffsOn ==>
  /if (!${checkEventArgs[${ChatSender},${line},UZR,${MobID}]}) /return
    /varset MobID ${c_eventArgData}
     /declare lookupKey string local ${MobID}_${Zone.ID}

		/if (${Defined[Command_Debuffs2D]}) {
			| Check if the target is a corpse
			/if (${Spawn[id ${MobID}].Type.Equal[Corpse]} || !${Select[${Spawn[id ${MobID}].Type},${AcceptableTargetTypes}]}) {
        /docommand ${ChatToggle} Unacceptable Target ${Spawn[id ${MobID}].Type}
			} else {
				
              | Check range to the DoTs target.
        /if (${Spawn[id ${MobID}].Distance} > ${MaxAssistDistance}) {
			    /echo ${Spawn[${MobID}].CleanName} is too far away
          /docommand ${ChatToggle} ${Spawn[${MobID}].CleanName} is too far away.
        } else {
           
           /if (${Debuff_Target_Map.Contains[${lookupKey}]}) {
              /docommand ${ChatToggle} I am already debuffing ${Spawn[id ${MobID}].CleanName}.
           } else {

              /if (${Debuff_Target_Map.Add[${lookupKey},${lookupKey}]}) {
                /bc [+g+] -- Debuffing [${Spawn[${MobID}].CleanName}] --
              }
           }
           
        }
 			}
		}
	:Skip
/if (${Debug} || ${Debug_Assists}) /echo <== event_DebuffsOn -|
/RETURN



#EVENT debuffsOnAll "<|${Me.Name}|> Debuff On All"
SUB event_debuffsOnAll(line)
  /declare i int local
  /for i 1 to 13
    /if (${Me.XTarget[${i}].TargetType.Equal[Auto Hater]} && ${Me.XTarget[${i}].LineOfSight} && ${Me.XTarget[${i}].Aggressive} && ${Me.XTarget[${i}].ID}) {
      /bc Debuff on ${Me.XTarget[${i}].ID}
    }
  /next i
/RETURN

|--------------------------------------------------------------------|
|- Casts Command_Debuffs on targets listed in DebuffTargets array.	-|
|--------------------------------------------------------------------|
SUB Debuffs_OnCommand
/if (${Debug} || ${Debug_Assists}) /echo |- Debuffs_OnCommand ==>
	
  /if (${Debuff_Target_Map.Count}<1) {
    /if (${Debug} || ${Debug_Assists}) /echo <== DoT_OnCommand -|
    /return
  }
	/declare targetID int local
	/declare targetZone int local
  /declare keyValue string local
	/declare i int local
  /declare mi mapiterator local
  /vardata mi Debuff_Target_Map.First
    
    /while (!${mi.IsEnd}) {
        /varset keyValue ${mi.Key}
        /varset targetID ${keyValue.Arg[1,_]}
        /varset targetZone ${keyValue.Arg[2,_]}

        /if (${Zone.ID} != ${targetZone}) {
            /if (${Debuff_Target_Map_Removal.Add[${keyValue},${keyValue}]}) {}
        } else {
       
          | If the DoT_Target has died, remove them from the array.
          /if (!${Bool[${Spawn[id ${targetID}].Type.NotEqual[Corpse]}]}) {
            
            /if (${Debuff_Target_Map_Removal.Add[${keyValue},${keyValue}]}) {}
          
          } else {
            |/call cast_longTermSpells "${targetID}" "Command_DoTs2D"
            |Ewiclip MOD: New method for DoTs
            /call cast_longTermSpells ${targetID} "Command_Debuffs2D"
          }
        }
        /if (${ActionTaken}) /break
        /if (${mi.Advance}) {}

    }
    |remove the values from Debuff_Target_Map that we couldn't while iterating
    /if (${Debuff_Target_Map_Removal.Count}>0) {
      /declare miRemove mapiterator local
      /vardata miRemove Debuff_Target_Map_Removal.First
      /while (!${miRemove.IsEnd}) {
        /if (${Debuff_Target_Map.Remove[${miRemove.Key}]} ) {}
        /if (${miRemove.Advance}) {}
      }
      |Clear the removal map as we are now done processing
      /if (${Debuff_Target_Map_Removal.Clear} ) {}
    }
  
/if (${Debug} || ${Debug_Assists}) /echo <== Debuffs_OnCommand -|
/RETURN


|----------------------------------------------------|
|- Casts debuffs and dots on various assist targets -|
|----------------------------------------------------|
SUB cast_longTermSpells(int spellTarget, string ArrayName)
  /if (${Debug} || ${Debug_Assists}) /echo |- cast_longTermSpells - [${ArrayName}] ==>
|/varset Debug_Assists TRUE
  /declare spellTarg int local
  /declare spellTimer string local
  /declare spellRecast int local 0
  /declare s int local
  /declare checkForName string local
  /declare castIfName string local 
  /declare spellName string local 
  /declare spellID int local 0

	/for s 1 to ${${ArrayName}.Size[1]}
 /if (${${ArrayName}.Size[1]}==0) /break

    
     /varset spellName ${${ArrayName}[${s},${iCastName}]} 
     /if (${Bool[${FindItem[=${spellName}]}]}) {
        /varset spellID ${FindItem[${spellName}].Spell.ID}
        /varset spellName ${FindItem[${spellName}].Spell}
    } else {
      /varset spellID ${Spell[${spellName}].ID}
    }

   
	  |this check lets you use OOW debuff BP in debuff section
	  /if (${${ArrayName}[${s},${iSpellType}].Find[Beneficial]}) {
	    /varset spellTarg ${Me.ID}
      /if (${Debug} || ${Debug_Assists}) /echo ${${ArrayName}[${s},${iCastName}]} is beneficial, going directly to casting
	    /goto :attemptCast
    } else {
      /varset spellTarg ${spellTarget}
    }
		/varset spellTimer spellTimer_${spellTarg}_${spellID}
		/varset resistCount resist_${spellTarg}_${spellID}
    |/echo stdef ${Defined[${spellTimer}]} st ${spellTarg}  tgt ${Target.ID}
		/if (${Defined[${spellTimer}]}) {
		  /if (${${spellTimer}}) {
        /if (${Debug} || ${Debug_Assists}) /echo |- cast_longTermSpells - [${ArrayName}] -|	Waiting ${${spellTimer}} to recast ${${ArrayName}[${s},${iCastName}]} on ${spellTarg}.
        /goto :skipCast
      }
    }
    |check if the target is already debuffed, and if the duration is longer than the recast time
    /if (${spellTarg} != ${Target.ID}) {
      /call TrueTarget ${spellTarg}
      /delay 5 ${Target.BuffsPopulated}
    }
    
    /varset checkForName	${${ArrayName}[${s},${iCheckFor}]}
    /if (!${checkForName.Equal[-1]} ) {
          /if (${Bool[${Target.Buff[${checkForName}]}]}) {
            /goto :skipcast
          }   
    }
    /if (${Bool[${Select[${ArrayName},Command_Debuffs2D,Assist_Debuffs2D]}]}) {
      /if (${Target.BuffDuration[${${ArrayName}[${s},${iSpellName}]}]}/1000 > ${longTermRecastTime} && !${${ArrayName}[${s},${iSpellName}].Equal[${spammableSpell}]}) {
        /varcalc spellRecast ${Target.BuffDuration[${${ArrayName}[${s},${iSpellName}]}]}/1000 - ${longTermRecastTime}
        /call createTimer ${spellTimer} ${spellRecast}s
        /echo \ag ${Target.CleanName} already debuffed ${${ArrayName}[${s},${iSpellName}]} recast ${spellRecast}
        /goto :skipCast
      }

     
      |**/if (${Target.BuffDuration[${checkForName}]}/1000 > ${longTermRecastTime}) {
        /varcalc spellRecast ${Target.BuffDuration[${checkForName}]}/1000 - 10
        /call createTimer ${spellTimer} ${spellRecast}s
        /echo \ag ${Target.CleanName} already debuffed ${checkForName} recast ${spellRecast}
        /goto :skipCast
      }**|      
    }
    :attemptCast
    /if (!${${ArrayName}[${s},${iIfs}]}) {
        | need to have some sort of timer ... this will skip most encounters but raids will probably allow rechecking if conditions change
        /call createTimer "${spellTimer}" "30s"
        /if (${Debug} || ${Debug_Assists}) /echo \gy Skipping ${${ArrayName}[${s},${iSpellName}]} due to Ifs on ${Target.CleanName}
        /goto :skipCast
    }
    
    /varset castIfName	${${ArrayName}[${s},${iCastIf}]}
    /if (${Bool[${castIfName}]}  && !${castIfName.Equal[-1]} ) {
            |for using CastIfs in dots/debuffs, retarget to the AssistTarget before checking CastIfs
          /if (${Target.ID} != ${AssistTarget}) /call TrueTarget ${AssistTarget}
          /if (!${Bool[${Target.Buff[${castIfName}]}]}) {
            /echo \ag [${Time}] ${Target.CleanName} is not debuffed with [${castIfName}]  Skipping
            /goto :skipCast
          }   
    }
    /call check_Ready "${ArrayName}" ${s}
    /if (${c_Ready}) {
      /if (${check_Mana[${ArrayName},${s}]}) {
        /if (${check_Distance[${spellTarg},${${ArrayName}[${s},${iMyRange}]}]}) {
    
            /if (${Select[${Me.Class.ShortName},CLR,DRU,SHM]}) {
              |before we even begin to cast, verify again if we need to heal, and kickout if so
               /call check_HealCasting_DuringDetrimental
               /if (!${c_SubToRun}) {
                  /bc [+g+] [${Time}] Before Nuke cast check for Heals, Heals needed, kicking out.
                  /varset ActionTaken True
                  /if (${Debug} || ${Debug_Assists}) /echo <== cast_longTermSpells -|
                  /return
               }
               /varset ${ArrayName}[${s},${iSubToRun}] check_HealCasting_DuringDetrimental
            }
            |Cast the spell
            /call e3_Cast ${spellTarg} "${ArrayName}" "${s}"
            /if (${castReturn.Equal[CAST_INTERRUPTED]} && ${ActionTaken}) {
                |spell was interrupted due to a heal check kickout
                /return
            }
      
        } else {
          /if (${Debug} || ${Debug_Assists}) /echo ${spellTarg} out of range for ${${ArrayName}[${s},${iMyRange}]} ${${ArrayName}[${s},${iCastName}]}
          /goto :skipCast
        }
      } else {
        /if (${Debug} || ${Debug_Assists}) /echo lack mana for ${${ArrayName}[${s},${iCastName}]}
        /goto :skipCast
      }
    } else {
      /if (${Debug} || ${Debug_Assists}) /echo ${${ArrayName}[${s},${iCastName}]} not ready
      /goto :skipCast
    }
    |/if (${Debug} || ${Debug_Assists}) /echo castretun ${castReturn} res ${Cast.Result}
    |/if (${Debug} || ${Debug_Assists}) /echo maxtries ${${ArrayName}[${i},${iMaxTries}]} rescount ${resistCount} ${${resistCount}}
    |/if (${Debug} || ${Debug_Assists}) /echo castname ${${ArrayName}[${i},${iCastName}]} stored ${Cast.Stored.Name} return ${castReturn}
    | If the spell was resisted.	
    /if (${Cast.Stored.Name.Equal[${${ArrayName}[${s},${iSpellName}]}]} && ${castReturn.Equal[CAST_RESIST]}) {
      /if (!${Defined[${resistCount}]}) {
        /declare ${resistCount} int outer 1
      } else {
        /varcalc ${resistCount} ${${resistCount}}+1
        /if (${${resistCount}} >= ${${ArrayName}[${s},${iMaxTries}]}) {
          /deletevar ${resistCount}
          /call createTimer "${spellTimer}" "1m"
        }
      }
    | If the spell did not take hold.
    } else /if (${Cast.Stored.Name.Equal[${${ArrayName}[${s},${iSpellName}]}]} && ${castReturn.Equal[CAST_TAKEHOLD]} && !${Cast.Stored.Name.Equal[${spammableSpell}]}) {
      /if (${Defined[${resistCount}]}) {
        /deletevar ${resistCount}
      }
      /call createTimer "${spellTimer}" "3m"
      | If the target was immune.
    } else /if (${Cast.Stored.Name.Equal[${${ArrayName}[${s},${iSpellName}]}]} && ${castReturn.Equal[CAST_IMMUNE]} && !${Cast.Stored.Name.Equal[${spammableSpell}]}) {
      /if (${Defined[${resistCount}]}) {
        /deletevar ${resistCount}
      }
      /call createTimer "${spellTimer}" "30m"
    | If the cast was successful.
    } else /if (${Cast.Stored.Name.Equal[${${ArrayName}[${s},${iSpellName}]}]} && ${castReturn.Equal[CAST_SUCCESS]} && !${Cast.Stored.Name.Equal[${spammableSpell}]}) {
      /if (${Defined[${resistCount}]}) {
        /deletevar ${resistCount}
      }
      |if i dont have spell awarenes use default buff duration
      /varcalc spellRecast ${Target.BuffDuration[${${ArrayName}[${s},${iSpellName}]}]}/1000
      /if (${spellRecast}< ${Math.Calc[${${ArrayName}[${s},${iDuration}]}*6]}) /varcalc spellRecast ${${ArrayName}[${s},${iDuration}]}*6
      |/echo ${Target.Name}  ${Target.BuffDuration[${${ArrayName}[${i},${iSpellName}]}]} ${spellRecast}
      |duration over 90 seconds is considered long term
      /if (${spellRecast}>90) {
        /varcalc spellRecast ${spellRecast}-${longTermRecastTime}
      } else {
        /varcalc spellRecast ${spellRecast}-${shortTermRecastTime}
      }
      /if (${Debug} || ${Debug_Assists}) /echo |- longTermSpells - [${ArrayName}] -| Creating timer [${spellTimer}] -- [${spellRecast}]
      |/echo Creating timer [${spellTimer}] -- [${spellRecast}]
      /call createTimer "${spellTimer}" "${spellRecast}s"
    }
  :skipCast
  /if (${ActionTaken}) /break
 /next s
/if (${Debug} || ${Debug_Assists}) /echo <== cast_longTermSpells - [${ArrayName}] -|
/RETURN

|------------------------------------------------|
|- Cancels debuffs on command on all targets. 	-|
|------------------------------------------------|
#EVENT DebuffsOff "<#1#> End debuffs"
#EVENT DebuffsOff "<#1#> End debuffs on #2#"
SUB event_DebuffsOff(line, ChatSender, MobID)
/if (${Debug} || ${Debug_Assists}) /echo |- event_DebuffsOff ==>
	/if (${Debuff_Target_Map.Count}>0) {
	  /if (!${Defined[MobID]}) /declare MobID int
    /if (!${checkEventArgs[${ChatSender},${line},UZR,${MobID}]}) /return
    /if (${Bool[${c_eventArgData}]}) {
      /docommand ${ChatToggle} Ending Debuffs on ${Spawn[id ${c_eventArgData}].CleanName}.
      /if (${Debuff_Target_Map.Remove[${c_eventArgData}_${Zone.ID}]}) {}
    } else {
      /docommand ${ChatToggle} Ending Debuffs on all targets.
      /if (${Debuff_Target_Map.Clear}) {}
    }
  }
/if (${Debug} || ${Debug_Assists}) /echo <== event_DebuffsOff -|
/RETURN




