package rdno_wifi

import (
	denv "github.com/jurgen-kluft/ccode/denv"
	rdno_network "github.com/jurgen-kluft/rdno_network/package"
)

// rdno_network is the Network core package for Arduino Esp32 projects.
const (
	repo_path = "github.com\\jurgen-kluft"
	repo_name = "rdno_wifi"
)

func GetPackage() *denv.Package {
	name := repo_name

	// dependencies
	networkpkg := rdno_network.GetPackage()

	// main package
	mainpkg := denv.NewPackage(repo_path, repo_name)
	mainpkg.AddPackage(networkpkg)

	// esp32 library
	esp32wifilib := denv.SetupCppLibProjectForArduinoEsp32(mainpkg, name+"_arduino")
	esp32wifilib.AddEnvironmentVariable("ESP_SDK")
	esp32wifilib.AddInclude("{ESP_SDK}", "libraries/WiFi", "src")
	esp32wifilib.SourceFilesFrom("{ESP_SDK}", "libraries/WiFi", "src")
	esp32wifilib.AddDependencies(networkpkg.GetMainLib()...)

	// main library
	mainlib := denv.SetupCppLibProject(mainpkg, name)
	mainlib.AddDependencies(networkpkg.GetMainLib()...)
	mainlib.AddDependencies(esp32wifilib)

	// test library
	testlib := denv.SetupCppTestLibProject(mainpkg, name)
	testlib.AddDependencies(networkpkg.GetTestLib()...)

	mainpkg.AddMainLib(mainlib)
	mainpkg.AddMainLib(esp32wifilib)
	mainpkg.AddTestLib(testlib)
	return mainpkg
}
