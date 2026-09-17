#pragma once

namespace Nilesoft
{
	namespace Shell
	{
		// Immutable snapshot of Windows-owned visual state. Capture it at menu
		// construction/refresh boundaries and consume it from layout/paint paths.
		// NSS remains an override layer above these system-derived defaults.
		struct WindowsVisualState
		{
			bool highContrast{};
			bool systemUsesLightTheme{ true };
			bool appsUseLightTheme{ true };
			bool transparencyEnabled{};
			bool compositionEnabled{};
			bool windows11{};
			bool transientBackdropSupported{};
			uint32_t build{};
			uint32_t dpi{ 96 };
			COLORREF accent{};

			bool dark(bool taskbar = false) const
			{
				return !highContrast && !(taskbar ? systemUsesLightTheme : appsUseLightTheme);
			}

			bool allow_transient_backdrop() const
			{
				return transientBackdropSupported && compositionEnabled &&
					transparencyEnabled && !highContrast;
			}

			// Applies only Windows-owned non-client policy. Shell continues to draw
			// its content and NSS can still override the renderer's semantic tokens.
			// Every DWM call is deliberately best-effort so Explorer always retains
			// the existing solid rendering path when a capability is unavailable.
			void ApplyTransientWindow(HWND window, bool taskbar = false) const
			{
				if(!window || !compositionEnabled)
					return;

				constexpr DWORD DWMWA_USE_IMMERSIVE_DARK_MODE_ = 20;
				constexpr DWORD DWMWA_WINDOW_CORNER_PREFERENCE_ = 33;
				constexpr DWORD DWMWA_SYSTEMBACKDROP_TYPE_ = 38;
				constexpr int DWMWCP_ROUNDSMALL_ = 3;
				constexpr int DWMSBT_NONE_ = 1;
				constexpr int DWMSBT_TRANSIENTWINDOW_ = 3;

				const BOOL useDark = dark(taskbar) ? TRUE : FALSE;
				::DwmSetWindowAttribute(window, DWMWA_USE_IMMERSIVE_DARK_MODE_, &useDark, sizeof(useDark));

				if(windows11)
				{
					const int corner = DWMWCP_ROUNDSMALL_;
					::DwmSetWindowAttribute(window, DWMWA_WINDOW_CORNER_PREFERENCE_, &corner, sizeof(corner));
				}

				// DWMSBT_TRANSIENTWINDOW delegates the actual transient material to
				// Windows instead of hard-coding an Acrylic implementation in Shell.
				// This also lets future Windows versions evolve the material policy.
				if(transientBackdropSupported)
				{
					const int backdrop = allow_transient_backdrop() ? DWMSBT_TRANSIENTWINDOW_ : DWMSBT_NONE_;
					::DwmSetWindowAttribute(window, DWMWA_SYSTEMBACKDROP_TYPE_, &backdrop, sizeof(backdrop));
				}
			}

			static WindowsVisualState Capture(HWND owner = nullptr, uint32_t requestedDpi = 0)
			{
				WindowsVisualState state;

				HIGHCONTRASTW hc{ sizeof(HIGHCONTRASTW) };
				if(::SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0))
					state.highContrast = (hc.dwFlags & HCF_HIGHCONTRASTON) != 0;

				DWORD value{};
				DWORD size = sizeof(value);
				constexpr auto personalize = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize";

				if(::RegGetValueW(HKEY_CURRENT_USER, personalize, L"SystemUsesLightTheme",
					RRF_RT_REG_DWORD, nullptr, &value, &size) == ERROR_SUCCESS)
					state.systemUsesLightTheme = value != 0;

				value = 1;
				size = sizeof(value);
				if(::RegGetValueW(HKEY_CURRENT_USER, personalize, L"AppsUseLightTheme",
					RRF_RT_REG_DWORD, nullptr, &value, &size) == ERROR_SUCCESS)
					state.appsUseLightTheme = value != 0;

				value = 0;
				size = sizeof(value);
				if(::RegGetValueW(HKEY_CURRENT_USER, personalize, L"EnableTransparency",
					RRF_RT_REG_DWORD, nullptr, &value, &size) == ERROR_SUCCESS)
					state.transparencyEnabled = value != 0;

				BOOL composition{};
				if(SUCCEEDED(::DwmIsCompositionEnabled(&composition)))
					state.compositionEnabled = composition != FALSE;

				BOOL opaqueBlend{};
				DWORD colorization{};
				if(SUCCEEDED(::DwmGetColorizationColor(&colorization, &opaqueBlend)))
					state.accent = RGB((colorization >> 16) & 0xff, (colorization >> 8) & 0xff, colorization & 0xff);
				else
					state.accent = ::GetSysColor(COLOR_HIGHLIGHT);

				state.windows11 = Windows::Version::Instance().IsWindows11OrGreater();

				// DWMWA_SYSTEMBACKDROP_TYPE is documented for Windows 11 22H2
				// (build 22621) and later. Query the real OS build so we do not send
				// attribute 38 merely because the machine is some Windows 11 build.
				using RtlGetVersion_t = LONG(WINAPI *)(PRTL_OSVERSIONINFOW);
				if(auto ntdll = ::GetModuleHandleW(L"ntdll.dll"))
				{
					if(auto rtlGetVersion = reinterpret_cast<RtlGetVersion_t>(::GetProcAddress(ntdll, "RtlGetVersion")))
					{
						RTL_OSVERSIONINFOW version{ sizeof(version) };
						if(rtlGetVersion(&version) == 0)
						{
							state.build = version.dwBuildNumber;
							state.windows11 = version.dwMajorVersion >= 10 && version.dwBuildNumber >= 22000;
							state.transientBackdropSupported = version.dwMajorVersion >= 10 && version.dwBuildNumber >= 22621;
						}
					}
				}

				if(requestedDpi)
					state.dpi = requestedDpi;
				else if(owner && Windows::Version::Instance().IsWindowsVersionOrGreater(10, 0, 14393))
				{
					auto user32 = ::GetModuleHandleW(L"user32.dll");
					if(user32)
					{
						using GetDpiForWindow_t = UINT(WINAPI *)(HWND);
						auto getDpiForWindow = reinterpret_cast<GetDpiForWindow_t>(::GetProcAddress(user32, "GetDpiForWindow"));
						if(getDpiForWindow)
						{
							auto dpi = getDpiForWindow(owner);
							if(dpi) state.dpi = dpi;
						}
					}
				}

				// Accessibility is authoritative: decorative transparency must not
				// survive a High Contrast transition even if the registry value is on.
				if(state.highContrast)
					state.transparencyEnabled = false;

				return state;
			}
		};
	}
}
