#include "std_include.hpp"

#define VERSION "0.1"

using namespace game::mp;

namespace components::mp
{
	void draw_custom_hud()
	{
		// pmove speed hud
		if (dvars::doart_hud && dvars::doart_hud->current.enabled)
		{
			const float velo_x = game::mp::cgs->activeSnapshots->ps.velocity[0];
			const float velo_z = game::mp::cgs->activeSnapshots->ps.velocity[1];
			const float speed = sqrt(velo_x * velo_x + velo_z * velo_z);

			game::mp::draw_text_with_engine(
				/* x	*/ dvars::doart_hud_x->current.value,
				/* y	*/ dvars::doart_hud_y->current.value,
				/* scaX */ dvars::doart_hud_fontScale->current.value,
				/* scaY */ dvars::doart_hud_fontScale->current.value,
				/* font */ fonts::get_font_for_style(dvars::doart_hud_fontStyle->current.integer),
				/* colr */ dvars::doart_hud_fontColor->current.vector,
				/* txt	*/ utils::va("X: %f\nY: %f\nZ: %f\nYAW: %f\nSPEED: %f",
					game::mp::cgs->activeSnapshots[0].ps.origin[0], 
					game::mp::cgs->activeSnapshots[0].ps.origin[1], 
					game::mp::cgs->activeSnapshots[0].ps.origin[2],
					game::mp::cgs->activeSnapshots[0].ps.viewangles[1],
					speed
				));
		}
	}

	// hk call to CG_Drawcrosshair in CG_Draw2D ~ only active if cg_draw2d
	__declspec(naked) void draw_custom_hud_stub()
	{
		const static uint32_t func_addr = 0x441900;
		const static uint32_t retn_addr = 0x43E4DD;
		__asm
		{
			call	func_addr;
			add		esp, 4;

			pushad;
			call	draw_custom_hud;
			popad;

			jmp		retn_addr;
		}
	}

	__declspec(naked) void killfeed_filter_stub()
	{
		const static uint32_t stock1_addr = 0x44B7AB;
		const static uint32_t stock2_addr = 0x44B755;
		const static uint32_t retn_addr = 0x44B7F3;
		
		__asm
		{
			push	eax;
			mov		eax, dvars::doart_killfeed_filter;
			cmp		byte ptr[eax + 0x10], 1;
			pop		eax;

			je		YUMP;

			cmp		[esp + 20], ecx;
			jnz		STOCK;

			jmp		stock2_addr;

		STOCK:
			jmp		stock1_addr;

		YUMP:
			jmp		retn_addr;
		}
	}

	__declspec(naked) void CG_DrawDisconnect_stub()
	{
		const static uint32_t dword_9E2AA0 = 0x9E2AA0;
		const static uint32_t stock_addr = 0x440005;
		const static uint32_t retn_addr = 0x44012C;

		__asm
		{
			push	eax;
			mov		eax, dvars::doart_lagIcon;
			cmp		byte ptr[eax + 0x10], 1;
			pop		eax;

			je		STOCK;

			jmp		retn_addr;

		STOCK:
			mov		ecx, dword_9E2AA0;
			cmp		eax, dword ptr[ecx];

			jg		loc_44012C;

			jmp		stock_addr;

		loc_44012C:
			jmp		retn_addr;
		}
	}

	doart_module::doart_module() {
		static auto version_str = std::string("DOART Tool v"s + VERSION + "> "s);

		// console string ;)
		utils::hook::set<const char*>(0x48AF32 + 1, version_str.c_str());

		// hook in cg_obituary for killfeed filter
		utils::hook(0x44B74F, killfeed_filter_stub, HOOK_JUMP).install()->quick();

		// hook in cg_drawdisconnect to hide connection interrupted icon with dvar
		utils::hook(0x43FFF9, CG_DrawDisconnect_stub, HOOK_JUMP).install()->quick();

		// hook call to cg_drawcrosshair in cg_draw2d
		utils::hook(0x43E4D5, draw_custom_hud_stub, HOOK_JUMP).install()->quick();

		// no forward/backslash for console cmds
		utils::hook::nop(0x493DEF, 5);

		// Remove Impure client (iwd) check - needed?
		utils::hook::nop(0x5B260F, 30);

		// don't play intro video - allows to devmap into a map via commandline
		utils::hook::nop(0x564CB9, 5);

		dvars::doart_hud = game::Dvar_RegisterBool(
			/* name		*/ "doart_hud",
			/* desc		*/ "Display current player's position.",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved);

		dvars::doart_hud_x = game::Dvar_RegisterFloat(
			/* name		*/ "doart_hud_x",
			/* default	*/ 20.0f,
			/* minVal	*/ -10000.0f,
			/* maxVal	*/ 10000.0f,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "Coordinate hud horizontal position");

		dvars::doart_hud_y = game::Dvar_RegisterFloat(
			/* name		*/ "doart_hud_y",
			/* default	*/ 360.0f,
			/* minVal	*/ -10000.0f,
			/* maxVal	*/ 10000.0f,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "Coordinate hud vertical position");

		dvars::doart_hud_fontScale = game::Dvar_RegisterFloat(
			/* name		*/ "doart_hud_fontScale",
			/* default	*/ 0.75f,
			/* minVal	*/ 0.1f,
			/* maxVal	*/ 10.0f,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "Coordinate hud font scale");

		dvars::doart_hud_fontStyle = game::Dvar_RegisterInt(
			/* name		*/ "doart_hud_fontStyle",
			/* default	*/ 3,
			/* minVal	*/ 0,
			/* maxVal	*/ 8,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "Coordinate hud font Style");

		dvars::doart_hud_fontColor = game::Dvar_RegisterVec4(
			/* name		*/ "doart_hud_fontColor",
			/* x		*/ 0.0f,
			/* y		*/ 1.0f,
			/* z		*/ 1.0f,
			/* w		*/ 1.0f,
			/* min		*/ 0.0f,
			/* max		*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "Coordinate hud font color");
		
		dvars::doart_killfeed_filter = game::Dvar_RegisterBool(
			/* name		*/ "doart_killfeed_filter",
			/* desc		*/ "Filter killfeed to show only your kills.",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved);

		dvars::doart_lagIcon = game::Dvar_RegisterBool(
			/* name		*/ "doart_lagIcon",
			/* desc		*/ "When set to 0, hides connection interrupted icon.",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved);
	}
}