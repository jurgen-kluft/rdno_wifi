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

	// main library
	mainlib := denv.SetupCppLibProject(mainpkg, name)
	mainlib.AddEnvironmentVariable("ESP_SDK")
	mainlib.AddInclude("{ESP_SDK}", "libraries/WiFi", "src")
	mainlib.SourceFilesFrom("{ESP_SDK}", "libraries/WiFi", "src")
	mainlib.AddDependencies(networkpkg.GetMainLib()...)

	mainpkg.AddMainLib(mainlib)
	return mainpkg
}
