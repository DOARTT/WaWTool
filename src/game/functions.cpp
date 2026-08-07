#include "std_include.hpp"

namespace game
{
	bool is_sp = false;
	bool is_mp = false;

	std::vector<std::string> loaded_modules;
	HWND main_window = nullptr;

	namespace mp
	{
		game::mp::cg_s* cgs = reinterpret_cast<game::mp::cg_s*>(0x98FCE0);
		game::mp::gentity_s* g_entities = reinterpret_cast<game::mp::gentity_s*>(0x1F8E300);

		DxGlobals* dx = reinterpret_cast<DxGlobals*>(0x1087DD04);
		clipMap_t* cm = reinterpret_cast<clipMap_t*>(0x2223A80);

		r_global_permanent_t* rgp = reinterpret_cast<r_global_permanent_t*>(0x1087BA80);
		game::GfxBuffers* gfx_buf = reinterpret_cast<game::GfxBuffers*>(0x10F3A7C8);

		CmdArgs* cmd_args = reinterpret_cast<CmdArgs*>(0x22236F8);
		cmd_function_s** cmd_ptr = reinterpret_cast<cmd_function_s**>(0x222377C);

		Cmd_ExecuteSingleCommand_t	Cmd_ExecuteSingleCommand = Cmd_ExecuteSingleCommand_t(0x55CD90);

		Com_PrintMessage_t Com_PrintMessage = Com_PrintMessage_t(0x5622B0);
		Com_Error_t Com_Error = Com_Error_t(0x562DA0);
		DB_EnumXAssets_FastFile_t DB_EnumXAssets_FastFile = DB_EnumXAssets_FastFile_t(0x4B9AE0);
		DB_LoadXAssets_t DB_LoadXAssets = DB_LoadXAssets_t(0x4BAD70);

		scr_const_t* scr_const = reinterpret_cast<scr_const_t*>(0x221A910);

		void Cbuf_AddText(const char* text /*eax*/)
		{
			const static uint32_t Cbuf_AddText_func = 0x55C130;
			__asm
			{
				mov		ecx, 0;
				mov		eax, text;
				call	Cbuf_AddText_func;
			}
		}

		void draw_text_with_engine(float x, float y, float scale_x, float scale_y, const char* font, const float* color, const char* text)
		{
			void* fontHandle = R_RegisterFont(font);
			R_AddCmdDrawTextASM(text, 0x7FFFFFFF, fontHandle, x, y, scale_x, scale_y, 0.0f, color, 0);
		}

		void R_AddCmdDrawTextASM(const char* text, int max_chars, void* font, float x, float y, float x_scale, float y_scale, float rotation, const float* color, int style)
		{
			const static uint32_t R_AddCmdDrawText_func = 0x6BDDE0;
			__asm
			{
				push	style;
				sub     esp, 0x14;

				fld		rotation;
				fstp[esp + 0x10];

				fld		y_scale;
				fstp[esp + 0x0C];

				fld		x_scale;
				fstp[esp + 0x8];

				fld		y;
				fstp[esp + 0x4];

				fld		x;
				fstp[esp];

				push	font;
				push	max_chars;
				push	text;
				mov		ecx, [color];

				call	R_AddCmdDrawText_func;
				add		esp, 0x24;
			}
		}

		void Vec2UnpackTexCoords(unsigned int packed, float* texcoord_out /*ecx*/)
		{
			const static uint32_t func_addr = 0x5BB880;
			__asm
			{
				mov		ecx, texcoord_out;
				push	packed;
				call	func_addr;
				add		esp, 4;
			}
		}

		bool DB_FileExists(const char* file_name, game::DB_FILE_EXISTS_PATH src)
		{
			const static uint32_t DB_FileExists_func = 0x4BC460;
			__asm
			{
				push	src;
				mov		eax, file_name;

				call	DB_FileExists_func;
				add     esp, 4;
			}
		}

		// db
		int** DB_XAssetPool = reinterpret_cast<int**>(0x8D0958);
		unsigned int* g_poolSize = reinterpret_cast<unsigned int*>(0x8D06E8);
		DB_GetXAssetSizeHandler_t* DB_GetXAssetSizeHandler = reinterpret_cast<DB_GetXAssetSizeHandler_t*>(0x8D0D48);

		int DB_GetXAssetTypeSize(int type)
		{
			return DB_GetXAssetSizeHandler[type]();
		}

