theme
{
	name="modern"
	view=medium
	dark=auto

	background
	{
		color=auto
		opacity=88
		// Native acrylic supported by Shell. Windows automatically falls back
		// when transparency is unavailable.
		effect=3
	}

	font
	{
		name="Segoe UI Variable Text"
		size=9
		weight=4
		italic=0
	}

	item
	{
		opacity=100
		radius=2
		prefix=1
		padding=[10,6]
		margin=[4,2]
	}

	border
	{
		enabled=true
		size=1
		opacity=18
		radius=3
		padding=[4,4]
	}

	shadow
	{
		enabled=true
		size=12
		opacity=24
		offset=4
	}

	separator
	{
		size=1
		opacity=18
		margin=[10,5]
	}

	image
	{
		enabled=true
		glyph="Segoe Fluent Icons"
		gap=10
		scale=true
		align=2
	}

	layout
	{
		popup=0
	}
}