package rdno_wifi

import (
	denv "github.com/jurgen-kluft/ccode/denv"
)

// rdno_wifi is the WiFi package for Arduino Esp32/Esp8266 projects.
const (
	repo_path = "github.com\\jurgen-kluft"
	repo_name = "rdno_wifi"
)

func GetPackage() *denv.Package {
	name := repo_name

	// dependencies
	// networkpkg := rdno_network.GetPackage()

	// main package
	mainpkg := denv.NewPackage(repo_path, repo_name)
	//mainpkg.AddPackage(networkpkg)

	// esp32 library
	esp32wifilib := denv.SetupCppLibProjectForArduinoEsp32(mainpkg, name+"_arduino")
	esp32wifilib.AddEnvironmentVariable("ESP32_SDK")
	esp32wifilib.AddInclude("{ESP32_SDK}", "libraries/WiFi", "src")
	esp32wifilib.SourceFilesFrom("{ESP32_SDK}", "libraries/WiFi", "src")
	//esp32wifilib.AddDependencies(networkpkg.GetMainLib())

	// esp8266 library
	esp8266wifilib := denv.SetupCppLibProjectForArduinoEsp8266(mainpkg, name+"_arduino")
	esp8266wifilib.AddEnvironmentVariable("ESP8266_SDK")
	esp8266wifilib.AddInclude("{ESP8266_SDK}", "libraries/ESP2866WiFi", "src")
	esp8266wifilib.SourceFilesFrom("{ESP8266_SDK}", "libraries/ESP2866WiFi", "src")
	//esp8266wifilib.AddDependencies(networkpkg.GetMainLib())

	// main library
	mainlib := denv.SetupCppLibProject(mainpkg, name)
	//mainlib.AddDependencies(networkpkg.GetMainLib())
	mainlib.AddDependency(esp32wifilib)
	mainlib.AddDependency(esp8266wifilib)

	// test library
	testlib := denv.SetupCppTestLibProject(mainpkg, name)
	//testlib.AddDependencies(networkpkg.GetTestLib())

	mainpkg.AddMainLib(mainlib)
	mainpkg.AddMainLib(esp32wifilib)
	mainpkg.AddMainLib(esp8266wifilib)
	mainpkg.AddTestLib(testlib)
	return mainpkg
}
