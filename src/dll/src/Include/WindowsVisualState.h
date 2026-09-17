#pragma once

namespace Nilesoft
{
	namespace Shell
	{
		// Snapshot of Windows-owned visual state. Capture once while constructing a
		// context menu and consume the snapshot instead of repeatedly querying the
		// registry from paint/layout hot paths.
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
		};
	}
}
