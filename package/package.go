package rdno_wifi

import (
	denv "github.com/jurgen-kluft/ccode/denv"
	rdno_network "github.com/jurgen-kluft/rdno_network/package"
)

// rdno_wifi is the Wifi package for Arduino projects.
func GetPackage() *denv.Package {
	unetpkg := rdno_network.GetPackage()

	mainpkg := denv.NewPackage("rdno_wifi")
	mainpkg.AddPackage(unetpkg)

	mainlib := denv.SetupCppLibProject("rdno_wifi", "github.com\\jurgen-kluft\\rdno_wifi")
	mainlib.AddEnvironmentVariable("ESP_SDK")
	mainlib.AddExternalInclude("{ESP_SDK}/libraries/WiFi/src")
	mainlib.AddExternalSourcesFromForArduino("{ESP_SDK}/libraries/WiFi/src")
	mainlib.AddDependencies(unetpkg.GetMainLib()...)

	mainpkg.AddMainLib(mainlib)
	return mainpkg
}
