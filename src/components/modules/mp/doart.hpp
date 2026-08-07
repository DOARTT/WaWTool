#pragma once

namespace components::mp
{
	class doart_module : public component
	{
	public:
		doart_module();
		~doart_module() = default;
		const char* get_name() override { return "doart_module_mp"; }

	private:
	};
}