#ifndef GAMMOU_ABSTRACT_SOUND_COMPONENT_H_
#define GAMMOU_ABSTRACT_SOUND_COMPONENT_H_

#include "gammou_process.h"
#include "sound_component_manager.h"
#include "../plugin_management/data_stream.h"

namespace Gammou {

	namespace Sound {
		
		// Todo verifier tout ca 
		constexpr double DEFAULT_SAMPLE_RATE = 44100.0;
		constexpr double DEFAULT_SAMPLE_DURATION = 1.0 / DEFAULT_SAMPLE_RATE;
		constexpr unsigned int NO_FACTORY = 0xFFFFFFFF;
		
		class abstract_sound_component : public Process::abstract_component<double>,
											public Process::observer<sound_component_manager, sound_component_notification_tag>  {

			friend class abstract_plugin_factory;

		public:
			abstract_sound_component(
				const std::string& name,
				const unsigned int input_count,
				const unsigned int output_count);

			virtual ~abstract_sound_component() {}

			unsigned int get_factory_id() const;
			virtual unsigned int save_state(data_sink& data) { return 0; };

			virtual unsigned int get_channel_count() const { return 1;  }
			virtual void set_current_working_channel(const unsigned int new_chanel) {};
			void set_sample_rate(const double sample_rate);

		protected:
			inline double get_sample_duration() const { return m_sample_duration; }
			inline double get_sample_rate() const { return m_sample_rate; }
			virtual void on_sample_rate_change(const double new_sample_rate) {};

		private:
			void on_notify(const sound_component_notification_tag notification_tag) override;

			double m_sample_rate;
			double m_sample_duration;
			unsigned int m_factory_id;
		};
		
		
	} /* Sound */
} /* Gammou */

#endif