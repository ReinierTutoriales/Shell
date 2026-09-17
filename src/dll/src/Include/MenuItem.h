#pragma once

#include <oleacc.h>

/*
sent to: +0x18, dwItemData start with 0xaa0df00d before its submenu being populated
// Looks like tagMSAAMENUINFO (MSAAMENUINFO0
// https://docs.microsoft.com/en-us/windows/desktop/api/oleacc/ns-oleacc-tagmsaamenuinfo
struct open_with_submenu
{
	INT32 magicNumber;	// 0d f0 0d aa (0xaa0df00d)
	INT32 length;		//
	WCHAR* text;		// The menu text
};

other menu items: +0x20

HBITMAP hBmp = *((HBITMAP*)(mii.itemData + 0x20));
*/

/*
* //windows 10 dwItemData
struct MenuItemData
{
	wchar_t*	text;		//
	uintptr_t	cch;
	uintptr_t	cch1;
	uint32_t	type;		// MF_POPUP or MF_SEPARATOR or MF_STRING
	uint32_t	iconIndex; 	//def -1
	HBITMAP		hbmpItem;
	HBITMAP		hbmpChecked;
	HBITMAP		hbmpUnchecked;
	uintptr_t	pos;		//def 1=top, 2=bottom
	uint32_t	fontSize;	//def 96=0x60
	uint32_t	state;		// 0 or 101 or 256=0x100
	uint32_t	dummy2;
	uint32_t	dummy3;
	uintptr_t	shared;		//all
	uint32_t	dummy5;
	uint16_t	dummy6;
	uint16_t	dummy7;
};
*/


namespace Nilesoft
{
	namespace Shell
	{
		struct open_with_submenu
		{
			DWORD magicNumber;	// 0d f0 0d aa (0xaa0df00d)
			DWORD length;		//
			WCHAR *text;		// The menu text
		};

		struct MenuItemData
		{
			wchar_t *text;		//
			size_t		cch;
			size_t		cch1;
			uint32_t	type;		// MF_POPUP or MF_SEPARATOR or MF_STRING
			uint32_t	iconIndex; 	//def -1
			HBITMAP		hbmpItem;
			HBITMAP		hbmpChecked;
			HBITMAP		hbmpUnchecked;
			size_t		pos;		//def 0, 1=top, 2=bottom
			uint32_t	dpi;		//def 96=0x60
			uint32_t	state;		// 0 or 0x101 or 256=0x100 // 0 explorer, 0x101 taskabr WM_KEYUP==0x0101
			uint32_t	dummy1;
			uint32_t	dummy2;
			uint32_t	dummy3;
			uint32_t	dummy4;
			uint32_t	dummy5;
			uint32_t	dummy6;
			uint32_t	dummy7;
			uint32_t	dummy8;
			uint32_t	dummy9;
			uint32_t	dummy10;
		};

		struct MenuItemData2
		{
			uint32_t	sig;		// 0x00
			uint32_t	length;		// 0x04
			wchar_t *	text;		// 0x08
			uint32_t	type;		// 0x0f xMF_POPUP or MF_SEPARATOR or MF_STRING
			HBITMAP		hbmpItem0x18; 	// 0x18
			HBITMAP		hbmpItem0x20;	// 0x20
			HBITMAP		hbmpChecked;
			HBITMAP		hbmpUnchecked;
			size_t		pos;		//def 0, 1=top, 2=bottom
			uint32_t	dpi;		//def 96=0x60
			uint32_t	state;		// 0 or 0x101 or 256=0x100 // 0 explorer, 0x101 taskabr WM_KEYUP==0x0101
		};

/*
		struct MenuStyle
		{
			//Renderer
			int Width{ -1 };		//default 158
			int Height{ -1 };		//
			uint32_t Background{};		//BackColor
			uint32_t BackgroundHover{};//BackColorSelected
			uint32_t Title{};			//ForegroundColor
			uint32_t TitleHover{};		//ForegroundColorSelected
			uint32_t Disable{};
			uint32_t Font{};
		};

		struct MenuPopup
		{
			bool	Image{ false };
			bool	Checked{ false };
			bool	HasDraw{ false };
			HMENU	Submenu{ nullptr };
			SIZE	Size{ 0, 0 };
			uint32_t	MaxWidth{ 0 };
		};
*/
		enum class ScaleType
		{
			DPI,
			PPI // This name is from debug symbol. Actually it means scale by window dpi, I don't know why it's called PPI.
		};

		enum ImmersiveContextMenuOptions
		{
			ICMO_USEPPI = 0x1,
			ICMO_OVERRIDECOMPATCHECK = 0x2,
			ICMO_FORCEMOUSESTYLING = 0x4,
			ICMO_USESYSTEMTHEME = 0x8,
			ICMO_ICMBRUSHAPPLIED = 0x10
		};

