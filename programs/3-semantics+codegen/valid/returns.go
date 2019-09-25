//~+7.000000e+000
//~0
//~0
//~+0.000000e+000 +0.000000e+000 +0.000000e+000
//Mostly borrowed from program-solutions/2-typecheck
package main

func incr(x float64) float64 {
	return x + 1.0
}


type point struct {
	x, y, z float64
}

func new_point() point {
	var p point
	p.x = incr(-1.0)
	p.y = 0.0
	p.z = 0.0
	return p
}

func ret_if() int {
	if true {
		return 0
	} else if false {
		return 1
	} else {
		return 2
	}
}

func ret_switch() int {
	switch {
		case true:
			return 0
		case false:
			return 1
		default:
			return 2
	}
}

func main() {
	println(incr(incr(incr(incr(3.0)))))
	println(ret_if())
	println(ret_switch())
	var p1 point = new_point()
	println(p1.x, p1.y, p1.z)
	return
}
