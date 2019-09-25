//~yay
//~7
package main

type blank_struct struct {
	_ int
	_ float64
}

func print_int(i int) int {
	println(i)
	return i;
}

func blank_params(_ int, _ float64, _ int) {
	println("yay")
}

func main() {
	blank_params(4, 6.0, 5)
	_ = print_int(7)
  _= 4+4;
}
