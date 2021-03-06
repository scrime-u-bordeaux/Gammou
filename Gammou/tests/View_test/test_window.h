
#include <memory>
#include <view.h>
#include <variant>

namespace Gammou {

    
    class test_window : public View::window_widget {

        public:
            test_window(
                const unsigned int px_width, 
                const unsigned int py_height);

            bool on_mouse_dbl_click(const int x, const int y) override;
            bool on_key_down(const View::keycode key) override;

        private:
            View::storage_directory_model<std::string, int> m_model;
    };

	class test_dialog : public View::dialog {

		public:
			test_dialog();
			void draw(cairo_t *cr) override;

	};


} /* Gammou */


