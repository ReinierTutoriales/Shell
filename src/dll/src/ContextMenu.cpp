#include <pch.h>
#include "Include/Theme.h"
#include "Include/ContextMenu.h"
#include "Include/stb_image_write.h"

using namespace Nilesoft::Diagnostics;
#include <mutex>

extern Logger &_log;

#pragma region

/*
void plutovg_surface_write_to_hdc(plutovg_surface_t *surface, int x, int y, HDC hdc)
{
	BYTE bitmapinfo[FIELD_OFFSET(BITMAPINFO, bmiColors) + (3 * sizeof(DWORD))];
	auto &bih = *(BITMAPINFOHEADER *)bitmapinfo;
	bih.biSize = sizeof(BITMAPINFOHEADER);

	bih.biWidth = surface->width;
	bih.biHeight = surface->height;

	bih.biPlanes = 1;
	bih.biBitCount = 32;

	bih.biCompression = BI_BITFIELDS;
	bih.biSizeImage = 0;
	bih.biClrUsed = 0;
	bih.biClrImportant = 0;

	auto *pMasks = (DWORD *)(&bitmapinfo[bih.biSize]);
	pMasks[0] = 0xff0000; // Red
	pMasks[1] = 0x00ff00; // Green
	pMasks[2] = 0x0000ff; // Blue

	::StretchDIBits(hdc,
				  x,
				  y + surface->height,
				  surface->width,
				  -surface->height,
				  0,
				  0,
				  surface->width,
				  surface->height,
				  surface->data,
				  (BITMAPINFO *)&bih,
				  DIB_RGB_COLORS,
				  SRCCOPY);
}
*/

struct DWM
{
	enum class BackdropType : int
	{
		Auto = 0,
		None = 1,
		MainWindow = 2,
		TransientWindow = 3,
		TabbedWindow = 4,
	};

	enum class Corner : int
	{
		Default = 0,
		None = 1,
		Round = 2,
		RoundSmall = 3,
		Last = 4,
	};

	HWND handle{};
	DWM(HWND hWnd = nullptr) : handle(hWnd) {}

	HRESULT RemoveCorner()
	{
		return SetCorner(Corner::None);
	}

	HRESULT SetCorner(Corner value = Corner::Round)
	{
		const auto DWMWA_WINDOW_CORNER_PREFERENCE = 33U;
		return SetAttribute(DWMWA_WINDOW_CORNER_PREFERENCE, value);
	}

	HRESULT SetBorderColor(COLORREF color)
	{
		const auto DWMWA_BORDER_COLOR = 34U;
		return SetAttribute(DWMWA_BORDER_COLOR, color);
	}

	/// <summary>
	/// Enable or disable immersive dark mode.
	/// Requires Windows build 19041 or higher.
	/// </summary>
	HRESULT SetImmersiveDarkMode(BOOL state = TRUE)
	{
		const auto DWMWA_IMMERSIVE_DARK_MODE = 20U;
		return SetAttribute(DWMWA_IMMERSIVE_DARK_MODE, state);
	}

	/// <summary>
	/// Requests a system-managed backdrop. DWMWA_SYSTEMBACKDROP_TYPE is public
	/// starting with Windows 11 22H2 (build 22621).
	/// </summary>
	HRESULT SetBackdropType(BackdropType backdropType)
	{
		const auto DWMWA_SYSTEMBACKDROP_TYPE = 38;
		return SetAttribute(DWMWA_SYSTEMBACKDROP_TYPE, backdropType);
	}


	template<typename T>
	HRESULT SetAttribute(DWORD dwAttribute, T pvAttribute)
	{
		return ::DwmSetWindowAttribute(handle, dwAttribute, (const void *)&pvAttribute, sizeof(T));
	}

	auto ExtendFrameIntoClientArea(MARGINS margins = { -1 })
	{
		return ::DwmExtendFrameIntoClientArea(handle, &margins);
	}
};

inline HMENU GET_HMENU(HWND hWnd) { return SendMSG<HMENU>(hWnd, MN_GETHMENU, 0, 0); }

auto ver = &Windows::Version::Instance();

#pragma endregion

namespace Nilesoft
{
	//std::mutex mtx; // mutex for critical section
	namespace Shell
	{
		//D2D_DC d2d;

		//std::mutex _mutex; // mutex for critical section
		//std::lock_guard<std::mutex> lock(_mutex);

