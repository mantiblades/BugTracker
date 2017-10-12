/* Copyright (C) 2006 - 2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* ScriptData
SDName: Boss_Broggok
SD%Complete: 100
SDComment:
SDCategory: Hellfire Citadel, Blood Furnace
EndScriptData */


#include "def_blood_furnace.h"

#define SAY_AGGRO               -1542008

#define SPELL_SLIME_SPRAY       30913
#define SPELL_SLIME_SPRAY_H     38458
#define SPELL_POISON_CLOUD      30916
#define SPELL_POISON_BOLT       30917
#define SPELL_POISON_BOLT_H     38459


class boss_broggok : public CreatureScript
{
public:
    boss_broggok() : CreatureScript("boss_broggok")
    { }

    class boss_broggokAI : public ScriptedAI
    {
        public:
        boss_broggokAI(Creature *c) : ScriptedAI(c) 
        {
            pInstance = ((InstanceScript*)c->GetInstanceScript());
            HeroicMode = me->GetMap()->IsHeroic();
        }
        
        InstanceScript* pInstance;
    
        uint32 AcidSpray_Timer;
        uint32 PoisonSpawn_Timer;
        uint32 PoisonBolt_Timer;
        
        bool HeroicMode;
    
        void Reset() override
        {
            me->SetUnitMovementFlags(MOVEMENTFLAG_NONE);
            AcidSpray_Timer = 5000;
            PoisonSpawn_Timer = 10000;
            PoisonBolt_Timer = 8000;
            if (pInstance && me->IsAlive())
                pInstance->SetData(DATA_BROGGOKEVENT, NOT_STARTED);
        }
    
        void EnterCombat(Unit *who) override
        {
            DoScriptText(SAY_AGGRO, me);
    
            if (pInstance)
                pInstance->SetData(DATA_BROGGOKEVENT, IN_PROGRESS);
    
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        }
        
        void JustDied(Unit* Killer) override
        {
            if (pInstance)
                pInstance->SetData(DATA_BROGGOKEVENT, DONE);
        }
        
        void EnterEvadeMode(EvadeReason /* why */) override
        {
            me->RemoveAllAuras();
            me->GetThreatManager().ClearAllThreat();
            me->CombatStop(true);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            Reset();
    
            if (!me->IsAlive())
                return;    
    
            if (pInstance) {
                pInstance->SetData(DATA_BROGGOKEVENT, FAIL);
                float fRespX, fRespY, fRespZ;
                me->GetRespawnPosition(fRespX, fRespY, fRespZ);
                me->GetMotionMaster()->MovePoint(0, fRespX, fRespY, fRespZ);
            }
            else
                me->GetMotionMaster()->MoveTargetedHome();
        }
    
        void MovementInform(uint32 uiMotionType, uint32 uiPointId)
        override {
            if (uiMotionType == POINT_MOTION_TYPE) {
                if (GameObject* pFrontDoor = me->FindNearestGameObject(181819, 60.0f)) {
                    me->SetOrientation(me->GetAngle(pFrontDoor->GetPositionX(), pFrontDoor->GetPositionY()));
                    me->SendMovementFlagUpdate();
                }
            }
        }
    
        void UpdateAI(const uint32 diff)
        override {
    
            if (!UpdateVictim())
                return;
    
            if (AcidSpray_Timer <= diff) {
                DoCast(me->GetVictim(), HeroicMode ? SPELL_SLIME_SPRAY_H : SPELL_SLIME_SPRAY);
                AcidSpray_Timer = 10000;
            }
            else
                AcidSpray_Timer -= diff;
    
            if (PoisonBolt_Timer <= diff) {
                DoCast(me->GetVictim(), HeroicMode ? SPELL_POISON_BOLT_H : SPELL_POISON_BOLT);
                PoisonBolt_Timer = 8000;
            }
            else
                PoisonBolt_Timer -= diff;
    
            if (PoisonSpawn_Timer <= diff) {
                DoCast(me, SPELL_POISON_CLOUD);
                PoisonSpawn_Timer = 10000;
            }
            else
                PoisonSpawn_Timer -= diff;
    
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_broggokAI(creature);
    }
};


/*######
## mob_nascent_orc
######*/

#define SPELL_BLOW     22427
#define SPELL_STOMP    31900


class mob_nascent_orc : public CreatureScript
{
public:
    mob_nascent_orc() : CreatureScript("mob_nascent_orc")
    { }

    class mob_nascent_orcAI : public ScriptedAI
    {
        public:
        mob_nascent_orcAI(Creature *c) : ScriptedAI(c)
        {
            pInstance = ((InstanceScript*)c->GetInstanceScript());
            HeroicMode = me->GetMap()->IsHeroic();
        }
    
        InstanceScript* pInstance;
    
