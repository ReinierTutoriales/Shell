menu(mode="multiple" title=loc.develop sep=sep.bottom image=\uE26E)
{
	menu(mode="single" title=loc.editors image=\uE17A)
	{
		item(title='Visual Studio Code' image=[\uE272, #22A7F2] cmd='code' args='"@sel.path"')
		separator
		item(type='file' mode="single" title=loc.windows_notepad image cmd='@sys.bin\notepad.exe' args='"@sel.path"')
	}

	menu(mode="multiple" title='dotnet' image=\uE143)
	{
		item(title=loc.dotnet_run cmd-line='/K dotnet run' image=\uE149)
		item(title=loc.dotnet_watch cmd-line='/K dotnet watch')
		item(title=loc.dotnet_clean image=\uE0CE cmd-line='/K dotnet clean')
		separator
		item(title=loc.dotnet_build_debug cmd-line='/K dotnet build')
		item(title=loc.dotnet_build_release cmd-line='/K dotnet build -c release /p:DebugType=None')

		menu(mode="multiple" sep="both" title=loc.dotnet_publish image=\ue11f)
		{
			$publish='dotnet publish -r win-x64 -c release --output publish /*/p:CopyOutputSymbolsToPublishDirectory=false*/'
			item(title=loc.dotnet_publish_single sep="after" cmd-line='/K @publish --no-self-contained /p:PublishSingleFile=true')
			item(title=loc.dotnet_framework_dependent cmd-line='/K @publish')
			item(title=loc.dotnet_framework_executable cmd-line='/K @publish --self-contained false')
			item(title=loc.dotnet_self_contained cmd-line='/K @publish --self-contained true')
			item(title=loc.dotnet_single_file cmd-line='/K @publish /p:PublishSingleFile=true /p:PublishTrimmed=false')
			item(title=loc.dotnet_single_file_trimmed cmd-line='/K @publish /p:PublishSingleFile=true /p:PublishTrimmed=true')
		}
		
		item(title=loc.dotnet_migrations cmd-line='/K dotnet ef migrations add InitialCreate')
		item(title=loc.dotnet_database_update cmd-line='/K dotnet ef database update')
		separator
		item(title=loc.help image=\uE136 cmd-line='/k dotnet -h')
		item(title=loc.version cmd-line='/k dotnet --info')
	}

	item(type="file" title="Resource Hacker" sep="top" image cmd='D:\config\Programs\dev\petools\resource\reshack\reshack.exe' args=sel(1))
	item(type="file" title="PEview" image cmd='D:\config\Programs\dev\petools\peview.exe' args=sel(1))
	
	item(type="dir|dir.back" title=loc.web_server vis=key.shift() cmd='D:\config\Programs\dev\web\HTTPServer\WebServer.exe' args='-open -path:"@sel.path"')
}