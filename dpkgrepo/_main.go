package main

import (
	"./dpkgrepo"
	"fmt"
)

func TestConfig() {
	rc, err := dpkgrepo.LoadRepoConfig("./repo.yml")
	if err != nil {
		fmt.Println("Failed to load config:", err)
		return
	}

	for _, dist := range rc.RepoList {
		fmt.Println("URI:", dist.URI)
		fmt.Println("\tDistribution:", dist.Distribution)
		fmt.Println("\tComponents:", dist.Components)
	}
}

func TestComponent() {
	list, err := dpkgrepo.ParseComponent("http://pools.corp.deepin.com/deepin",
		"stable", "main", "amd64")
	if err != nil {
		fmt.Println("Failed to parse component:", err)
		return
	}
	var count = 0
	for _, repo := range list {
		if count > 10 {
			break
		}
		count++
		fmt.Println(repo.Package)
		fmt.Println("\tDistribution:", repo.Distribution)
		fmt.Println("\tComponent:", repo.Component)
		fmt.Println("\tVersion:", repo.Version)
		fmt.Println("\tArchitecture:", repo.Architecture)
		fmt.Println("\tHomepage:", repo.Homepage)
		fmt.Println("\tSection:", repo.Section)
		fmt.Println("\tSummary:", repo.Summary)
		fmt.Println("\tInstalled Size:", repo.InstalledSize)
		fmt.Println("\tSize:", repo.Size)
	}
}

func main() {
	TestComponent()
}
