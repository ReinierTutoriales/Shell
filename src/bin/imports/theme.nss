theme
{
	name="modern"

	// Use Shell's documented view constants. The previous bare "medium"
	// value was not the intended enum form and could fall back to defaults.
	view=view.large

	// Follow the Windows application theme.
	dark=default

	background
	{
		// Let Shell derive the light/dark surface color from Windows.
		color=auto

		// Keep the surface translucent enough for Acrylic to be visible.
		opacity=72

		// 3 = Acrylic in Shell's theme engine.
		effect=3
	}

	font
	{
		// Segoe UI is guaranteed on supported Windows versions and matches
		// Explorer more reliably than forcing a variable-font face here.
		name="Segoe UI"
		size=10
		weight=4
		italic=0
	}

	item
	{
		opacity=100
		radius=3
		prefix=1

		// Windows 11-style touch-friendly row height and horizontal spacing.
		padding=[12,7]
		margin=[4,2]
	}

	border
	{
		enabled=true
		size=1
		opacity=20
		radius=3
		padding=[4,4]
	}

	shadow
	{
		enabled=true
		size=16
		opacity=28
		offset=4
	}

	separator
	{
		size=1
		opacity=18
		margin=[12,6]
	}

	image
	{
		enabled=true
		// Do not globally replace Shell's glyph font: many built-in codepoints
		// belong to Shell's own icon map. Native/SVG icons remain intact.
		gap=10
		scale=true
		align=2
	}

	layout
	{
		popup=0
	}
}
