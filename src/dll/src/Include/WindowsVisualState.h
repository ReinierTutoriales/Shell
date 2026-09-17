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
			uint32_t dpi{ 96 };
			COLORREF accent{};

			bool dark(bool taskbar = false) const
			{
				return !highContrast && !(taskbar ? systemUsesLightTheme : appsUseLightTheme);
			}

			bool allow_transient_backdrop() const
			{
				return windows11 && transientBackdropSupported && compositionEnabled &&
					transparencyEnabled && !highContrast;
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
				// DWMWA_SYSTEMBACKDROP_TYPE is a Windows 11 capability. The actual
				// DWM call remains fail-safe: callers must keep a solid fallback.
				state.transientBackdropSupported = state.windows11;

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
