#ifndef GAMMOU_CONTROL_COMPONENT_IDS_H_
#define GAMMOU_CONTROL_COMPONENT_IDS_H_


namespace Gammou {

	namespace Gui {

		enum control_ids : unsigned int {
			knob_value_id		=	500u,
			knob_gain_id		=	501u,
			integer_value_id	=	502u,
			integer_gain_id		=	503u,
			slider_value_id		=	504u,
			slider_gain_id		=	505u
		};

        constexpr auto ControlCategory = "Control";
	}

}

#endif
