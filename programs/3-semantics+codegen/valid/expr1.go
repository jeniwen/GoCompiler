//~7
//~-1
//~0
//~12
//~+7.000000e+000
//~-1.000000e+000
//~+7.500000e-001
//~+1.200000e+001
//~229
//~-1
//~0
//~13110
//~HelloWorld
//~true

package main

func main() {

	var i1, i2 int
	var f1, f2 float64
	var r1, r2 rune
	var s1, s2 string
	var b1, b2 bool

	i1, i2 = 3, 4
	f1, f2 = 3.0, 4.0
	r1, r2 = 'r', 's'
	b1, b2 = true, false
	s1, s2 = "Hello", "World"

	println(i1 + i2)
	println(i1 - i2)
	println(i1 / i2)
	println(i1 * i2)

	println(f1 + f2)
	println(f1 - f2)
	println(f1 / f2)
	println(f1 * f2)

	println(r1 + r2)
	println(r1 - r2)
	println(r1 / r2)
	println(r1 * r2)


	println(s1 + s2)
	println(b1 || b2)

}
