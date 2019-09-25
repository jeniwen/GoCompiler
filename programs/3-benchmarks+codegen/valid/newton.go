package main



func mat_transpose(ar1 [1000][1000]float64) [1000][1000]float64{
	for i := 0; i < 1000; i++{
		for j := 0 ; j < i ; j++{
			ar1[i][j] = ar1[j][i]
		}
	}
	return ar1
}

func mat_mul(ar1, ar2 [1000][1000]float64) [1000][1000]float64{
	ar2 = mat_transpose(ar2)
	var to_ret [1000][1000]float64
	for i := 0; i< 1000; i++{
		for j:=0 ; j < 1000; j++{
			for k := 0 ; k < 1000; k++{
				to_ret[i][j] += ar1[i][k] + ar2[j][k]
			}
		}
	}
	return to_ret
}

func make_matrix(x float64) [1000][1000]float64{
	var to_ret [1000][1000]float64
	for i:= 0 ; i < 1000; i++{
		for j := 0 ; j < 1000; j++{
			to_ret[i][j] = x
		}
	}
	return to_ret
}

func print_matrix(array [1000][1000]float64){
	for i:=0; i < 1000; i++{
		for j := 0 ; j < 1000; j++{
			print(array[i][j], " ")
		}
		print("\n")
	}
}

func main(){
	var ar1 = make_matrix(1.2)
	var ar2 = make_matrix(3.0)
	mat_mul(ar1, ar2)
	println("Computed the multiplication of 2 1000 by 1000 matrices")
}
