#pragma once

#include <mutex>

#include "../../debug.h"
#include "view.h"
#include "gui_sound_component.h"
#include "gui_synthesizer_circuit.h"
#include "../../Synthesizer/plugin_management/main_factory.h"
#include "../../Synthesizer/builtin_components.h"

#define GAMMOU_SYNTHESIZER_CHANNEL_COUNT 128

namespace Gammou {

	namespace Gui {

		class synthesizer_gui : public View::generic_window {

		public:
			synthesizer_gui(
				Sound::synthesizer *synthesizer, std::mutex *synthesizer_mutex,
				unsigned int width, const unsigned int height);
			
			~synthesizer_gui();

		private:
			void add_plugin_factory(Sound::abstract_plugin_factory *factory);
			void init_main_factory(); // Load all built-in and plug in component

			View::list_box *m_plugin_list_box;

			std::vector<unsigned int> m_factory_ids;
			Sound::main_factory m_main_factory;
			unsigned int page_id;
		};
	}
}
