package main

func main() {
	println("The Ackermann function for m = 3 and n = 4 is ", ackermann(3,4))
}

func ackermann(m, n int, a, b bool) int{
	println(a);
	var a = 5;
	if (m < 0 || n < 0) {
		println("m and n have to be nonnegative");
		return -1
	}
	if  (m == 0) {
		return n+1
	} else if n == 0 {
		return ackermann(m-1, 1);
	} else{
		return ackermann(m-1, ackermann(m, n-1))
	}
}