		enum ContextMenuPaddingType
		{
			CMPT_NONE = 0x0,
			CMPT_TOP_PADDING = 0x1,
			CMPT_BOTTOM_PADDING = 0x2,
			CMPT_TOUCH_INPUT = 0x4,
		};

		struct ContextMenuRenderingData;
		using CMRDArray = std::vector<std::unique_ptr<ContextMenuRenderingData>>;

#define SUBMENU 0x10 // For menuFlags
		struct ContextMenuRenderingData
		{
			std::wstring text;
			UINT menuFlags;
			HBITMAP hbmpItem;
			HBITMAP hbmpChecked;
			HBITMAP hbmpUnchecked;
			ContextMenuPaddingType cmpt;
			ScaleType scaleType;
			UINT dpi;
			bool useDarkTheme;
			bool useSystemPadding;
			bool forceAccelerators;
			CMRDArray *parentArray;
		}; 
		//IMEMENUITEMINFOW
		constexpr auto MF_NOITEM = 0xFFFFFFFFU;
		
		struct MenuItemInfo : public MENUITEMINFOW
		{
			static const uint32_t FMASK = MIIM_FTYPE | MIIM_STRING | MIIM_BITMAP |
				MIIM_DATA | MIIM_ID | MIIM_CHECKMARKS | MIIM_STATE | MIIM_SUBMENU;

			HMENU handle{};
			MenuItemInfo *owner{};
			NativeMenu *owner_static{};
			NativeMenu *owner_dynamic{};
			
			string path;

			struct
			{
				string text;
				string normalize;
				string raw;

				operator const wchar_t *() const { return text; }
				operator wchar_t *() const { return text; }
				explicit operator bool() const { return !text.empty(); }
				template<typename T = uint32_t>
				const T length() const { return text.length<T>(); }
				bool empty() const { return text.empty(); }
				bool equals(const wchar_t *str) const { return normalize.equals(str); }
			}title;


			string keys;
			std::vector<uint32_t> parent;
			std::vector<MenuItemInfo *> items;
			std::vector<NativeMenu *> native_items;

			std::vector<struct menuitem_t *> *sys_items=0;

			//Expression *tip{};
			uint32_t hash = 0;
			uint32_t id = 0;
			uint32_t length = 0;
			uint32_t index = 0;
			NativeMenuType type = NativeMenuType::Item;
			Position position = Position::Auto;
			Visibility visibility = Visibility::Enabled;
			SelectionMode mode = SelectionMode::Single;
			Privileges privileges = Privileges::None;
			ULONG_PTR itemdata = 0;
			Rect rect;
			bool populate{};
			bool dynamic{};
			bool destroy{};
			bool submenu_delete{};
			int separator{};
			int tab = -1;
			int column = 0;
			SIZE size = { -1,-1 };
			bool is_system{};
			struct MUID *ui{};

			struct {
				string val;
				int pos = 0;
				int def = -1;
			} indexof;

			struct {
				uint8_t type{};
				string text;
				uint16_t time = UINT16_MAX;
				explicit operator bool()const { return !text.empty(); }
			}tip;

			struct IMAGE
			{
				HBITMAP hbitmap{};
				SIZE size{};

				ImageImport import = ImageImport::None;
				bool inherited{};
				Expression *expr{};
				uint16_t display = 0;

				struct draw_t
				{
					enum {
						DT_NONE,
						DT_SHAPE,
						DT_GLYPH
					}type = DT_NONE;

					struct
					{
						bool solid = true;
						uint8_t radius = 0;
						SIZE size = { 16, 16 };
						long stroke = -1;
						Color color[2];
					} shape;

					struct glyph_t
					{
						wchar_t code[2] = {0, 0};
						Color color[2];
						SIZE size = { 0,0 };
						HFONT font = nullptr;
					} glyph;
				} draw;

				void destroy()
				{
					if(hbitmap && !inherited)
						::DeleteObject(hbitmap);

					hbitmap = nullptr;
					inherited = false;
					import = ImageImport::None;
				}

				bool isvalid() const 
				{
					return hbitmap != nullptr || inherited || import != ImageImport::None;
				}

				void inherit(IMAGE *img)
				{
					hbitmap = img->hbitmap;
					size = img->size;
					inherited = true;
				}

			} image, image_select;