		void* DB_ReallocXAssetPool(int type, unsigned int size)
		{
			const auto s = DB_GetXAssetTypeSize(type);
			const auto pool_entry = utils::memory::allocate(size * s);

			DB_XAssetPool[type] = static_cast<int*>(pool_entry);
			g_poolSize[type] = size;

			return pool_entry;
		}
	}

	//IDirect3DDevice9* get_device()
	//{
	//	return game::mp::dx->device;
	//}

	//GfxCmdBufSourceState* get_cmdbufsourcestate()
	//{
	//	const auto out = reinterpret_cast<GfxCmdBufSourceState*>(*game::mp::gfxCmdBufSourceState_ptr);
	//	return out;
	//}

	//GfxBackEndData* get_frontenddata()
	//{
	//	const auto out = reinterpret_cast<GfxBackEndData*>(*mp::frontEndDataOut_ptr);
	//	return out;
	//}

	//GfxBackEndData* get_backenddata()
	//{
	//	const auto out = reinterpret_cast<GfxBackEndData*>(*mp::backEndDataOut_ptr);
	//	return out;
	//}

	void Cmd_AddCommand(const char* name, void(*callback)(), cmd_function_s* data, char)
	{
		data->name = name;
		data->function = callback;
		data->next = *mp::cmd_ptr;

		*game::mp::cmd_ptr = data;
	}

	utils::function<dvar_s* (const char* dvarName, int type, int flags, DvarValue value, DvarLimits domain, const char* description)> Dvar_RegisterVariant;

	dvar_s* Dvar_RegisterInt(const char* name, int value, int min, int max, int flags, const char* description)
	{
		game::DvarValue val;
		val.integer = value;

		game::DvarLimits lim;
		lim.integer.min = min;
		lim.integer.max = max;

		return game::Dvar_RegisterVariant(name, game::dvar_type::integer, flags, val, lim, description);
	}

	dvar_s* Dvar_RegisterFloat(const char* name, float value, float min, float max, dvar_flags flags, const char* description)
	{
		game::DvarValue val = {};
		val.value = value;

		game::DvarLimits lim = {};
		lim.value.min = min;
		lim.value.max = max;

		return game::Dvar_RegisterVariant(name, game::dvar_type::value, flags, val, lim, description);
	}

	dvar_s* Dvar_RegisterVec3(const char* name, float x, float y, float z, float min, float max, dvar_flags flags, const char* description)
	{
		game::DvarValue val = {};
		val.vector[0] = x;
		val.vector[1] = y;
		val.vector[2] = z;

		game::DvarLimits lim = {};
		lim.vector.min = min;
		lim.vector.max = max;

		return game::Dvar_RegisterVariant(name, game::dvar_type::vec3, flags, val, lim, description);
	}

	dvar_s* Dvar_RegisterVec4(const char* name, float x, float y, float z, float w, float min, float max, dvar_flags flags, const char* description)
	{
		game::DvarValue val = {};
		val.vector[0] = x;
		val.vector[1] = y;
		val.vector[2] = z;
		val.vector[3] = w;

		game::DvarLimits lim = {};
		lim.vector.min = min;
		lim.vector.max = max;

		return game::Dvar_RegisterVariant(name, game::dvar_type::vec4, flags, val, lim, description);
	}

	dvar_s* Dvar_RegisterEnum(const char* dvar_name, const char** values, std::uint32_t value_count, int default_value, int flags, const char* description)
	{
		game::DvarValue val = {};
		val.integer = default_value;

		game::DvarLimits lim = {};
		lim.enumeration.stringCount = static_cast<int>(value_count);
		lim.enumeration.strings = values;

		return game::Dvar_RegisterVariant(dvar_name, game::enumeration, flags, val, lim, description);
	}

	dvar_s* Dvar_RegisterBool(const char* name, const char* description, bool value, int flags)
	{
		game::DvarValue val = {};
		val.enabled = value;

		game::DvarLimits lim = {};
		lim.integer.min = 0;
		lim.integer.max = 1;

		return game::Dvar_RegisterVariant(name, game::boolean, flags, val, lim, description);
	}

	XAssetHeader DB_FindXAssetHeader(XAssetType type, const char* name)
	{
		typedef XAssetHeader(*DB_FindXAssetHeader_t)(XAssetType, const char*, int, int);
		if (game::is_sp)
		{
			auto sp_func = reinterpret_cast<DB_FindXAssetHeader_t>(0x48DA30);
			return sp_func(type, name, 1, -1);
		}
		else
		{
			return (XAssetHeader) nullptr;
		}
	}

	void init_offsets()
	{
		Dvar_RegisterVariant = 0x5C5100;// 0x5EED90 sp
	}
}
