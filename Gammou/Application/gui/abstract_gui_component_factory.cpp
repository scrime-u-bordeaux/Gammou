
#include "abstract_gui_component_factory.h"

namespace Gammou {

	namespace Gui {

		abstract_gui_component_factory::abstract_gui_component_factory(
			const std::string & name, 
			const std::string & category, 
			unsigned int factory_id)
			: abstract_plugin_factory(name, category, factory_id)
		{
		}
		
		void abstract_gui_component_factory::delete_sound_component(
			Sound::abstract_sound_component * component) const
		{
			if (component != nullptr)
				delete component;
		}
	} /* Gui */
} /* Gammou */