			MenuItemInfo()
			{
				cbSize = sizeof(MENUITEMINFOW);
				fMask = FMASK;
				fType = 0;
				fState = 0;
				wID = 0;
				hSubMenu = nullptr;
				hbmpChecked = nullptr;
				hbmpUnchecked = nullptr;
				dwItemData = 0;
				hbmpItem = nullptr;
				dwTypeData = nullptr;
				cch = 0;
			}

			MenuItemInfo(uint32_t fMask)
				: MenuItemInfo()
			{
				this->fMask = fMask;
			}

			MenuItemInfo(uint32_t fMask, uint32_t fType)
				: MenuItemInfo(fMask)
			{
				this->fType = fType;
			}

			MenuItemInfo(uint32_t fMask, uint32_t fType, int wID)
				: MenuItemInfo(fMask, fType)
			{
				this->wID = static_cast<uint32_t>(wID);
			}

			MenuItemInfo(uint32_t fMask, uint32_t fType, int wID, const string &title)
				: MenuItemInfo(fMask, fType, wID)
			{
				this->dwTypeData = title;
				this->cch = title.length<uint32_t>();
			}

			MenuItemInfo(const string &title, int wID)
				: MenuItemInfo(MIIM_STRING | MIIM_ID, 0, wID, title)
			{
			}

			~MenuItemInfo()
			{
				if(destroy)
					destroy_hSubMenu();
				image.destroy();
			}

			void delete_dwItemData()
			{
				if(Signed(dwItemData))
				{
					delete reinterpret_cast<MenuItemInfo *>(dwItemData);
					dwItemData = 0;
				}
				else
				{
					//if(HeapValidate(GetProcessHeap(), 0, (void *)dwItemData))
					{
						/*if(HeapFree(GetProcessHeap(), 0, (void *)dwItemData))
						{
							Logger::Info(L"%s", this->dwTypeData);
						}*/
					}
				}
			}

			bool is_id(std::initializer_list<uint32_t> ids) const
			{
				for(const auto &h : ids)
				{
					if(id == h) return true;
				}
				return false;
			}
			auto get_data() const { return reinterpret_cast<MenuItemInfo *>(dwItemData); }
			void set_data() { set_data(this); }

			template<typename T>
			void set_data(T data)
			{
				itemdata = dwItemData;
				dwItemData = reinterpret_cast<uintptr_t>(data);
			}

			void get_title()
			{
				dwTypeData = title.text.buffer(MAX_PATH);
				cch = MAX_PATH;
			}

			void set_title(const string &title)
			{
				this->title.text = title;
				dwTypeData = this->title.text;
				cch = this->title.text.length<uint32_t>();
			}

			void set_title(const wchar_t *title)
			{
				this->title.text = title;
				dwTypeData = this->title.text;
				cch = this->title.text.length<uint32_t>();
			}

			bool get_menuiteminfo(HMENU hMenu, uint32_t uItem, bool fByPosition)
			{
				handle = hMenu;
				return GetMenuItemInfoW(hMenu, uItem, fByPosition, this);
			}

			bool set_menuiteminfo(HMENU hMenu, uint32_t uItem, bool fByPosition)
			{
				handle = hMenu;
				return SetMenuItemInfoW(hMenu, uItem, fByPosition, this);
			}

			bool insert_menuitem(HMENU hMenu, uint32_t uItem, bool fByPosition)
			{
				handle = hMenu;
				return InsertMenuItemW(hMenu, uItem, fByPosition, this);
			}

			bool is_separator() const { return fType & MFT_SEPARATOR; }
			bool is_checked() const { return fState & MFS_CHECKED; }
			bool is_disabled() const { return fState & (MFS_DISABLED | MFS_GRAYED); }
			bool is_popup() const { return hSubMenu != nullptr; }
			bool is_bitmap() const { return fType & MFT_BITMAP; }
			bool is_ownerdraw() const { return fType & MFT_OWNERDRAW; }
			bool is_string() const { return !(fType & MFT_BITMAP) && !(fType & MFT_SEPARATOR); }
			bool is_default() const { return fState & MFS_DEFAULT; }

			bool is_radio() const { return fType & MFT_RADIOCHECK; }
			bool has_image() const { return hbmpItem != nullptr && hbmpItem != HBMMENU_CALLBACK; }
			bool has_checked() const { return hbmpChecked != nullptr && hbmpChecked != HBMMENU_CALLBACK; }
			bool has_unchecked() const { return hbmpUnchecked != nullptr && hbmpUnchecked != HBMMENU_CALLBACK; }

			bool has_submenu() const
			{
				return hSubMenu != nullptr && ::IsMenu(hSubMenu);
			}

			void destroy_hSubMenu()
			{
				if(hSubMenu)
				{
					::DestroyMenu(hSubMenu);
					hSubMenu = nullptr;
				}
			}
		};
	}
}