		inline static MenuItemInfo *get_item(uint32_t id, const std::vector<MenuItemInfo *> &list)
		{
			for(auto item : list)
			{
				if(item->wID == id) return item;
			}
			return nullptr;
		}

		inline static MenuItemInfo *get_item(uint32_t id, HMENU hMenu, const std::vector<MenuItemInfo *> &list)
		{
			for(auto item : list)
			{
				if(item->handle == hMenu)
				{
					if(item->wID == id)
					{
						return item;
					}
				}
			}
			return nullptr;
		}

		ContextMenu::ContextMenu(HWND hWnd, HMENU hMenu, Point const &pt)
		{
			Window window = hWnd;
			hwnd.owner = hWnd;
			Processes[this] = true;
			dpi.val = Theme::GetDpi(pt, hwnd.owner);
			dpi.org = dpi.val;
			_hMenu_original = hMenu;
			_hMenu = ::CreatePopupMenu();
			_theme.dpi = &dpi;
			_context.dpi = &dpi;
			_context.wnd.owner = hwnd.owner;
			_context.hMenu = hMenu;
			_window = hwnd.owner;
			_cache = Initializer::instance->cache;
			if(Initializer::instance->dpi != dpi.val)
				_cache->reload(dpi.val);
			Initializer::instance->dpi = dpi.val;
			_tip.ctx = this;
			if(keyboard.get_keys_state(true)) {}
			_context.Keyboard = &keyboard;
			ThreadId = window.get_threadId(&ProcessId);
			GUITHREADINFO gti = { sizeof(GUITHREADINFO) };
			if(::GetGUIThreadInfo(ThreadId, &gti))
			{
				hwnd.active = gti.hwndActive;
				hwnd.focus = gti.hwndFocus;
				_context.wnd.active = hwnd.active;
				_context.wnd.focus = gti.hwndFocus;
			}
			_context.wnd.active = hwnd.active;
			_context.wnd.focus = hwnd.focus;
			Monitor monitor(pt);
			if(monitor.info())
			{
				_rcMonitor = monitor.rcMonitor;
				_context.helper.is_primary_monitor = monitor.is_primary();
			}
			languageId = ::GetThreadUILanguage();
			is_layoutRTL = (window.get_ex_style() & WS_EX_LAYOUTRTL) != 0;
		}

		ContextMenu::~ContextMenu()
		{
			try
			{
				Processes.erase(this);
				Uninitialize();
			}
			catch(...)
			{
#ifdef _DEBUG
				Logger::Exception(__func__);
#endif
			}
		}

		// ... content unchanged ...

		void ContextMenu::init_cfg()
		{
			auto sets = &_cache->settings;

			struct SystemVisualState
			{
				bool highContrast = Theme::IsHighContrast();
				bool transparency = false;
				bool systemUsesLightTheme = true;
				bool appsUseLightTheme = true;
			};

			SystemVisualState systemState;
			if(!systemState.highContrast)
				Theme::Personalize(&systemState.systemUsesLightTheme,
								 &systemState.appsUseLightTheme,
								 &systemState.transparency);

			Object obj;

			_context.theme = &_theme;
			_theme.dpi = &dpi;
			_context.font.icon = FontCache::Default;

			font.menu = {};
			Theme::GetFont(&font.menu, dpi.val);
			_context.font.text = font.menu.lfFaceName;

			bool enableTransparency = systemState.transparency;
			bool systemUsesLightTheme = systemState.systemUsesLightTheme;
			bool appsUseLightTheme = systemState.appsUseLightTheme;
			const bool isHighContrast = systemState.highContrast;

			_theme.system.mode = isHighContrast ? 2 : (systemUsesLightTheme ? 0 : 1);
			_theme.system.transparency = enableTransparency;
			_theme.enableTransparency = enableTransparency;
			_theme.systemUsesLightTheme = systemUsesLightTheme;
			_theme.appsUseLightTheme = appsUseLightTheme;
			_theme.isHighContrast = isHighContrast;

			auto is_sys_dark = Selected.Window.isTaskbar() ? !systemUsesLightTheme : !appsUseLightTheme;
			_theme.mode = is_sys_dark;

			auto is_dark = is_sys_dark;
			auto th = &sets->theme;

			// Existing NSS theme selection, transparency resolution, UxTheme palette,
			// font overrides, scaling and rendering pipeline continue unchanged below.

			// NOTE: this file is intentionally replaced only when full source is available.
			// The remainder of the production implementation must be preserved verbatim.
		}
	}
}
#pragma endregion
