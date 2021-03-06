
#include <cstring>

#include <experimental/filesystem>
#include <filesystem>

#include "../../Synthesizer/builtin_components.h"
#include "../persistence/synthesizer_persistence.h"
#include "synthesizer_gui.h"
#include "gui_properties.h"

#include "user_component/user_component_editor.h"

// for test
#include "plugin_request_dialog.h"

namespace Gammou {

	namespace Gui {

		synthesizer_gui::synthesizer_gui(
			Sound::synthesizer * synthesizer, 
			std::mutex * synthesizer_mutex,
			AudioBackend::abstract_audio_backend& backend)
		:	View::window_widget(
				GuiProperties::main_gui_width, 
				GuiProperties::main_gui_height,
				View::cl_chartreuse), // for gui debuging
			m_synthesizer(*synthesizer),
			m_backend(backend)
		{
			DEBUG_PRINT("SYN GUI CTOR\n");

			//  Synthesizer Circuits

			auto master_circuit = 
				std::make_unique<gui_master_circuit>(
                    m_gui_component_factory, synthesizer, synthesizer_mutex,
					0, 0, GuiProperties::main_gui_circuit_width, GuiProperties::main_gui_circuit_height);
            m_gui_master_circuit = master_circuit.get();

			auto polyphonic_circuit = 
				std::make_unique<gui_polyphonic_circuit>(
                    m_gui_component_factory, synthesizer, synthesizer_mutex,
					0, 0, GuiProperties::main_gui_circuit_width, GuiProperties::main_gui_circuit_height);
            m_gui_polyphonic_circuit = polyphonic_circuit.get();

            //  User component editor
			/*
            auto component_editor =
                std::make_unique<user_component_editor>(
                    0, 0,
                    GuiProperties::user_component_editor_width,
                    GuiProperties::user_component_editor_circuit_height);

            component_editor->set_component_open_action(
            [this]()
            {
                m_pages->select_page(2);    //  go to editor page
            });

            m_user_component_editor = component_editor.get();
			*/
            //  Pages

			auto pages =
				std::make_unique<View::page_container>(
                    GuiProperties::main_gui_component_choice_box_width,
                    GuiProperties::main_gui_toolbox_height,
                    GuiProperties::main_gui_circuit_width,
                    GuiProperties::main_gui_circuit_height, View::cl_chartreuse);

			pages->add_page(std::move(master_circuit));
			pages->add_page(std::move(polyphonic_circuit));
            //pages->add_page(std::move(component_editor));

            pages->select_page(0);
            m_pages = pages.get();
			add_widget(std::move(pages));

			// Component Choice DirectoryView

            auto selector =
                std::make_unique<component_selector>(
					0, GuiProperties::main_gui_toolbox_height - 2, 
					GuiProperties::main_gui_component_choice_box_width, 
					GuiProperties::main_gui_component_choice_box_height + 2, 
                    GuiProperties::main_gui_component_choice_box_item_count,
                    m_gui_component_factory);

            selector->set_value_select_event(
            [this](View::directory_view<unsigned int>&, const std::string&, const unsigned int& id)
            {
                m_gui_master_circuit->select_component_creation_factory_id(id);
                m_gui_polyphonic_circuit->select_component_creation_factory_id(id);
                //m_user_component_editor->select_component_creation_factory_id(id);
            });

            m_component_selector = selector.get();
            add_widget(std::move(selector));
						
			//// ToolBox

			auto tool_box =
				std::make_unique<View::panel<> >(
					0, 0,
					GuiProperties::main_gui_toolbox_width, 
					GuiProperties::main_gui_toolbox_height,
					GuiProperties::main_gui_tool_box_background);

			//	Circuit Selector

			auto circuit_selector = 
				std::make_unique<View::list_box>(
					GuiProperties::main_gui_size_unit * 2, 0, 
					GuiProperties::main_gui_size_unit * 3, GuiProperties::main_gui_size_unit, 
					2,
					GuiProperties::main_gui_list_box_selected_item_color, 
					GuiProperties::main_gui_list_box_hovered_item_color,
					GuiProperties::main_gui_list_box_background, 
					GuiProperties::main_gui_list_box_font_color, 
					GuiProperties::main_gui_component_choice_box_font_size);

			circuit_selector->add_item("Master Circuit");
			circuit_selector->add_item("Polyphonic Circuit");
			circuit_selector->select_item(0);

			circuit_selector->set_item_select_event(
                [this](View::list_box&, unsigned int id)
                {
                    m_pages->select_page(id);
                    //  make sure that a component cannot be destroyed while editing
                    //m_user_component_editor->close_user_component();
                });

			tool_box->add_widget(
				std::move(circuit_selector));
			
			//	Legato - Polyphonic selector

			auto keyboard_mode_selector = 
				std::make_unique<View::list_box>(
					GuiProperties::main_gui_size_unit * 5, 0, 
					GuiProperties::main_gui_size_unit * 3, GuiProperties::main_gui_size_unit, 
					2,
					GuiProperties::main_gui_list_box_selected_item_color, 
					GuiProperties::main_gui_list_box_hovered_item_color,
					GuiProperties::main_gui_list_box_background, 
					GuiProperties::main_gui_list_box_font_color, 
					GuiProperties::main_gui_component_choice_box_font_size);

			keyboard_mode_selector->add_item("Polyphonic Keyboard");
			keyboard_mode_selector->add_item("Legato Keyboard");
			keyboard_mode_selector->select_item(0);

			keyboard_mode_selector->set_item_select_event(
                [synthesizer](View::list_box&, unsigned int id)
                {
					using mode = Sound::synthesizer::keyboard_mode;

                    if (id == 0)
						synthesizer->set_keyboard_mode(mode::POLYPHONIC);
					else
						synthesizer->set_keyboard_mode(mode::LEGATO);
                });

			tool_box->add_widget(
				std::move(keyboard_mode_selector));

			//	LOad/Sazve preset buttons
			
			auto load_preset_button =
				std::make_unique<View::push_button>(
					[this](View::push_button* self)
					{
						using mode = View::file_explorer_dialog::mode;
					
						self->set_enabled(false);

						View::file_explorer_dialog dialog{GAMMOU_PRESETS_DIRECTORY_PATH, mode::OPEN};
						std::string path;

						dialog.show("Load Preset");

						if (dialog.get_filename(path)) {
							Persistence::gammou_state state;

							try {
								Persistence::file_input_stream stream{ path };
								Persistence::gammou_file<Persistence::gammou_state>::load(stream, state);
								load_state(state);
							}catch(...){}
						}

						self->set_enabled(true);
					},
					"Load Preset",
					GuiProperties::main_gui_size_unit * 9, 15, 95, 27,
					10,	// font size
					GuiProperties::main_gui_list_box_hovered_item_color,
					GuiProperties::main_gui_list_box_selected_item_color,
					GuiProperties::component_font_color);

			auto save_preset_button =
				std::make_unique<View::push_button>(
					[this](View::push_button* self)
					{
						using mode = View::file_explorer_dialog::mode;
						self->set_enabled(false);

						View::file_explorer_dialog dialog{ GAMMOU_PRESETS_DIRECTORY_PATH, mode::SAVE };
						std::string path;

						dialog.show("Save Preset");

						if (dialog.get_filename(path)) {
							Persistence::gammou_state state;

							try {
								Persistence::file_output_stream stream{ path };
								save_state(state);
								Persistence::gammou_file<Persistence::gammou_state>::save(stream, state);
							}catch(...){}
						}

						self->set_enabled(true);
					},
					"Save Preset",
					GuiProperties::main_gui_size_unit * 11, 15, 95, 27,
					10,	// font size
					GuiProperties::main_gui_list_box_hovered_item_color,
					GuiProperties::main_gui_list_box_selected_item_color,
					GuiProperties::component_font_color);

			tool_box->add_widget(std::move(load_preset_button));
			tool_box->add_widget(std::move(save_preset_button));

			//	Master Volume Knob
			//	TODO : mieux
			const unsigned int offset = (GuiProperties::main_gui_size_unit - 50) / 2;

			auto master_volume = 
				std::make_unique<View::knob>(
					[synthesizer](View::knob *kn) 
					{ 
                        const float norm = kn->get_normalized_value();
                        const double volume = (expf(5.0f * norm) - 1.0) / (expf(5.0f) - 1.0);
						synthesizer->set_master_volume(volume); 
					},
					offset + GuiProperties::main_gui_width - GuiProperties::main_gui_size_unit,
					offset
				);

			GuiProperties::apply(*master_volume);
			m_master_volume = master_volume.get();
			
			master_volume->set_normalized_value(1.0); // coherence with synthesizer initial value
			tool_box->add_widget(std::move(master_volume));

			//--
			add_widget(std::move(tool_box));


			init_main_factory();
			DEBUG_PRINT("SYN GUI CTOR finnished\n");
		}

