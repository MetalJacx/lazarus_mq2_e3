|--------------------------------------------------------|
|- Casts direct damage spells on a specified target.	  -|
|--------------------------------------------------------|
SUB check_Nukes

/if (${Debug} || ${Debug_Assists}) /echo |- check_Nukes ==>
	/if (${Defined[Nukes2D]} && ${Assisting} && ${use_Nukes}) {
   
    /declare castIfName string local  
	  /declare castIndex int local
	  /declare s int local
    /declare m int local
    /declare assistToT int local
    /declare typeOfCast string local 
    /declare HaveGiftOfManaBuff bool False local
    
    /if (${Bool[${Me.Song[Gift of Mana].ID}]} || ${Bool[${Me.Song[Celestial Gift].ID}]} || ${Bool[${Me.Song[Celestial Boon].ID}]}) {
        /varset HaveGiftOfManaBuff True
    }

   /for s 1 to ${Nukes2D.Size[1]}
 /if (${Nukes2D.Size[1]}==0) /break
    
    /varset typeOfCast  
    
    /varset castIndex ${s}
    /if (${Debug} || ${Debug_Assists}) /echo ${s} ${castIndex} ${Nukes2D[${castIndex},${iCastName}]}

    

    |allows use of druid/mage DS, etc, in nukes based on spellset. casts on assistTargets target
    /if (${Select[${Me.Class.ShortName},DRU,MAG]} && ${Nukes2D[${castIndex},${iSpellType}].Find[Beneficial]} && !${Nukes2D[${castIndex},${iCastName}].Find[ Magic]}) {
     
      /if (${Target.ID} != ${AssistTarget}) /call TrueTarget ${AssistTarget}
      /if (${Select[${Me.TargetOfTarget.Class.ShortName},WAR,PAL,SHD]}) {
        /if (${Spawn[pc ${Me.TargetOfTarget}].ID}) {
          /varset assistToT ${Me.TargetOfTarget.ID}
          /call check_Ready "Nukes2D" ${castIndex}
          /if (${c_Ready} && ${Nukes2D[${castIndex},${iIfs}]}) {
            /if (${check_Mana["Nukes2D",${castIndex}]}) {
              /if (${check_Distance[${assistToT},${Nukes2D[${s},${iMyRange}]}]}) {
                /call e3_Cast ${assistToT} "Nukes2D" "${castIndex}"
              }
            }
          }
        }
      }
    } else {

  
      | skip casting if a delay is defined and active
      /if (${Defined[castDelay]} && ${Bool[${Nukes2D[${castIndex},${iDelay}]}]}) {
        /if (${castDelay}) {
          /if (${Debug} || ${Debug_Assists}) /echo skipping nukes, /delay from previous cast
          /goto :skipcast
          |/return
        }
      }
      |if im burning and /noburn is set, skip the spell
      |if noburn and rotate is defined, then skip this spell and set lastSuccessfulCast to this spell, so the next spell can be used
      /if ((${use_QUICKBurns} || ${use_LONGBurns} || ${use_FULLBurns}) && ${Nukes2D[${castIndex},${iNoBurn}]}) {
        /if (${Nukes2D[${castIndex},${iRotate}]} && ${Nukes2D.Size[1]} > 1) {
          /if (${Debug} || ${Debug_Assists}) /echo setting lastSuccessfulCast to ${lastSuccessfulCast} because noburn and rotate defined
          /varset lastSuccessfulCast ${Nukes2D[${castIndex},${iCastName}]}
        } else {
          /if (${Debug} || ${Debug_Assists}) /echo skipping cast of ${Nukes2D[${castIndex},${iCastName}]}, i am burning and /NoBurn is set
          /goto :skipCast
        }
      }
      | if /rotate is defined, and the current array index was the last successful cast, skip to next index
    /if (${Nukes2D[${castIndex},${iCastName}].Equal[${lastSuccessfulCast}]} && ${Nukes2D[${castIndex},${iRotate}]} && ${Nukes2D.Size[1]} > 1) {
      /if (${Debug} || ${Debug_Assists}) /echo Possibly skipping cast of ${Nukes2D[${castIndex},${iCastName}]} /rotate is defined
      |need to check if any of our other spells can cast, if not, we can ignore this rotate
      /declare otherSpellsReady bool local
      /varset otherSpellsReady FALSE
      /declare currentCastName string local
      /varset currentCastName ${Nukes2D[${castIndex},${iCastName}]}
      /declare i int local
      /for i 1 to ${Nukes2D.Size[1]}
 /if (${Nukes2D.Size[1]}==0) /break
        |if not the currently casting spell, lets check if they are ready to cast.          
        /if (!${Nukes2D[${i},${iCastName}].Equal[${currentCastName}]}) {
          
          /call check_Ready "Nukes2D" ${i}
          |needed for target checks on ifs
          /if (${Target.ID} != ${AssistTarget}) /call TrueTarget ${AssistTarget}
          /if (${c_Ready} && ${Nukes2D[${i},${iIfs}]}) {
            /if (${Bool[${Nukes2D[${i},${iGiftOfMana}]}]}) {
              |its specified to use gift of mana, well do we have it?
              /if (${HaveGiftOfManaBuff}) {
                /varset otherSpellsReady TRUE
                /if (${Debug} || ${Debug_Assists}) /echo Another spell is defined as ready:${Nukes2D[${i},${iCastName}]}
                /break
              }
            } else {
              /varset otherSpellsReady TRUE
              /if (${Debug} || ${Debug_Assists}) /echo Another spell is defined as ready:${Nukes2D[${i},${iCastName}]}
              /break
            }
          }
        }
      /next i
   
    
      |if others spells are ready to cast, its ok to skip this.
        /if (${otherSpellsReady}) {
          /if (${Debug} || ${Debug_Assists}) /echo another spell is ready to cast
           
          /goto :skipCast
        } else {
          /if (${Debug} || ${Debug_Assists}) /echo no other spell ready, allowing the current rotate spell to cast.
        }  
      }

    | if i have Gift Of Mana find a spell with /GoM, that is ready
      /if (${HaveGiftOfManaBuff}) {
        /if (${Debug} || ${Debug_Assists}) /echo have gift of mana, selecting GOM spell flag
        /for m 1 to ${Nukes2D.Size[1]}
 /if (${Nukes2D.Size[1]}==0) /break
          /if (${Nukes2D[${m},${iGiftOfMana}]}) {
        
            /call check_Ready "Nukes2D" ${m}
            /if (${c_Ready}) {
              /if (${Debug} || ${Debug_Assists}) /echo found a ready spell for GOM
              /varset castIndex ${m}
              |/bc CheckNukes_GOMBUFF: resetting  lastSuccessfulCast 
              /varset lastSuccessfulCast 0
              /break
            } 
          }
        /next m
      }
      |if this is a spell with /GoM, yet we don't have GoM buff, skip this spell.
      /if (!${HaveGiftOfManaBuff} && ${Bool[${Nukes2D[${castIndex},${iGiftOfMana}]}]} ) {
          | we do not have gift of mana, make sure our current index is not a GOM spell, if so skip it
          /if (${Debug} || ${Debug_Assists}) /echo skipping gift of mana spell ${Nukes2D[${castIndex},${iCastName}]}
          /goto :skipcast
      }
       
      |if i have aggro and /noaggro is set, skip the spell
      /if (${Nukes2D[${castIndex},${iNoAggro}]}) {
        /if (${Me.TargetOfTarget.CleanName.Equal[${Me.CleanName}]}) {
          /if (${Debug} || ${Debug_Assists}) /echo skipping cast of ${Nukes2D[${castIndex},${iCastName}]}, i have aggro and /noaggro is set
          /goto :skipCast
        }
      }
      |only cast if my aggro is greater than PctAggro and the target is a Named
      /if (${Nukes2D[${castIndex},${iPctAggro}]} > 0) {
        /if (${Me.PctAggro} < ${Nukes2D[${castIndex},${iPctAggro}]} || !${Target.Named}) {
          /if (${Debug} || ${Debug_Assists}) /echo skipping cast of ${Nukes2D[${castIndex},${iCastName}]}, my aggro ${Me.PctAggro} is < PctAggro ${Nukes2D[${castIndex},${iPctAggro}]}
          /if (${Debug} || ${Debug_Assists}) /echo ${Nukes2D[${castIndex},${iPctAggro}]} ${Me.PctAggro} < ${Nukes2D[${castIndex},${iPctAggro}]} !${Target.Named}
          /goto :skipCast
        }
      }

      | if i have spell damage reduction recourse and the mob has more than 25% health or i have < 10% mana, skip cast.  cast anyway if i have GoM to avoid losing it
      /if (${Me.Buff[Weave of Weakness].ID} || ${Me.Buff[Flames of Weakness].ID}) {
        /if ((${Target.PctHPs} > 25 || ${Me.PctMana} < 10) && !${Bool[${Me.Song[Gift of Mana].ID}]} && !${use_QUICKBurns} && !${use_LONGBurns} && !${use_FULLBurns}) {
          /if (${Debug} || ${Debug_Assists}) /echo skipping cast of ${Nukes2D[${castIndex},${iCastName}]}, i have spell damage reduction
          /goto :skipCast
        }
      }

     

      /varset castIfName	${Nukes2D[${s},${iCastIf}]}
      /if (${Bool[${castIfName}]} && !${castIfName.Equal[-1]} ) {
             |for using CastIfs in Nukes, retarget to the AssistTarget before checking CastIfs
  	        /if (${Target.ID} != ${AssistTarget}) /call TrueTarget ${AssistTarget}
            /if (!${Bool[${Target.Buff[${castIfName}]}]}) {
              /if (${Debug} || ${Debug_Assists}) /echo [+g+] [${Time}] ${Target.CleanName} is not debuffed with ${castIfName} , skipping
              /goto :skipCast
            }   
      }
      /if (${Debug} || ${Debug_Assists}) /echo Doing a ready check
      /call check_Ready "Nukes2D" ${castIndex}
	  
      /if (${Debug} || ${Debug_Assists}) {
      /echo Checking Nuke ${Nukes2D[${castIndex},${iIfs}]}
      /echo Mana ${check_Mana["Nukes2D",${castIndex}]}
      /echo Target  ${AssistTarget}
      /echo Cast index ${castIndex}
      /echo Check Distance ${check_Distance[${AssistTarget},${Nukes2D[${s},${iMyRange}]}]}
      /echo Range ${Nukes2D[${s},${iMyRange}]}
      }
	  
      /if (${c_Ready}) {
        /if (${check_Mana["Nukes2D",${castIndex}]}) {
          /if (${check_Distance[${AssistTarget},${Nukes2D[${s},${iMyRange}]}]}) {
             /if (${Select[${Me.Class.ShortName},CLR,DRU,SHM]}) {
              |before we even begin to cast, verify again if we need to heal, and kickout if so
               /call check_HealCasting_DuringDetrimental
               /if (!${c_SubToRun}) {
                  /bc [${Time}] Before Nuke cast check for Heals, Heals needed, kicking out.
                  /varset ActionTaken True
                  /if (${Debug} || ${Debug_Assists}) /echo <== check_Nukes -|
                  /return
               }
                |set the checks for during the cast for interrupts
               /varset Nukes2D[${castIndex},${iSubToRun}] check_HealCasting_DuringDetrimental
            }
            /varset typeOfCast ${Nukes2D[${castIndex},${iCastType}]}
            |for ifs check they may want to check the target buffs, so have to make sure we are targeting before we cast
            /if (${Target.ID} != ${AssistTarget}) /call TrueTarget ${AssistTarget}
            /if (${Nukes2D[${castIndex},${iIfs}]}) {
                  /call e3_Cast ${AssistTarget} "Nukes2D" "${castIndex}"
            }
        
          }
        }
      }
      
      |if the nuke was succesful, see if there is a delay defined and create a timer which locks out all nukes for the duration
      |mainly used for PBAOE stuns
      /if (${Bool[${Nukes2D[${castIndex},${iDelay}]}]} && ${Select[${castReturn},CAST_SUCCESS]}) {
        /if (${Debug} || ${Debug_Assists}) /echo delaying ${Nukes2D[${castIndex},${iCastName}]} for ${Nukes2D[${castIndex},${iDelay}]}
        /call CreateTimer "castDelay" "${Nukes2D[${castIndex},${iDelay}]}"
      }
    }
	:skipCast
  /if (${ActionTaken}) /break
 /next s
  }
