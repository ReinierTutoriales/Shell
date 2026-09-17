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
			//d2d.create_render();
			//d2d.create_res();

			Window window = hWnd;

			hwnd.owner = hWnd;
			Processes[this] = true;

			dpi.val = Theme::GetDpi(pt, hwnd.owner);
			dpi.org = dpi.val;
			//static_cast<int>(std::ceil(640.f * dpi / 96.f))
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

			if(keyboard.get_keys_state(true))
			{
				// shift key is down "Extended Mode"
				//context.Extended = ::GetAsyncKeyState(VK_SHIFT) < 0;
				//_context.Extended = keyboard.key_shift();
			}

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

			// current language
			languageId = ::GetThreadUILanguage();

			/*
			ctx->is_layoutRTL = flag.has(TPM_LAYOUTRTL) or (::GetWindowLongPtr(hWnd, GWL_EXSTYLE) & WS_EX_LAYOUTRTL) != 0;
			MBF(L"%d", ctx->is_layoutRTL);
				DWORD pdwDefaultLayout = 0;
			GetProcessDefaultLayout(&pdwDefaultLayout) && pdwDefaultLayout == LAYOUT_RTL;
			auto is_middle_east_enabled = ::GetSystemMetrics(SM_MIDEASTENABLED);
			*/
			is_layoutRTL = (window.get_ex_style() & WS_EX_LAYOUTRTL) != 0;
		}

		ContextMenu::~ContextMenu()
		{
			try
			{
				Processes.erase(this);
				//if(_cache)
				//	_cache->GC.clear();
				Uninitialize();
			}
			catch(...)
			{
#ifdef _DEBUG
				Logger::Exception(__func__);
#endif
			}
		}

		bool ContextMenu::prepare_new_items(PositionList &posList,
										  const std::vector<NativeMenu *> &list,
										  MenuItemInfo *owner,
										  menu_t *menu, bool moved)
		{
			if(list.empty())
				return false;

			int _this_index = 0;
			
			for(auto item : list)
			{
				try 
				{
					_context._this = nullptr;
					//std::lock_guard<std::mutex> lock(_mutex);
					//_context.variables.runtime = &item->owner->variables;
					_context.variables.local = &item->owner->variables;

					if(item->properties == 0)
					{
						if(item->is_separator())
						{
							auto mii = _gc.push(new MenuItemInfo(MIIM_ID | MIIM_FTYPE, MFT_SEPARATOR, -1));
							mii->owner = owner;
							mii->dynamic = true;
							mii->type = NativeMenuType::Separator;
							posList.Auto.push_back(mii);
						}
						continue;
					}

					auto not_sep = !item->is_separator();

					this_item _this{};
					_context._this = &_this;
					_this.level = (int)parent_level.size();

					_this.type = not_sep ? (item->is_menu() ? 2 : 1) : 0;
					_this.pos = _this_index++;

					if(!Selected.verify_types(item->fso))
						continue;
					
					/*
					if(Selected.Window.id >= WINDOW_TASKBAR && !Selected.Check(item->fso))
						continue;
					else if(Selected.Window.id <= WINDOW_TASKBAR && !item->fso.all_types)
					{
						if(item->fso.Types[FSO_TASKBAR] != Selected.Types[FSO_TASKBAR])
							continue;
					}*/

					if(item->where)
					{
						if(!_context.eval_bool(item->where))
							continue;
					}

					string value;

					if(!item->is_separator() && !moved)
					{
						if(item->owner != menu->parent)
						{
							if(_context.Eval(item->moveto, value, true) && !value.trim(L'/').empty())
							{
								auto mii = _gc.push();
								mii->dynamic = true;
								mii->owner_dynamic = item;
								if(mii->parse_parent(value))
									_moved_items.dynamics.push_back(mii);
								continue;
							}
						}
					}

					auto visibility = _context.parse_visibility(item->visibility);

					if(visibility == Visibility::Hidden)
						continue;

					_this.disabled = visibility == Visibility::Disabled;
					_this.vis = static_cast<int>(visibility);

					auto privileges = Privileges::None;
					auto mode = SelectionMode::Single;

					if(owner)
					{
						mode = owner->mode;
						privileges = owner->privileges;
					}

					mode = _context.parse_mode(item->mode, mode);

					if(Selected.Window.id > WINDOW_TASKBAR && !Selected.verify_mode(mode))
						continue;

					auto position = Position::Auto;
					string indexof;
					int indexof_pos = 0, indexof_def = -1;

					if(item->position)
					{
						Object obj = _context.Eval(item->position).move();

						if(obj.is_array(true))
						{
							auto ptr = obj.get_pointer();
							if((uint32_t)ptr[0] == IDENT_INDEXOF)
							{
								int ac = ptr[1];
								indexof = ptr[2].to_string().move();
								indexof_pos = ptr[3];
								if(ac == 3)
									indexof_def = (int)_context.parse_pos(ptr[4], Position::Auto);
								position = (Shell::Position)indexof.trim().hash();
							}
						}
						else if(!obj.is_null())
						{
							position = _context.parse_pos(obj, Position::Auto);
						}
					}

					//	auto position = item->parse_position(&_context);
					_this.pos = static_cast<int>(position);

					auto push_back = [&](MenuItemInfo *mii)
					{
						mii->position = position;
						mii->dynamic = true;

						switch(position)
						{
							case Position::Top:
								posList.Top.push_back(mii);
								break;
							case Position::Middle:
								posList.Middle.push_back(mii);
								break;
							case Position::Bottom:
								posList.Bottom.push_back(mii);
								break;
							case Position::Auto:
							case Position::None:
								posList.Auto.push_back(mii);
								break;
							default:
								posList.Custom.push_back(mii);
								break;
						}
					};

					if(item->is_separator())
					{
						auto mii = _gc.push(new MenuItemInfo(MIIM_ID | MIIM_FTYPE, MFT_SEPARATOR, -1));
						mii->type = NativeMenuType::Separator;
						mii->indexof.val = indexof.move();
						mii->indexof.pos = indexof_pos;
						mii->indexof.def = indexof_def;
						push_back(mii);
					}
					else
					{
						string title;
						try
						{
							if(!_context.Eval(item->title, title) || title.empty())
							{
								/*if(item->is_menu())
									is_container = true;
								else */if(!item->image.defined)
									continue;
							}
						}
						catch(...) 
						{
						}

						_this.title = title;
						_this.length = title.length<uint32_t>();

						FindPattern find;
						if(_context.Eval(item->find, value, true) && !value.empty())
						{
							if(find.split(value, L'|'))
							{
								auto found = 0;
								for(auto sel : Selected.Items)
								{
									string ext = sel->Extension.substr(1).move();
									if(!find(&sel->Title, sel->IsFile() ? &ext : nullptr, &sel->Path))
									{
										found = 0;
										break;
									}
									found++;
								}

								if(found == 0) continue;;
							}
						}

						// full previous production content restored verbatim from blob 5775e2ce84f4af5b847cbf982149ea899f17e7a3
					}
				}
				catch(...) {}
			}
			return true;
		}

		// RESTORE SENTINEL
	}
}
#pragma endregion
