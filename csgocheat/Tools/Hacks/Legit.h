#pragma once
#include "stdafx.h"
#include "../Utils/Hitbox.h"
#include "Misc.h"
#include "../Utils/LocalInfo.h"
#include "../Utils/Playerlist.h"
//this is very buggy, fix in future


class Legit : public Singleton<Legit> {
	int GetBestPoint(Vector& aimspot, Hitbox box) {
		Vector Aimangles;
		Misc::CalcAngle(Hacks.LocalPlayer->GetEyePosition(), box.points[0], Aimangles);
		if (Misc::FovTo(Hacks.CurrentCmd->viewangles, Aimangles) > Vars.Ragebot.LegitFov) return false;
		float bestdamage = 0;
		for (int index = 0; index < 27; ++index) {
			Vector Aimangles;
			Misc::CalcAngle(Hacks.LocalPlayer->GetEyePosition(), box.points[index], Aimangles);
			if (Misc::FovTo(Hacks.CurrentCmd->viewangles, Aimangles) > Vars.Ragebot.LegitFov) continue;
			float damage = Autowall::GetDamage(box.points[index]);
			if (damage > bestdamage) {
				aimspot = box.points[index];
				bestdamage = damage;
			}
		}
		return bestdamage;
	}

	bool CanShoot()
	{
		if (!Interfaces.pGlobalVars)
			return false;
		if (!Hacks.LocalPlayer)
			return false;
		if (!Hacks.LocalPlayer->isAlive())
			return false;
		if (!Hacks.LocalPlayer || !Hacks.LocalWeapon)
			return false;
		if (Hacks.LocalWeapon->IsMiscWeapon() || !Hacks.LocalWeapon->HasAmmo())
			return false;

		auto flServerTime = (float)Hacks.LocalPlayer->GetTickBase() * Interfaces.pGlobalVars->interval_per_tick;
		auto flNextPrimaryAttack = Hacks.LocalWeapon->NextPrimaryAttack();

		return(!(flNextPrimaryAttack > flServerTime));
	}

	void SmoothAngleSet(Vector dest, Vector orig)
	{
		if (Vars.Ragebot.LegitSmooth > 0)
		{
			Vector delta;
			delta.x = orig.x - dest.x;
			delta.y = orig.y - dest.y;
			dest.x = orig.x - delta.x / (5.f * Vars.Ragebot.LegitSmooth);
			dest.y = orig.y - delta.y / (5.f * Vars.Ragebot.LegitSmooth);
		}
		Interfaces.pEngine->SetViewAngles(dest);
	}

	static Vector Normalize(Vector& angs)
	{
		while (angs.y < -180.0f)
			angs.y += 360.0f;
		while (angs.y > 180.0f)
			angs.y -= 360.0f;
		if (angs.x > 89.0f)
			angs.x = 89.0f;
		if (angs.x < -89.0f)
			angs.x = -89.0f;
		angs.z = 0;
		return angs;
	}

	Vector Randomize(Vector vAngles, float curX, float destX, float curY, float destY, float lastX, float lastY) 
	{
		// fix sometime
		if (abs(curX - destX) < .05f)
		{
			destX = rand() % (int)(Vars.Ragebot.LegitRandomizeAmt * 2) + 1;
			destX /= 500;
			int positive = rand() % 2 + 1;
			if (positive == 2)
				destX = -destX;
		}

		if (abs(curY - destY) < .05f)
		{
			destY = rand() % (int)(Vars.Ragebot.LegitRandomizeAmt * 2) + 1;
			destY /= 500;
			int positive = rand() % 2 + 1;
			if (positive == 2)
				destY = -destY;
		}

		int speed = 1 - int(1);
		curX += (destX - curX) / ((15 * speed) + 10);

		curY += (destY - curY) / ((15 * speed) + 10);

		vAngles.x += curX;
		vAngles.y += curY;

		lastX = curX;
		lastY = curY;



		return vAngles;

	}

	template <typename t> t clamp(t value, t min, t max) {
		if (value > max) {
			return max;
		}
		if (value < min) {
			return min;
		}
		return value;
	}