/if (${Debug} || ${Debug_Assists}) /echo <== check_Nukes -|
/RETURN





|----------------------------|
|- Turns AE functions on.	  -|
|----------------------------|
#event AEs "<#1#> #2#AE #3#"
#event AEs "#1# tells you, '#2#AE #3#'"
SUB Event_AEs(line, ChatSender, aeType, aeToggle)
/if (${Debug} || ${Debug_Assists}) /echo |- Event_AllAEOn ==>
  /if (!${Select[${aeType},Target,PB,All]}) /return
  /if (${aeType.Equal[Target]} && !${Defined[TargetAE_Spells2D]})  /return
  /if (${aeType.Equal[PB]} && !${Defined[PBAE_Spells2D]})          /return
  /if (${aeType.Equal[All]} && (!${Defined[PBAE_Spells2D]} && !${Defined[TargetAE_Spells2D]})) /return

  /if (!${checkEventArgs[${ChatSender},${line},UZR,${aeToggle}]}) /return
  /varset aeToggle ${c_eventArgData}

  /if (${aeToggle.Equal[on]}) {
    /bc [+g+] [${aeType}AE]-${aeToggle.Upper}
    /if (${aeType.Equal[PB]} || ${aeType.Equal[All]}) {
      /varset use_PBAE TRUE
      /call TrueTarget ${NetBots[${c_eventArgChatSender}].ID}
      /call MoveToLoc ${Target.Y} ${Target.X} 30 18
    }
    /if (${aeType.Equal[Target]} || ${aeType.Equal[All]}) {
      /varset use_TargetAE TRUE
	  /if (${Debug} || ${Debug_Assists}) /echo use_TargetAE = TRUE
      /varset aeTarget ${Spawn[=${ChatSender}].ID}
      /if (${Me.Class.ShortName.Equal[RNG]}) {
        /if (${swapTgtAeRanged}) /call swapToRangedSet TRUE
      }
    }
  } else /if (${aeToggle.Equal[off]}) {
    /if (${use_PBAE} || ${use_TargetAE}) {
      /if (${Me.Casting.ID}) /call interrupt
      /bc [+y+] [${aeType}AE]-${aeToggle.Upper}
    }
    /if (${aeType.Equal[PB]} || ${aeType.Equal[All]}) {
      /varset use_PBAE FALSE
    }
    /if (${aeType.Equal[Target]} || ${aeType.Equal[All]}) {
      /varset use_TargetAE FALSE
      /varset aeTarget 0
      /varset aeTarget ${Spawn[=${ChatSender}].ID}
      /if (${revertRngToMelee}) /call swapToRangedSet FALSE
    }
  }
