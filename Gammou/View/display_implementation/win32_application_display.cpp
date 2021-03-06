
#include "win32_application_display.h"

namespace Gammou {

	namespace View {

		win32_application_display::win32_application_display(
			View::widget & root_widget)
		:	abstract_display(root_widget),
			abstract_win32_display(root_widget),
			abstract_application_display(root_widget)
		{
		}

		win32_application_display::win32_application_display(
			abstract_display& parent, View::widget & root_widget)
		:
			abstract_display(root_widget),
			abstract_win32_display(parent, root_widget),
			abstract_application_display(root_widget)
		{
		}

		win32_application_display::~win32_application_display()
		{
			DEBUG_PRINT("Win32 App Display DTOR\n");
			close();
		}

		void win32_application_display::open(
			const std::string & title)
		{
			m_is_open = true;
			m_window_manager = 
				std::thread(window_manager, this);
		}

		void win32_application_display::wait()
		{
			if (m_window_manager.joinable())
				m_window_manager.join();
		}

		void win32_application_display::close()
		{
			m_is_open = false;

			if (std::this_thread::get_id() != 
                	m_window_manager.get_id())
                wait();
			//	windows is destroyed by the manager thread
		}

		void win32_application_display::window_manager(
			win32_application_display *self)
		{
			DEBUG_PRINT("Entering Window  Manager Thread\n");
			//	Create Window
			// TODO handle parent and title
			DEBUG_PRINT("Create Window ..\n");
			self->create_window(nullptr, "");

			HWND handle = self->get_window_handle();
			SetFocus(handle);

			//	Event loop			
			while (self->is_open()) {
				MSG msg;

				if (GetMessage(&msg, handle, 0, 0) > 0) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				else {
					DEBUG_PRINT("WM_QUIT received\n");
					break;
				}
			}
			
			DEBUG_PRINT("Destroying Window\n");
			self->destroy_window();
			DEBUG_PRINT("Quiting Window Manager\n");
		}

	}	/* View */

} /* Gammou */