        uint32 Blow_Timer;
        uint32 Stomp_Timer;
        
        bool HeroicMode;
    
        void Reset() override
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            me->SetUnitMovementFlags(MOVEMENTFLAG_NONE);
            Blow_Timer = 4000 + rand()%4000;
            Stomp_Timer = 5000 + rand()%4000;
        }
    
        void MovementInform(uint32 uiMotionType, uint32 uiPointId) override
        {
            if (uiMotionType == POINT_MOTION_TYPE)
            {
                if (Unit *pTarget = me->SelectNearestTarget(99.0f))
                {
                    me->AI()->AttackStart(pTarget);
                }
           }
        }
    
        void EnterEvadeMode(EvadeReason /* why */) override
        {
            if (pInstance)
                pInstance->SetData(DATA_BROGGOKEVENT, FAIL);
    
            me->GetThreatManager().ClearAllThreat();
            me->CombatStop(true);
            me->GetMotionMaster()->MoveTargetedHome();
            Reset();
        }
    
        void UpdateAI(const uint32 diff) override
        {
            if (!UpdateVictim())
                return;
    
            if (Blow_Timer <= diff)
            {
                DoCast(me->GetVictim(),SPELL_BLOW);
                Blow_Timer = 10000+rand()%4000;
            } else Blow_Timer -=diff;
    
            if (Stomp_Timer <= diff)
            {
                DoCast(me->GetVictim(),SPELL_STOMP);
                Stomp_Timer = 15000+rand()%4000;
            } else Stomp_Timer -=diff;
    
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new mob_nascent_orcAI(creature);
    }
};


/*######
## mob_fel_orc_neophyte
######*/

#define SPELL_CHARGE    22120
#define SPELL_FRENZY    8269


class mob_fel_orc_neophyte : public CreatureScript
{
public:
    mob_fel_orc_neophyte() : CreatureScript("mob_fel_orc_neophyte")
    { }

    class mob_fel_orc_neophyteAI : public ScriptedAI
    {
        public:
        mob_fel_orc_neophyteAI(Creature *c) : ScriptedAI(c)
        {
            pInstance = ((InstanceScript*)c->GetInstanceScript());
            HeroicMode = me->GetMap()->IsHeroic();
        }
    
        InstanceScript* pInstance;
    
        uint32 ChargeTimer;
        
        bool HeroicMode;
        bool Frenzy;
    
        void Reset() override
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            me->SetUnitMovementFlags(MOVEMENTFLAG_NONE);
            ChargeTimer = 8000;
        }
        
        void MovementInform(uint32 uiMotionType, uint32 uiPointId) override
        {
            if (uiMotionType == POINT_MOTION_TYPE)
            {
                if (Unit *pTarget = me->SelectNearestTarget(99.0f))
                {
                    me->AI()->AttackStart(pTarget);
                }
           }
        }
    
        void EnterEvadeMode(EvadeReason /* why */) override
        {
            if (pInstance)
                pInstance->SetData(DATA_BROGGOKEVENT, FAIL);
    
            me->GetThreatManager().ClearAllThreat();
            me->CombatStop(true);
            me->GetMotionMaster()->MoveTargetedHome();
            Reset();
        }
    
        void UpdateAI(const uint32 diff)
        override {
            if (!UpdateVictim())
                return;
    
            if (ChargeTimer <= diff) {
                DoCast(SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0, true, true), SPELL_CHARGE);
                ChargeTimer = 25000 + rand()+5000;
            }
            else
                ChargeTimer -= diff;
                
            if (!Frenzy && me->IsBelowHPPercent(30)) {
                DoCast(me, SPELL_FRENZY);
                Frenzy = true;
            }
    
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new mob_fel_orc_neophyteAI(creature);
    }
};


/*######
## mob_broggok_poisoncloud
######*/

#define SPELL_POISON      30914
#define SPELL_POISON_H    38462


class mob_broggok_poisoncloud : public CreatureScript
{
public:
    mob_broggok_poisoncloud() : CreatureScript("mob_broggok_poisoncloud")
    { }

    class mob_broggok_poisoncloudAI : public ScriptedAI
    {
        public:
        mob_broggok_poisoncloudAI(Creature *c) : ScriptedAI(c)
        {
            pInstance = ((InstanceScript*)c->GetInstanceScript());
            HeroicMode = me->GetMap()->IsHeroic();
        }
    
        InstanceScript* pInstance;
        
        bool HeroicMode;
    
        void Reset() override
        {
            DoCast(me, HeroicMode ? SPELL_POISON_H : SPELL_POISON);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new mob_broggok_poisoncloudAI(creature);
    }
};


void AddSC_boss_broggok()
{
    new boss_broggok();
    
    new mob_nascent_orc();
    
    new mob_fel_orc_neophyte();

    new mob_broggok_poisoncloud();
}

