//~true false false false

package main

func main(){
	var a []int
	a = append(a, 3)
	var b = a
	b[0] = 4
	var check1 bool = (b[0] == a[0])
	
	a = append(a, 2)
	a = append(a, 1)
	b[0] = 5
	var check2 bool = (b[0] == a[0])
	
	var c [5]int
	c[0] = 1
	var d = c
	c[1] = 2
	d[1] = 3
	var check3 bool = (c[1] == d[1])
	
	var e struct{a int;}
	e.a = 3
	var f = e
	f.a = 4
	var check4 bool = (e.a == f.a)
	
	println(check1, check2, check3, check4);
}
