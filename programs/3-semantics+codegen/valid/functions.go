//~1st to invoke
//~2nd to invoke
//~3rd to invoke
package main

func init () {
	println("1st to invoke")
}

func main() {
	println("3rd to invoke")
}

func init () {
	println("2nd to invoke")
}

type (
	int1 int
	int2 int1
	int3 int2
)

func foo1 (a struct { a int ; b float64;}) {}
func foo2 (a [5]int) {}
func foo3 (a []int) {}
func foo4 (a,b,c [5]int) {}
func foo5 (a,b,c int3, d,e,f int2 ) {}


func goo (_ int , a int , _ int ) {}

func hoo () int { return 5;}
func ioo () float64 { return 5.0;}
func joo () string { return "five";}
func koo () rune { return '\n';}


type struct1 struct {
	h, i int
	j, k float64
	l int3
}
func loo () struct1 { var g struct1; return g;}


//Some special function names
func None() {}
func malloc() {}
func memcpy() {}
func perror() {}
