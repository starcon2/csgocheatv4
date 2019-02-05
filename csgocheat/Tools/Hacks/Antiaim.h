#pragma once
#include "stdafx.h"
#include "../Menu/Menu.h"
#include "Misc.h"
#include "Aimbot.h"
#include "../Utils/LocalInfo.h"
#include "../Menu/SettingsManager.h"
#include <chrono>
#include "../../SDK/SDK Headers/EngineClient.h"
#include "../../Singleton.h"
#define TIME_TO_TICKS(dt) ((int)( 0.5f + (float)(dt) / Interfaces.pGlobalVars->interval_per_tick))

class CAntiAim : public Singleton<CAntiAim> {
private:

	static void CalcAngleOld(Vector src, Vector dst, Vector& angles)
	{
		Vector delta = src - dst;
		double hyp = delta.Length2D();
		angles.y = atan(delta.y / delta.x) * 57.295779513082f;
		angles.x = atan(delta.z / hyp) * 57.295779513082f;
		if (delta.x >= 0.0)
			angles.y += 180.0f;
		angles.z = 0;
	}



public:

	void SlideWalk()
	{
		auto cmd = Hacks.CurrentCmd;
		if (cmd->forwardmove > 0)
		{
			cmd->buttons |= IN_BACK;
			cmd->buttons &= ~IN_FORWARD;
		}

		if (cmd->forwardmove < 0)
		{
			cmd->buttons |= IN_FORWARD;
			cmd->buttons &= ~IN_BACK;
		}

		if (cmd->sidemove < 0)
		{
			cmd->buttons |= IN_MOVERIGHT;
			cmd->buttons &= ~IN_MOVELEFT;
		}

		if (cmd->sidemove > 0)
		{
			cmd->buttons |= IN_MOVELEFT;
			cmd->buttons &= ~IN_MOVERIGHT;
		}
	}


