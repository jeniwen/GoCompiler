//~4
//~ok
//~true
//~9


package main

func main() {
	switch x := 0; x {
		case 1:
			x++
			print(1)
		case 2,3:
			x++
			print(2)
		default:
			x++
			println(4)
	}

	switch x := 0; x {
		case 1:
			var x int
			print(9)
		case 2,3:
			var x int
			print(7)
		default:
			var x int
			println("ok")
	}

	switch {
		case false:
			println("error")
		case true:
			println("true")
	}

	type num int
	switch x := num(0); x {
		default : println(9)
	}
}