/if (${Debug} || ${Debug_Assists}) /echo <== Event_AllAEOn -|
/RETURN

|------------------------------------------------------------------------------------------------------------|
|- Casts spells listed in the [AE] section of the Character_Ini on mobs which wander into your AE_Radius.	-|
|------------------------------------------------------------------------------------------------------------|
SUB check_AE
/if (${Debug} || ${Debug_Assists}) /echo |- check_AE ==>
  /if ((${use_PBAE} || ${use_TargetAE}) && !${Bool[${SpawnCount[npc radius ${AE_Radius}]}]} && ${Me.CombatState.NotEqual[COMBAT]}) {
    /varset use_TargetAE FALSE
    /varset use_PBAE FALSE
    /varset aeTarget 0
    /docommand ${ChatToggle} Ending AE: No mobs in radius ${AE_Radius}
    /if (${revertRngToMelee}) /call swapToRangedSet FALSE
  }
  /declare s int local
	/if (${use_PBAE} && ${Defined[PBAE_Spells2D]}) {
    /for s 1 to ${PBAE_Spells2D.Size[1]}
 /if (${PBAE_Spells2D.Size[1]}==0) /break
      /call check_Ready "PBAE_Spells2D" ${s}
      /if (${c_Ready} && ${PBAE_Spells2D[${s},${iIfs}]}) {
        /if (${check_Mana["PBAE_Spells2D",${s}]}) {
          /squelch /face fast id ${AssistTarget}
          /call e3_Cast ${aeTarget} "PBAE_Spells2D" "${s}"
          /if (${Cast.Stored.Name.Equal[${PBAE_Spells2D[${s},${iSpellName}]}]} && ${castReturn.Equal[CAST_SUCCESS]}) {
            /if (${PBAE_Spells2D[${s},${iSpellName}].Equal[Idol of Malos]}) {
              /call CreateTimer "nht${aeTarget}-${PBAE_Spells2D[${s},${iCastID}]}" "1400"
            }
          }
        }
      }
    /if (${ActionTaken}) /break
 /next s
	}
  /if (${use_TargetAE} && ${Defined[TargetAE_Spells2D]}) {
    |if i have an assist target, use that as ae target, else use aeTarget
    /declare useBestTarget int local
	/if (${Debug} || ${Debug_Assists}) /echo AssistTarget set = ${Bool[${AssistTarget}]}
    /if (${Bool[${AssistTarget}]}) {
      /varset useBestTarget ${AssistTarget}
    } else {
      /varset useBestTarget ${aeTarget}
    }
    /for s 1 to ${TargetAE_Spells2D.Size[1]}
 /if (${TargetAE_Spells2D.Size[1]}==0) /break
      /call check_Ready "TargetAE_Spells2D" ${s}
      /if (${c_Ready} && ${TargetAE_Spells2D[${s},${iIfs}]}) {
        /if (${check_Mana["TargetAE_Spells2D",${s}]}) {
		  /if (${Debug} || ${Debug_Assists}) /echo Distance Check = ${check_Distance[${useBestTarget},${TargetAE_Spells2D[${s},${iMyRange}]}]}
		  /if (${Debug} || ${Debug_Assists}) /echo Range = ${TargetAE_Spells2D[${s},${iMyRange}]}
          /if (${check_Distance[${useBestTarget},${TargetAE_Spells2D[${s},${iMyRange}]}]}) {
            /squelch /face fast id ${useBestTarget}
            /call e3_Cast ${useBestTarget} "TargetAE_Spells2D" "${s}"
          }
        }
      }
    /if (${ActionTaken}) /break
 /next s
  }
/if (${Debug} || ${Debug_Assists}) /echo <== check_AE -|
/RETURN
