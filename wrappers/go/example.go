package main

import (
    "fmt"
    "image/png"
    "os"
    
    "github.com/Mittelstand-ai-GmbH-Co-KG/zxing-cpp"
)

func main() {
    // Open image file
    file, err := os.Open("barcode.png")
    if err != nil {
        panic(err)
    }
    defer file.Close()
    
    // Decode image
    img, err := png.Decode(file)
    if err != nil {
        panic(err)
    }
    
    // Create config and scan
    config := zxing.NewConfig()
    if err != nil {
		panic(err)
	}
	defer config.Close()

    symbols, err := config.Scan(img)
    if err != nil {
        panic(err)
    }
    
    // Print results
    for _, symbol := range symbols {
        fmt.Printf("Type: %s\nData: %s\n", symbol.BcType, symbol.Data)
    }
}