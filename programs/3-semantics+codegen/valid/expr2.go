//~true true true true
//~false false false false
//~true true true true
//~false false false false
//~false false false false false
//~true true true true

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

	println(i1 < i2, f1 < f2, r1 < r2, s1 < s2);
	println(i1 > i2, f1 > f2, r1 > r2, s1 > s2);
	println(i1 <= i2, f1 <= f2, r1 <= r2, s1 <= s2);
	println(i1 >= i2, f1 >= f2, r1 >= r2, s1 >= s2);

	println(i1 == i2, f1 == f2, r1 == r2, s1 == s2, b1 == b2);
	println(i1 != i2, f1 != f2, r1 != r2, s1 != s2);

}
