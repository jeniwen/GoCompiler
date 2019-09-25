//~5 5
//~5 3
//~1
//~2 3 1
//~8
//~8

package main

var global_a int = 0

func a_call() int{
	global_a++
	return 0
}

func main(){
	
	// Copy of underlying array in as structure
	var a struct {b [2]int;}
	
	a.b[0] = 5
	
	var s struct {b [2]int;}
	
	s = a
	
	println(a.b[0], s.b[0])
	
	s.b[0] = 3
	println(a.b[0], s.b[0])
	
	
	//blank identifier
	_ = a_call()
	
	println(global_a)
	
	//Swap
	var a2, b2, c2 int = 1,2,3
	a2,b2,c2 = b2, c2, a2
	println(a2, b2, c2)
	
	//bit clear
	
	var _z_ = 9&^37
	println(_z_)

	var b = 9
	b &^= 37
	println(b)
	
}
