//~1
//~2 3
//~4 5
//~0
//~1 +2.000000e+000 3 0 +0.000000e+000
//~1
//~4
//~9
//~16
//~optional init

package main

func main() {
	a := 1
	println(a)

	b, c := 2, 3
	println(b, c)

	b, d := 4, 5
	println(b, d)

	{
		a := 0
		println(a);
	}

	var e int
	var f float64

	e, f, g := 1, 2.0, 3

	h, _ := 0, 0.0
	i, _ := 0.0, 0
	println(e,f,g,h,i)

	for j:=1;j<5;j++{
		println(j*j);
	}

	if k:=0; k > -1 {
		println("optional init")
	}
}