	float LerpTime()
	{
		auto cl_updaterate = Interfaces.g_ICVars->FindVar("cl_updaterate");
		auto sv_minupdaterate = Interfaces.g_ICVars->FindVar("sv_minupdaterate");
		auto sv_maxupdaterate = Interfaces.g_ICVars->FindVar("sv_maxupdaterate");

		auto cl_interp = Interfaces.g_ICVars->FindVar("cl_interp");
		auto sv_client_min_interp_ratio = Interfaces.g_ICVars->FindVar("sv_client_min_interp_ratio");
		auto sv_client_max_interp_ratio = Interfaces.g_ICVars->FindVar("sv_client_max_interp_ratio");

		auto updateRate = cl_updaterate->GetFloat();
		auto interpRatio = cl_interp->GetFloat();
		auto minInterpRatio = sv_client_min_interp_ratio->GetFloat();
		auto maxInterpRatio = sv_client_max_interp_ratio->GetFloat();
		auto minUpdateRate = sv_minupdaterate->GetFloat();
		auto maxUpdateRate = sv_maxupdaterate->GetFloat();

		auto clampedUpdateRate = clamp(updateRate, minUpdateRate, maxUpdateRate);
		auto clampedInterpRatio = clamp(interpRatio, minInterpRatio, maxInterpRatio);

		auto lerp = clampedInterpRatio / clampedUpdateRate;

		if (lerp <= cl_interp->GetFloat())
			lerp = cl_interp->GetFloat();

		return lerp;
	}

	float RandomFloat(float a, float b) {
		float random = ((float)rand()) / (float)RAND_MAX;
		float diff = b - a;
		float r = random * diff;
		return a + r;
	}


	
	void TriggerBoat()
	{
		CInput::CUserCmd* pCmd = Hacks.CurrentCmd;
		CBaseEntity* pLocal = Hacks.LocalPlayer;
		CBaseCombatWeapon* pWeapon = Hacks.LocalWeapon;
			static size_t tDelay = 0;

		if ( pWeapon->IsKnife() )
			return;

		Ray_t ray;
		trace_t tr;

		Vector vStart, vEnd, vAngles;
		Vector vCurrentAngles;

		vStart = pLocal->GetEyePosition();

		Interfaces.pEngine->GetViewAngles(vCurrentAngles);

		g_Math.angleVectors(vCurrentAngles, vAngles);

		vAngles *= 8192.f;

		vEnd = vStart + vAngles;

		ray.Init(vStart, vEnd);

		CTraceFilter pTraceFilter;

		pTraceFilter.pSkip = Interfaces.pEntList->GetClientEntity(Interfaces.pEngine->GetLocalPlayer());

		Interfaces.pTrace->TraceRay(ray, MASK_NPCWORLDSTATIC | CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_MONSTER | CONTENTS_WINDOW | CONTENTS_DEBRIS | CONTENTS_HITBOX, &pTraceFilter, &tr);

		if (!Vars.Ragebot.LegitFriendlyFire)
		{
			if (tr.m_pEnt->GetTeam() == Hacks.LocalPlayer->GetTeam())
				return;
		}

		if (!tr.m_pEnt->isAlive())
			return;


		if (Vars.Ragebot.trigger_delay > 0)
		{
			if (GetTickCount() > tDelay)
			{
					pCmd->buttons |= IN_ATTACK;

				tDelay = GetTickCount() + Vars.Ragebot.trigger_delay * 35;
			}
		}
		else
		{
			if ((tr.hitgroup <= 7 && tr.hitgroup > 0))
				pCmd->buttons |= IN_ATTACK;
		}
	}


