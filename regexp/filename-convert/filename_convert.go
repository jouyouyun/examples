package main

import (
	"flag"
	"fmt"
	"os"
	"path"
	"regexp"
	"strings"
)

var (
	// leadingNumbersAndSpacesPattern matches two digits followed by one or more spaces at the beginning
	leadingNumbersAndSpacesPattern = regexp.MustCompile("^\\d{2} +")
)

// convertFilename processes a filename by:
// 1. Removing leading two digits and spaces if present
// 2. Removing all hyphens
// 3. Preserving the file extension
func convertFilename(filename string) string {
	// Split filename into base and extension
	lastDotIndex := strings.LastIndex(filename, ".")
	if lastDotIndex == -1 {
		return filename // No extension, return as-is
	}
	base := filename[0:lastDotIndex]
	ext := filename[lastDotIndex+1:]

	// Process base name
	base = stripLeadingNumbersAndSpaces(base)
	base = removeHyphens(base)

	return base + "." + ext
}

// stripLeadingNumbersAndSpaces removes leading two digits followed by spaces from the string
func stripLeadingNumbersAndSpaces(s string) string {
	return leadingNumbersAndSpacesPattern.ReplaceAllString(s, "")
}

// removeHyphens removes all hyphens from the string
func removeHyphens(s string) string {
	items := strings.Split(s, "-")
	num := len(items)
	if num <= 3 {
		return strings.ReplaceAll(s, "-", "")
	}

	mid := strings.Join(items[1:num-2], "-")
	return items[0] + mid + items[num-2] + items[num-1]
}

func main() {
	// Parse source and target directories from command line arguments
	srcDir := flag.String("src", "./", "Source directory to read files from")
	tgtDir := flag.String("dst", "./converted", "Target directory to move converted files")
	dryRun := flag.Bool("dry-run", false, "Only simulate run")
	flag.Parse()

	// Ensure target directories exist
	if err := os.MkdirAll(*tgtDir, 0755); err != nil {
		fmt.Printf("Error creating target directory: %v\n", err)
		return
	}

	// Process all files in source directory
	files, err := os.ReadDir(*srcDir)
	if err != nil {
		fmt.Printf("Error reading source directory: %v\n", err)
		return
	}

	for _, file := range files {
		if file.IsDir() {
			continue // Skip directories
		}

		// Convert filename
		newFilename := convertFilename(file.Name())

		// Copy file from source to target directory
		srcPath := path.Join(*srcDir, file.Name())
		tgtPath := path.Join(*tgtDir, newFilename)

		if !*dryRun {
			// Read source file
			data, err := os.ReadFile(srcPath)
			if err != nil {
				fmt.Printf("Error reading file %s: %v\n", file.Name(), err)
				continue
			}

			// Write to target file
			if err := os.WriteFile(tgtPath, data, 0644); err != nil {
				fmt.Printf("Error writing file %s: %v\n", newFilename, err)
				continue
			}
		}
		fmt.Printf("Successfully copied %s to %s\n", srcPath, tgtPath)
		if !*dryRun {
			_ = os.Remove(srcPath)
		}
	}
}