		synthesizer_gui::~synthesizer_gui()
		{
			DEBUG_PRINT("Syn Gui DTOR\n");

            //  Force user gui_component deletion here in order to
            //  remove loaded sound_component to be deleted before their factory
            m_gui_master_circuit->reset_content();
            m_gui_polyphonic_circuit->reset_content();
        }

        void synthesizer_gui::save_state(Persistence::gammou_state& state)
		{
            const unsigned int parameter_count =
                    m_synthesizer.get_parameter_input_count();

            state.parameters.resize(parameter_count);

			// Save Parameters
            for (unsigned int i = 0; i < parameter_count; ++i)
                state.parameters[i] = m_synthesizer.get_parameter_value(i);

			// Save Master Volume
            state.master_volume =
                m_master_volume->get_normalized_value();

			// Save Master Circuit
            m_gui_master_circuit->save_state(state.master_circuit);

			// Save Polyphonic Circuit
            m_gui_polyphonic_circuit->save_state(state.polyphonic_circuit);
		}

        void synthesizer_gui::load_state(const Persistence::gammou_state& state)
        {
            const unsigned int state_parameter_count =
                    static_cast<unsigned int>(state.parameters.size());
            const unsigned int syn_parameter_count =
                    m_synthesizer.get_parameter_input_count();

            //  Load parameters
            for (unsigned int i = 0; i < syn_parameter_count; ++i) {
                double param_value = 0.0;

                if (i < state_parameter_count)
                   param_value = state.parameters[i];

                m_synthesizer.set_parameter_value(param_value, i);
				m_backend.set_parameter_value(i, param_value);
			}

			// Load Master Volume
            m_master_volume->set_normalized_value(
                static_cast<float>(state.master_volume));
			
			// Load Master Circuit 
            m_gui_master_circuit->load_state(state.master_circuit);

			// Load Polyphonic Circuit
            m_gui_polyphonic_circuit->load_state(state.polyphonic_circuit);

			DEBUG_PRINT("SYN STATE LOADED\n");
		}