	inline float FastSqrt(float x) {

		unsigned int i = *(unsigned int*)&x;
		i += 127 << 23;
		i >>= 1;

		return *(float*)&i;
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

	int Ticks;
	void FakeLag()
	{
		bool canFakelag;
		

		if (Vars.Ragebot.Antiaim.Fakelag.Type == 0 || Hacks.LocalPlayer->GetVecVelocity().Length2D() < 10.f)
			Hacks.SendPacket = true;
			return;

		auto WishTicks = 14;

		bool ShouldChoke = false;

		if ((Hacks.LocalPlayer->GetFlags() & FL_ONGROUND && Hacks.LocalPlayer->GetVecVelocity().Length2D() == 0)) { // i think this turns it off when you arent moving or whatever
			canFakelag = false;
		}
		else {
			canFakelag = true;
		}

		if (canFakelag)
		{
			Ticks = 1;

			switch (Vars.Ragebot.Antiaim.Fakelag.Type) {
			case 0:			
				break;
			case 1: {
				if (Interfaces.Client_State->chokedcommands < WishTicks)
					ShouldChoke = true;
			}
					break;
			case 2: {//aimware v3 dump 
				auto VelocityY = Hacks.LocalPlayer->GetVecVelocity().y;
				auto VelocityX = Hacks.LocalPlayer->GetVecVelocity().x;
				auto WishTicks_1 = 0;
				auto AdaptTicks = 2;
				auto ExtrapolatedSpeed = sqrtf((VelocityX * VelocityX) + (VelocityY * VelocityY))
					* Interfaces.pGlobalVars->interval_per_tick;
				while ((WishTicks_1 * ExtrapolatedSpeed) <= 68.0) {
					if (((AdaptTicks - 1) * ExtrapolatedSpeed) > 68.0)
					{
						++WishTicks_1;
						break;
					}
					if ((AdaptTicks * ExtrapolatedSpeed) > 68.0)
					{
						WishTicks_1 += 2;
						break;
					}
					if (((AdaptTicks + 1) * ExtrapolatedSpeed) > 68.0)
					{
						WishTicks_1 += 3;
						break;
					}
					if (((AdaptTicks + 2) * ExtrapolatedSpeed) > 68.0)
					{
						WishTicks_1 += 4;
						break;
					}
					AdaptTicks += 5;
					WishTicks_1 += 5;
					if (AdaptTicks > 16)
						break;
				}
				Ticks = WishTicks_1;
			}
					break;
			}

			if (Ticks <= 15)
			{
				if (Interfaces.Client_State->chokedcommands < Ticks)
					Hacks.SendPacket = false;
				else
					Hacks.SendPacket = true;
			}
		}
	}


	// 		Interfaces.pEngine->ExecuteClientCmd(); easy pasta



	void fake_crouch() {
		static bool counter = false;
		if (GetAsyncKeyState(VK_MENU)) {
			if (Vars.Ragebot.FakeDuck) {
				static bool counter = false;
				static int counte = 0;
				if (counte == 3) {
					counte = 0;
					counter = !counter;
				}
				counte++;
				if (counter) {
					Hacks.CurrentCmd->buttons |= IN_DUCK;
					Hacks.SendPacket = true;
				}
				else {
					Hacks.CurrentCmd->buttons &= ~IN_DUCK;
					Hacks.SendPacket = false;
				}
			}
		}
	}





	void SlowWalk() {

		if (!(GetAsyncKeyState(VK_SHIFT))) {
			Vars.Ragebot.IsSlowWalking = false;
			return;
		}

		auto get_speed = 35;

		float min_speed = (float)(FastSqrt(square(Hacks.CurrentCmd->forwardmove) + square(Hacks.CurrentCmd->sidemove) + square(Hacks.CurrentCmd->upmove)));
		if (min_speed <= 0.f)
			return;

		if (Hacks.CurrentCmd->buttons & IN_DUCK)
			get_speed *= 2.94117647f;

		if (min_speed <= get_speed)
			return;

		float kys = get_speed / min_speed;
		Vars.Ragebot.IsSlowWalking = true;
		Hacks.CurrentCmd->forwardmove *= kys;
		Hacks.CurrentCmd->sidemove *= kys;
		Hacks.CurrentCmd->upmove *= kys;

	}


	void Run()
	{
		Vector View = Hacks.CurrentCmd->viewangles;
		if (!Vars.Ragebot.Antiaim.Enable)
			return;
		if (Hacks.CurrentCmd->buttons & IN_ATTACK)
			return;
		if (!Hacks.LocalPlayer)
			return;
		if (!Hacks.LocalPlayer->isAlive())
			return;
		fake_crouch();
		if (Vars.Ragebot.SlowwalkOn)
		{
			SlowWalk();
		}
		if (Vars.Ragebot.Slidewalk) {
			SlideWalk();
		}

		if (Hacks.LocalPlayer->GetMoveType() == MOVETYPE_LADDER)
			return;
		if (Hacks.LocalWeapon->IsGrenade())
			return;
		if (Vars.Ragebot.InfinityDuck) {
			Hacks.CurrentCmd->buttons |= IN_BULLRUSH; //infinity duck
		}
		switch (Vars.Ragebot.Antiaim.Pitch)
		{
		case 1: Hacks.CurrentCmd->viewangles.x = 89; break;
		case 2: Hacks.CurrentCmd->viewangles.x = 271; break;
		case 3: Hacks.CurrentCmd->viewangles.x = -271; break;
		case 4: Hacks.CurrentCmd->viewangles.x = 1080; break;
		case 5: Hacks.CurrentCmd->viewangles.x = Vars.Ragebot.CustomPitch;
		}

		static float RealAng = 0;


		switch (Vars.Ragebot.Antiaim.Yaw)
		{
		case 1: Hacks.CurrentCmd->viewangles.y += 180; break;
		case 2: Hacks.CurrentCmd->viewangles.y += 180 + g_Math.RandomFloat(25, -25); break;
		case 3: {

			static float asd = 120;
			asd += 5;
			if (asd > 240)
				asd = 120;

			Hacks.CurrentCmd->viewangles.y += asd;
		}
				break;
		}
		RealAng = Hacks.CurrentCmd->viewangles.y;
		static bool jitter = false;
		if (Vars.Ragebot.Antiaim.Desync == 3) { //zap
			if (Hacks.SendPacket)
			{
				Hacks.CurrentCmd->viewangles.y = RealAng += jitter ? 175 : 9;
			}
			// btw pasted
		}
		if (Vars.Ragebot.Antiaim.Desync == 2) { // razer
			if (Hacks.SendPacket)
			{
				Hacks.CurrentCmd->viewangles.y = RealAng += jitter ? -154 : -74; //old values -152 -20
			}
			// btw pasted
		}
		if (Vars.Ragebot.Antiaim.Desync == 1 ) //normal
		{
			if (Hacks.SendPacket)
			{
				float server_time = Hacks.LocalPlayer->GetTickBase() * Interfaces.pGlobalVars->interval_per_tick;
				float time = TIME_TO_TICKS(server_time);

				while (time >= server_time)
					time = 0.f;

				float idk = rand() % 100;

				jitter = !jitter;
				if (time >= server_time / 2)
				{
					if (idk < 70)
					{
						if (!jitter)
							Hacks.CurrentCmd->viewangles.y = RealAng + 55;

					}
					else
					{
						if (!jitter)
							Hacks.CurrentCmd->viewangles.y = RealAng - 55;

					}
				}
				else
				{
					if (idk < 70)
					{
						if (jitter)
							Hacks.CurrentCmd->viewangles.y = RealAng - Vars.Ragebot.JitterAmount;
					}
					else
					{
						if (jitter)
							Hacks.CurrentCmd->viewangles.y = RealAng + Vars.Ragebot.JitterAmount;

					}
				}
			}
		}
	}
};