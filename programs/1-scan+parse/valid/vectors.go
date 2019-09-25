package main

type vector struct{
	x, y, z float64
}

func vector_sum(a, b vector) vector{
	var res vector;
	res.x, res.y, res.z = a.x+b.x, a.y+b.y, a.z+b.z;
	return res;
	for{
		for{
			for{
				i:=0
			}
		}
	}
}

func norm(v vector) float64{
	var squares float64 = v.x*v.x+v.y*v.y+v.z*v.z
	return sqrt(squares, 20)
}

func sqrt(x float64, iter int) float64{
	var(
	 	guess float64 = float64(1)
		quot float64 = x;
	)
	for   ; iter > 0;{
		quot = x/guess;
		guess = 0.5 * (guess+quot)
	}
	return guess
}

func vector_slice_sum(array []vector) vector{
	var sum vector
	sum.x = .0
	sum.y = .0
	sum.z = .0
	for i := 0; i < len(array)-1; i++ {
		sum = vector_sum(sum, array[i])
	}
	return sum
}

func main(){
	var(
		a, b, c vector;
		z []vector;
	)
	a.x = 2.0
	a.y = 3.0
	a.z = 5.0
	z = append(z, a)
	b.x, b.y, b.z = 4.0, 7.0, 3.3
	z = append(z, b)
	c.x, c.y, c.z = 9.0, 1.1, 8.8
	z = append(z, c)
	println(norm(vector_slice_sum(z)))
}