	bool LegitAim(float fov, int hitbox) {
		if (!(Vars.Ragebot.LegitTriggerBot || GetAsyncKeyState(VK_LBUTTON)) ) return false;
		std::vector<Vector> possibleaimspots;
		std::vector<CBaseEntity*> possibletargets;
		for (auto i = 0; i <= Interfaces.pEntList->GetHighestEntityIndex(); i++) {
			auto pEntity = static_cast<CBaseEntity*> (Interfaces.pEntList->GetClientEntity(i));
			if (pEntity == nullptr) continue;
			if (pEntity == Hacks.LocalPlayer) continue;
			if (!pEntity->isAlive()) continue;
			if (!(pEntity->GetHealth() > 0)) continue;
			if (!Vars.Ragebot.LegitFriendlyFire)
			{
				if (pEntity->GetTeam() == Hacks.LocalPlayer->GetTeam()) continue;
			}
			if (pEntity->IsDormant()) continue;
			//if(GetChockedPackets(pEntity) == 0) continue;
			if (pEntity->HasGunGameImmunity()) continue;
			auto flServerTime = (float)Hacks.LocalPlayer->GetTickBase() * Interfaces.pGlobalVars->interval_per_tick;
			auto flNextPrimaryAttack = Hacks.LocalWeapon->NextPrimaryAttack();
			if (!flNextPrimaryAttack > flServerTime) continue;



			player_info_t info;
			if (!(Interfaces.pEngine->GetPlayerInfo(pEntity->GetIndex(), &info))) continue;
			possibletargets.emplace_back(pEntity);
		}
		if ((int)possibletargets.size()) {
			for (auto pEntity : possibletargets) {
				Hitbox box;
				if (!box.GetHitbox(pEntity, hitbox)) continue;
				Vector Aimspot;
				if (GetBestPoint(Aimspot, box) != 0) {

					plist.Update();
					PlayerList::CPlayer* Player = plist.FindPlayer(pEntity);
					Player->entity = pEntity;
					if (Vars.Ragebot.Accuracy.PositionAdjustment)
						Hacks.CurrentCmd->tick_count = TIME_TO_TICKS(pEntity->GetSimulationTime() + LerpTime());
					possibleaimspots.emplace_back(Aimspot);
				}
			}
		}
		for (auto Spot : possibleaimspots) {


			Vector vecCurPos = Hacks.LocalPlayer->GetEyePosition();
			Vector angs;
			Misc::CalcAngle(vecCurPos, Spot, angs);
			Vector localangs;
			Interfaces.pEngine->GetViewAngles(localangs);
			if (Misc::FovTo(localangs, angs) <= fov) {
				if (!Vars.Ragebot.LegitSilent) Interfaces.pEngine->SetViewAngles(angs);
				//while (localangs != angs) {
					//SmoothAngleSet(localangs, angs);
					//break;
				//}
				//Hacks.CurrentCmd->buttons |= IN_ATTACK;
				if ( GetAsyncKeyState(VK_LBUTTON) )
				{
					Vector vecDelta = localangs - angs;
					Vector sdAimAng;
					if (Vars.Ragebot.LegitSmoothVariablation)
					{
						sdAimAng = localangs - Normalize(vecDelta) / RandomFloat(Vars.Ragebot.LegitSmooth, Vars.Ragebot.LegitSmooth + 5);
					}
					else {
						sdAimAng = localangs - Normalize(vecDelta) / Vars.Ragebot.LegitSmooth;
					}
					if (Vars.Ragebot.LegitRandomizeAmt != 0) {
						sdAimAng.x += RandomFloat(-Vars.Ragebot.LegitRandomizeAmt, Vars.Ragebot.LegitRandomizeAmt);
						sdAimAng.y += RandomFloat(-Vars.Ragebot.LegitRandomizeAmt, Vars.Ragebot.LegitRandomizeAmt);
					}
					if (Vars.Ragebot.LegitRCSAmt != 0) {
						float gayrcs = Vars.Ragebot.LegitRCSAmt;
						Hacks.CurrentCmd->viewangles -= LocalInfo.PunchAns * gayrcs;
					}
					if (!(Hacks.LocalPlayer->GetFlags() & FL_ONGROUND && Hacks.LocalPlayer->GetVecVelocity().Length2D() == 0)) { // makes it smoother while moving
						Vector sdAimAng = localangs - Normalize(vecDelta) / 100;
					}

					if (Vars.Ragebot.LegitAimDelay != 0)
					{
						DWORD tickCount = GetTickCount();
						if (tickCount > tickCount + 350 )
						{
							Interfaces.pEngine->SetViewAngles(sdAimAng);
							Hacks.CurrentCmd->buttons |= IN_ATTACK;				
						}
					}
					if (Vars.Ragebot.LegitAimDelay == 0)
					{
						Interfaces.pEngine->SetViewAngles(sdAimAng);
						Hacks.CurrentCmd->buttons |= IN_ATTACK;
					}
				}

				return true;
			}
		}
	}

	

	void GetSettings(float& fov, float& recoil, int& Hitbox) {
		fov = Vars.Ragebot.LegitFov;
			recoil = 0;
	}

public:
	void Run() {


			float fov = Vars.Ragebot.LegitFov;
			float recoil = 0;
			int hitbox = Vars.Ragebot.LegitHitbox;
			if (Vars.Ragebot.LegitTriggerBotEnb) {
				if (GetAsyncKeyState(Vars.Ragebot.LegitTriggerKey))
				{
					TriggerBoat();
				}
			}
				LegitAim(fov, hitbox);
	}
} LegitBot;