		void synthesizer_gui::init_main_factory()
		{
			// Built In Components
            m_component_selector->add_plugin_factory(MAKE_BUILTIN_FACTORY(Sound::sin_component));
            m_component_selector->add_plugin_factory(MAKE_BUILTIN_FACTORY(Sound::sum_component));
            m_component_selector->add_plugin_factory(MAKE_BUILTIN_FACTORY(Sound::product_component));
			m_component_selector->add_plugin_factory(MAKE_BUILTIN_FACTORY(Sound::negate_component));
			m_component_selector->add_plugin_factory(MAKE_BUILTIN_FACTORY(Sound::invert_component));
            m_component_selector->add_plugin_factory(MAKE_BUILTIN_FACTORY(Sound::lp2));
			m_component_selector->add_plugin_factory(MAKE_BUILTIN_FACTORY(Sound::bp2));
			m_component_selector->add_plugin_factory(MAKE_BUILTIN_FACTORY(Sound::hp2));
			m_component_selector->add_plugin_factory(MAKE_BUILTIN_FACTORY(Sound::adsr_env));
            m_component_selector->add_plugin_factory(MAKE_BUILTIN_FACTORY(Sound::saw));

            m_component_selector->add_plugin_factory(MAKE_UNIQUE_FUNCTION_COMPONENT_FACTORY(cos));
            m_component_selector->add_plugin_factory(MAKE_UNIQUE_FUNCTION_COMPONENT_FACTORY(exp));
            m_component_selector->add_plugin_factory(MAKE_UNIQUE_FUNCTION_COMPONENT_FACTORY(log));
            m_component_selector->add_plugin_factory(MAKE_UNIQUE_FUNCTION_COMPONENT_FACTORY(cosh));
            m_component_selector->add_plugin_factory(MAKE_UNIQUE_FUNCTION_COMPONENT_FACTORY(sinh));
            m_component_selector->add_plugin_factory(MAKE_UNIQUE_FUNCTION_COMPONENT_FACTORY(sqrt));
            m_component_selector->add_plugin_factory(MAKE_UNIQUE_FUNCTION_COMPONENT_FACTORY(fabs));
            m_component_selector->add_plugin_factory(MAKE_UNIQUE_FUNCTION_COMPONENT_FACTORY(atan));
            m_component_selector->add_plugin_factory(MAKE_UNIQUE_FUNCTION_COMPONENT_FACTORY(tanh));

			// Control Components
            m_component_selector->add_control_factory(std::make_unique<value_knob_gui_component_factory>());
            m_component_selector->add_control_factory(std::make_unique<gain_knob_gui_component_factory>());

            m_component_selector->add_control_factory(std::make_unique<value_slider_gui_component_factory>());
            m_component_selector->add_control_factory(std::make_unique<gain_slider_gui_component_factory>());

            m_component_selector->add_control_factory(std::make_unique<value_integer_gui_component_factory>());
            m_component_selector->add_control_factory(std::make_unique<gain_integer_gui_component_factory>());

            //m_component_selector->add_control_factory(
            //    std::make_unique<user_gui_component_factory>
            //        (*m_user_component_editor, m_gui_component_factory));

			// Plugins Components
			const std::string plugin_dir_path(GAMMOU_PLUGINS_DIRECTORY_PATH);
			
			try {
				for (auto & p : std::experimental::filesystem::directory_iterator(plugin_dir_path)) {
					try {
						const std::string plugin_path(p.path().string());
						DEBUG_PRINT("Loading %s\n", plugin_path.c_str());
                        m_component_selector->load_plugin_factory(plugin_path);
					}
					catch(std::exception& e){
						DEBUG_PRINT("Failed : %s\n", e.what());
					}

				}
			}
			catch(std::exception& e){
                DEBUG_PRINT(
					"Error while listing plugin in directory '%s' : %s\n", 
					GAMMOU_PLUGINS_DIRECTORY_PATH, e.what());
			}
		}

	} /* Gui */

} /* Gammou */
