// A command to turn CSV twitter data into a sequence of
// NULL-terminated documents to be fed into the C API.

package main

import (
	"encoding/csv"
	"fmt"
	"io"
	"os"
)

func main() {
	reader := csv.NewReader(os.Stdin)
	for {
		row, err := reader.Read()
		if err != nil {
			if err == io.EOF {
				return
			}
			fmt.Fprintln(os.Stderr, err)
			os.Exit(1)
		}
		os.Stdout.Write(append([]byte(row[len(row)-1]), 0))
	}
}
