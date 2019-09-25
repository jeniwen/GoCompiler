package main // I don't really know if I did it right, but it runs and it takes a bit

type matrix [1000][1000]float64
type vector [1000]float64
type tuple struct{a,b matrix;}

func sqrt(x float64) float64{
	var num float64 = x
	for i:=0;i<10;i++{
		x = x - (x*x - num)/(2.0*x)
	}
	return x
}

func classical_graham_schmidt(A matrix, x,y int) tuple {

	m,n:=x,y
    var R matrix
	var Q matrix
	var v vector
	
	for j := 0; j < n;j++ {
		for k:=0;k<m;k++{
			v[k] = A[k][j];
		}
       
		for i:=0;i<j-1;i++{
			for k:=0;k<m;k++{
				R[i][j] = Q[i][k]*A[k][j];
			}
			for k:=0;k<m;k++{
				v[k] = v[k] - R[i][j]*Q[k][i];
			}
		}
		
		for k:=0;k<m;k++{
			R[j][j] = sqrt(v[k]*v[k]);
		}
		for k:=0;k<m;k++{
				Q[k][j] = v[k]/R[j][j];
		}
    }
	
	var val tuple
	val.a = Q
	val.b = R
	return val
}

func main(){
	var vandermonde matrix
	
	for i:=0.0;i<1000.0;i++{
		var curr float64 = i
		var item float64 = 1.0
		for j:=0.0;j<1000.0;j++{
			vandermonde[int(i)][int(j)] = item
			item *= curr
		}
	}
	
	var val tuple = classical_graham_schmidt(vandermonde, 1000, 1000)
	
	for i:=0;i<1000;i++{
		for j:=0;j<1000;j++{
			print(val.b[i][j])
			print(" ")
		}
		print("\n")
	}
}
