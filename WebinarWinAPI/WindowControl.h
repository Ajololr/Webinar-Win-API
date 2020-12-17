#pragma once
#include <string>
#include <windows.h>
#include <vector>

namespace webinar
{
	class WindowControl
	{
	public:

		typedef bool(*Action)(WPARAM wParam, LPARAM lParam);

		struct Event
		{
			Action act;

			UINT uMsg;
		};

	protected:
		HWND _hwndWindow;
		int _iID;
		std::string _strWindowName;
		std::vector<Event> events;

		WindowControl()
		{
			
		}

		virtual ~WindowControl()
		{
			
		}

	public:

		inline HWND GetHandler() const
		{
			return _hwndWindow;
		}

		inline int GatID() const
		{
			return _iID;
		}

		inline std::string GetName() const
		{
			return _strWindowName;
		}

		inline void SetEvent(Action action, UINT message)
		{
			events.push_back(Event{ action, message });
		}

		inline void EventStart(UINT message, WPARAM wParam, LPARAM lParam)
		{
			for(size_t i = 0; i < events.size(); i++)
			{
				if (events[i].uMsg == message)
					events[i].act(wParam, lParam);
			}
		}
	};
}
